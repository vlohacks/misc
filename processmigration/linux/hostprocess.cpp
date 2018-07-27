#include <iostream>
#include <thread>
#include <chrono>
#include <unistd.h>

int main(int argc, char ** argv) 
{
	pid_t pid = getpid();
	for (;;) {
		std::cout << pid << " is sleeping..." << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}




