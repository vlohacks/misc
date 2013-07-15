CC	= gcc
OBJ	= loader_mod.o module.o player.o effects_mod.o protracker.o ui.o ui_terminal.o main.o cmdline.o output_portaudio.o
BIN	= modplay
#CCFLAGS = -Wall -g
CCFLAGS	= -Wall -O3
#linux
LDFLAGS = -lm libportaudio.a -lpthread -lasound
#win32
#LDFLAGS = -lm libportaudio.a -lwinmm

all: $(OBJ)
	$(CC) $(CCFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f $(BIN) $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c $<





