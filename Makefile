CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = programaTrab

SOURCES = programaTrab.c \
          auxiliares/auxiliar.c \
          auxiliares/criterios.c \
          auxiliares/fornecidas.c \
          funcionalidades/func01.c \
          funcionalidades/func02.c \
          funcionalidades/func03.c \
          funcionalidades/func04.c \
          funcionalidades/func05.c \
          funcionalidades/func06.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe nul

.PHONY: all run clean
