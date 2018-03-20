void _start() 
{
	asm (	"		xor %%eax, %%eax\n"
			"		mov $0x42, %%bl\n"
			"		mov $0x0a, %%cl\n"
			"loopa: int $0x30\n"
			"       push %%ecx\n"
			"		movl $0x00ffffff, %%ecx\n"
			"yoloa:	loop yoloa\n"
			"       pop %%ecx\n"
			"       jmp loopa"
			:
			:
			: "ecx", "eax", "bl", "cl"
		);
}
