void _start() 
{
	asm volatile("xor %eax, %eax; div %eax, %eax");
}
