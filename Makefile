ifeq ($(OS),Windows_NT)
	EXEC = programaTrab.exe
	RM = del /q /f
	RUN = programaTrab.exe
else
	EXEC = programaTrab
	RM = rm -f
	RUN = ./programaTrab
endif

all:
	gcc -Wall -std=c99 -o $(EXEC) *.c auxiliares/*.c funcionalidades/*.c -lm

run:
	$(RUN)

clean:
	$(RM) $(EXEC)