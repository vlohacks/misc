CC	= gcc
OBJ	= osc.o filter_moog.o test.o note.o
BIN	= test
CCFLAGS = -Wall -O2
LDFLAGS = -lm

all: $(OBJ)
	$(CC) $(CCFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

.PHONY: clean
clean: 
	rm -f $(BIN) $(OBJ)

%.o: %.c
	$(CC) $(CCFLAGS) -c $<





