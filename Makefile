CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRCDIR = .
AUXDIR = auxiliares
FUNCDIR = funcionalidades
TARGET = programaTrab

SOURCES = $(SRCDIR)/programaTrab.c \
          $(AUXDIR)/auxiliar.c \
          $(AUXDIR)/criterios.c \
          $(AUXDIR)/fornecidas.c \
          $(FUNCDIR)/func01.c \
          $(FUNCDIR)/func02.c \
          $(FUNCDIR)/func03.c \
          $(FUNCDIR)/func04.c \
          $(FUNCDIR)/func05.c \
          $(FUNCDIR)/func06.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

ifeq ($(OS),Windows_NT)
clean:
	-del /F /Q $(subst /,\,$(OBJECTS)) $(TARGET) $(TARGET).exe 2>nul
else
clean:
	rm -f $(OBJECTS) $(TARGET)
endif

.PHONY: all run clean