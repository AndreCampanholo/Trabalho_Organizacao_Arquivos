EXEC = programaTrab

ifeq ($(OS),Windows_NT)
    EXEC := $(EXEC).exe
endif

CC = gcc
CFLAGS = -Wall -std=c99
LIBS = -lm

all:
	$(CC) $(CFLAGS) -o $(EXEC) *.c auxiliares/*.c funcionalidades/*.c $(LIBS)

run: all
	./$(EXEC)

clean:
	rm -f $(EXEC)

rebuild: clean all