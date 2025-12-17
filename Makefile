# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Included libraries
LIBS = -lSDL2

# Directories
SDIR = src
IDIR = include
BDIR = build

# Files
SRCS = $(SDIR)/chipOS.c $(SDIR)/utils.c
# Convert src/name.c to build/name.o
OBJS = $(patsubst $(SDIR)/%.c, $(BDIR)/%.o, $(SRCS))

# Target executable name
TARGET = chip_os

# Link all object files to create the final program
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

# Compile each .c file into the build/ folder as a .o file
$(BDIR)/%.o: $(SDIR)/%.c
	@mkdir -p $(BDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

# Cleanup
.PHONY: clean
clean:
	rm -rf $(BDIR) $(TARGET)