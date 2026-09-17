CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -Iinclude
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
SRCS = $(wildcard src/*.cpp)
TARGET = bin/juego.exe

all: $(TARGET)

$(TARGET): $(SRCS) $(wildcard include/*.h)
	@if not exist bin mkdir bin
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

test: all
	$(TARGET) --self-test

run: all
	$(TARGET)

.PHONY: all test run

