# Compiler and flags
CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0 gstreamer-1.0`
LDFLAGS = `pkg-config --libs gtk+-3.0 gstreamer-1.0` -lm
# Target executable name
TARGET = gtkball
# Source files
SOURCES = gtkball.c
# Object files
OBJECTS = $(SOURCES:.c=.o)
# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
    $(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
    rm -f $(OBJECTS) $(TARGET)

# Install dependencies (requires sudo)
deps:
    sudo apt-get update
    sudo apt-get install -y libgtk-3-dev libgstreamer1.0-dev

# Run the program
run: $(TARGET)
    ./$(TARGET)

# Phony targets
.PHONY: all clean deps run
