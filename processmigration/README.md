### Process injection / migration examples
This explains some windows process injection techniques by sourcecode 

## filez
inject.cpp: injects a bind shell (listening on port 31337) into target process
hostprocess.cpp: just a dummy hostprocess as more comprehensive example to notepad.exe ;-)

## usage
* compile inject.cpp: `mingw-w64-g++ -o inject.exe inject.cpp`
* [optional] complile hostprocess.cpp: `mingw-w64-g++ -o hostprocess.exe hostprocess.cpp`
* run a desired process (or hostprocess.exe), get it's pid
* run `inject.exe <pid>`, where <pid> is the pid of coz
* nc/telnet/... to bindshell listening on the box
* jerkoff

## disclaimer
This is for educational purposes only. if you do shit, i'm not gonna visit yo in jail.
Also windoze snakeoil viri shitware might cry because of msfvenom payload ^^


