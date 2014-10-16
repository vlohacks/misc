CC	= gcc
CXX	= g++
OBJ	= podfile.o tri_mesh.o main.o 
BIN	= tri

#CCFLAGS = -I/mingw/include -Wall -g
#CCFLAGS	= -Wall -O3
CCFLAGS	= -Wall -g
#CXXFLAGS = -Wall -O3
#linux 
LDFLAGS = -lSDL2
#linux alsa
#LDFLAGS = -lm -lpthread -lasound -lncurses
#win32
#LDFLAGS = 

all: $(OBJ)
	$(CC) $(CCFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f $(BIN) $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c $<
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<
	

