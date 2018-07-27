#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char ** argv) 
{
	DWORD pid = GetCurrentProcessId();
	for (;;) {
		std::cout << pid << " is sleeping..." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}




