CXX ?= g++
CXXFLAGS ?= -O2 -std=c++14 -Wall -Wextra

benchmark_maze: benchmark_maze.cpp disjoint_set.hpp maze_kruskal.hpp
	$(CXX) $(CXXFLAGS) -o $@ benchmark_maze.cpp

clean:
	rm -f benchmark_maze results_maze.csv
