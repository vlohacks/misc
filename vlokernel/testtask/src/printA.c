void _start() 
{
	asm (	"		xor %%eax, %%eax\n"
			"		mov $0x41, %%bl\n"
			"		mov $0x09, %%cl\n"
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
