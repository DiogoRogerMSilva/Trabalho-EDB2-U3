CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall

TARGET = benchmark_campo_minado

SRC = main.cpp
HDR = CampoMinado.h ConjuntoDisjunto.h

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean