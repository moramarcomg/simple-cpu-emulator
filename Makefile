CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
SRC      = src/main.cpp src/cpu.cpp

# Detecta si estamos en Windows (MinGW) o Linux/Mac
ifeq ($(OS), Windows_NT)
    TARGET = emulator.exe
    CLEAN  = del /Q $(TARGET)
else
    TARGET = emulator
    CLEAN  = rm -f $(TARGET)
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	$(CLEAN)

.PHONY: all clean