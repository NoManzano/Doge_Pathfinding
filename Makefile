
CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude
LIBS = -lsfml-graphics -lsfml-window -lsfml-system


SRC_DIR = src
BIN_DIR = bin
TARGET = $(BIN_DIR)/juego.exe


SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)


all: create_bin $(TARGET)

create_bin:
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)


$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $(TARGET)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)


run: all
	./$(TARGET)