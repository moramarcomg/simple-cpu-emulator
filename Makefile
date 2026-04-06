CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -Iinclude
SRC     = src/main.c src/cpu.c src/execute.c
TARGET  = emulator

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
