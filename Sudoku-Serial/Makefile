CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -fopenmp

TARGET = sudoku-serial
SRCS = main.cpp grid.cpp rules.cpp solver.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm sudoku-serial
	del /Q $(TARGET).exe 2>nul || exit 0

# TODO(Build): adicionar target para testes unitarios por modulo.
