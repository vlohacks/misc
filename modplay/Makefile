CC	= gcc
CXX	= g++
OBJ	= loader_mod.o loader_s3m.o loader.o module.o player.o effects_mod.o effects_s3m.o ui.o ui_terminal.o ui_ncurses.o main.o cmdline.o output_alsa.o output_portaudio.o player_command.o
BIN	= modplay
CCFLAGS = -Wall -g
#CCFLAGS	= -Wall -O3
#CXXFLAGS = -Wall -O3
#linux portaudio
LDFLAGS = libportaudio.a -lm -lpthread -lasound -lncurses
#linux alsa
#LDFLAGS = -lm -lpthread -lasound -lncurses
#win32
#LDFLAGS = -lm libportaudio.a -lwinmm

all: $(OBJ)
	$(CC) $(CCFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f $(BIN) $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c $<
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<
	

