void _start() 
{
	void (*fuckup)() = (void (*))"\xfe\x20\x20\x20\xde\xad\xbe\xef";
	for (;;)
		fuckup();
}
