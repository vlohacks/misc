#include <curl/curl.h>

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <windows.h>
#include <tlhelp32.h>
#include <time.h>

/* 
 * This signature always reliably shows up next to the 32 bit login count in 
 * game memory
 */
const uint64_t MAGIC_SIGNATURE = 0x0000000000013395;

/*
 * Format windows error messages
 */
void errmsg(char *ptr, size_t ptrl)
{
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), ptr, 
		ptrl, NULL);
}

/* 
 * find process id (PID) by it's exe file name
 */
DWORD pid_by_exename(const char *exe_filename) 
{
	PROCESSENTRY32 entry;
	DWORD ret = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
	entry.dwSize = sizeof(entry);

	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (strcmp(entry.szExeFile, exe_filename) == 0) {
				ret = entry.th32ProcessID;
				break;
			}
		}
	}

	CloseHandle(snapshot);
    return ret;
}

/* 
 * search the ffxiv process for the login count by searching a reliable magic 
 * value 4 bytes ahead. TODO Maybe there is a more solid way to find it by 
 * traversing tons of pointers back until we find a reliable address which is
 * not dynamically allocated on the heap, but this works pretty fine yet...
 */
void* find_mem_location(HANDLE proc) 
{
	uint64_t addr = 0;
	uint64_t ptr;
	SIZE_T num_read;
	MEMORY_BASIC_INFORMATION info;
	char *buffer;
	const size_t USERSPACE_LIMIT = 0x7fffffffffff;
	size_t buffer_size = 4096;
	
	buffer = malloc(buffer_size);

	// walk whole user address space
	while (addr < USERSPACE_LIMIT) {
		// check if region is mapped and read/write and therefore most likely 
		// heap memory
		if (VirtualQueryEx(proc, (void*)addr, &info, sizeof(info))) {
			if (info.Protect == PAGE_READWRITE) {
				printf("\rSearching for magic: %016llx, %016llx bytes ...", 
					info.BaseAddress, info.RegionSize);	
				fflush(stdout);
				ptr = (uint64_t)info.BaseAddress;
				// dynamicly grow local buffer, if mem region exceeds buffer
				// size
				if (info.RegionSize > buffer_size) {
					buffer_size = info.RegionSize;
					buffer = realloc(buffer, buffer_size);
				}
				// read the memory region and search for the magic which is 4 
				// bytes ahead the login counter. 
				// Note: reading the whole region and searching locally is way
				// faster than ReadProcessMemory system calls every 4 bytes.
				ReadProcessMemory(proc, (void*)ptr, buffer, 
					info.RegionSize, &num_read);
				for (ptr = 0; ptr < info.RegionSize - 4; ptr += 4) {
					if (*(uint64_t*)&buffer[ptr + 4] == MAGIC_SIGNATURE) {
						printf("\n");
						free(buffer);
						return (void*)(info.BaseAddress + ptr);
					}
				}
			}
			addr += info.RegionSize;
		} else {
			// this won't happen most likely... 
			addr += 0x10000; 
		}
	}

	free(buffer);
	printf("\n");
	return 0;
}

/*
 * Send a message through the callmebot API
 */
void callmebot_send_message(const char *message)
{
	// mobile phone number to send message to
	const char NUMBER[] = "<place mobile num here>";

	// Signal callmebot API key
	const char APIKEY[] = "<place api key here>";				

	char url[256];
	
	snprintf(url, sizeof(url), 
		"https://api.callmebot.com/signal/send.php?phone=%s&apikey=%s&text=%s",
		 NUMBER, APIKEY, message);

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();
}



int main(int argc, char **argv)
{
	DWORD		pid;
	HANDLE		proc;
	HANDLE		hToken;
	uint32_t	ppl_ahead;
	uint32_t	ppl_ahead_last = 0xffffffff;
	uint32_t	ppl_delta;
	SIZE_T		num_read;
	char		msg[256];
	char 		buffer[12];		// holds 4 byte login count + 8 bytes magic
	int 		i;
	void		*ptr;
	const char	*str;
	time_t		time_last;
	time_t		time_init;
	time_t		time_delta;
	time_t		time_est;

	const char	*exe_names[] = {
		"ffxiv_dx11.exe",
		"ffxiv.exe",
		0
	};

	printf("Welcome to the Cynthia Cyber FFXIV Login informer!\n");

	for (i = 0; (str = exe_names[i]) != 0; i++) {
		pid = pid_by_exename(str);
		if (pid > 0)
			break;
	}

	if (pid == 0) {
		printf("Could not find game process. Is FFXIV running?\n");
		exit(1);
	}

	printf("Game pid = %d(0x%x)\n", pid, pid);

	proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
	if (proc == NULL) {
		errmsg(msg, sizeof(msg));
		printf("OpenProcess failed: %s\n", msg);
		exit(1);
	}

	ptr = find_mem_location(proc);
	if (ptr == 0) {
		printf("Could not find memory location\n");
		exit(1);
	} else {
		printf("Found memory location at: %016llx\n", ptr);
	}

	time_init = time(NULL);

	for(;;) {
		if (!ReadProcessMemory(proc, ptr, buffer, sizeof(buffer), &num_read)) {
			errmsg(msg, sizeof(msg));
			printf("ReadProcessMemory failed: %s\n", msg);
		} else {
			// if magic signature still exists, login is in progress, 
			// else the user is logged in
			if (*(uint64_t*)&buffer[4] == MAGIC_SIGNATURE) {
				ppl_ahead = *(uint32_t*)buffer;
				if (ppl_ahead != ppl_ahead_last) {
					if (ppl_ahead_last != 0xffffffff) {
						ppl_delta = ppl_ahead_last - ppl_ahead;
						time_delta = time(NULL) - time_last;
						time_est = (time_t)(((float)time_delta 	/ 
							(float)ppl_delta) * (float)ppl_ahead);
					}
					time_last = time(NULL);			
					ppl_ahead_last = ppl_ahead;
					snprintf(msg, sizeof(msg), 
						"there+are+still+%d+suckers+ahead. Est. time: %d min", 
						ppl_ahead, time_est);
					callmebot_send_message(msg);
					printf("%s\n", msg);
				}
			} else {
				callmebot_send_message("You+are+logged+in!");
				printf("User logged in, bailing out...\n");
				CloseHandle(proc);
				exit(0);
			}
		}
		// refresh every 30 seconds
		sleep(30);		
	}
}
