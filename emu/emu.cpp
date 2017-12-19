#include <cstdio>
#include <cstdlib>
#include <functional>

/*
instruction set:
j 	imm	iiii0000	
jz	imm	iiii0001
jnz	imm	iiii0010
		iiii0011
ldal	imm	iiii0100
ldah	imm	iiii0101
nop		xxxx0110
hlt		xxxx0111

add	a, b	aabb1000
xor	a, b	aabb1010
mov	a, b	aabb1011
ld	a, b	aabb1100
st	a, b	aabb1101
out	a	aabb1001
inc	a	aaxx1110
dec	a	aaxx1111
*/

class Cpu 
{
private:
	typedef unsigned char reg_t;
	bool zflag;
	reg_t ip;
	reg_t r[4];
	reg_t imm;
	reg_t *a, *b, *m;
	std::function<void()> ops[16];
	void op_j()    {*a += (*b&8?8-*b:*b);}
	void op_jz()   {if(zflag){*a += (*b&8?8-*b:*b);}}
	void op_jnz()  {if(zflag){return;} *a += (*b&8?8-*b:*b);}
	void op_ldal() {*a &= 0xf0; *a |= *b;}
	void op_ldah() {*a &= 0x0f; *a |= ((*b)<<4);}
	void op_nop()  {}
	void op_hlt()  {exit(0);}
	void op_add()  {*a += *b;zflag=(*a==0);}
	void op_xor()  {*a ^= *b;zflag=(*a==0);}
	void op_mov()  {*a = *b;}
	void op_ld()   {*a = m[*b];}
	void op_st()   {m[*b] = *a;}
	void op_out()  {putc(*a, stdout);}
	void op_inc()  {(*a)++;zflag=(*a==0);}
	void op_dec()  {(*a)++;zflag=(*a==0);}

public:
	Cpu(unsigned char* data) 
		: m(static_cast<reg_t*>(data))
		, zflag(false)
	{
		ops[0]  = std::bind(&Cpu::op_j,    this);
		ops[1]  = std::bind(&Cpu::op_jz,   this);
		ops[2]  = std::bind(&Cpu::op_jnz,  this);
		ops[3]  = std::bind(&Cpu::op_nop,  this);
		ops[4]  = std::bind(&Cpu::op_ldal, this);
		ops[5]  = std::bind(&Cpu::op_ldah, this);
		ops[6]  = std::bind(&Cpu::op_nop,  this);
		ops[7]  = std::bind(&Cpu::op_hlt,  this);
		ops[8]  = std::bind(&Cpu::op_add,  this);
		ops[9]  = std::bind(&Cpu::op_xor,  this);
		ops[10] = std::bind(&Cpu::op_mov,  this);
		ops[11] = std::bind(&Cpu::op_ld,   this);
		ops[12] = std::bind(&Cpu::op_st,   this);
		ops[13] = std::bind(&Cpu::op_out,  this);
		ops[14] = std::bind(&Cpu::op_inc,  this);
		ops[15] = std::bind(&Cpu::op_dec,  this);
	}
	~Cpu()
	{}
	void exec() 
	{
		imm=m[ip]>>4;
		a=m[ip]&8 ? &r[(m[ip]&192)>>6]  : m[ip]&4?&ip:&r[0];
		b=m[ip]&8 ? &r[(m[ip]&48)>>4]   : &imm;
		ops[m[ip]&15]();
		ip++;
	}

};
