IDIR =../include

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c99 -I$(IDIR)

ODIR=obj
LDIR =../lib

# Linker flags
LDFLAGS = -LC:/raylib/w64devkit/x86_64-w64-mingw32/lib -lraylib -lgdi32 -lwinmm

_DEPS = globals.h game.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o game.o globals.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Executable name
TARGET = game

# Default target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: all clean run

clean:
	rm -f $(ODIR)/*.o *.exe

# Run the program
run: $(TARGET)
	IF EXIST $(TARGET).exe $(TARGET).exe
