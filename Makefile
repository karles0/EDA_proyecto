# Makefile - Conectividad dinamica: D-tree y Euler Tour Tree HKS
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Wshadow
SRC_COMUN = src/dtree.cpp src/ett_hks.cpp src/cargador_matpower.cpp
CABECERAS = src/adyacencia.hpp src/dtree.hpp src/ett_hks.hpp src/cargador_matpower.hpp

all: principal benchmark

principal: src/main.cpp $(SRC_COMUN) $(CABECERAS)
	$(CXX) $(CXXFLAGS) src/main.cpp $(SRC_COMUN) -o $@

# Benchmark: siempre sin aserciones para medir solo los algoritmos
benchmark: src/benchmark.cpp $(SRC_COMUN) $(CABECERAS)
	$(CXX) -std=c++17 -O2 -DNDEBUG -Wall -Wextra src/benchmark.cpp $(SRC_COMUN) -o $@

run: principal
	./principal data/ACTIVSg2000.m

clean:
	rm -f principal benchmark data/*_aristas.txt

medir: benchmark
	./benchmark --salida resultados

graficos:
	python3 herramientas/graficar.py resultados

.PHONY: all run medir graficos clean
