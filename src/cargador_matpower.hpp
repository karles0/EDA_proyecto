
#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

struct GrafoRed {
    int n = 0;
    vector<long long> barra_id;
    unordered_map<long long, int> indice;
    vector<pair<int, int>> aristas;

    int ramas_totales = 0;
    int ramas_fuera_servicio = 0;
    int lazos = 0;
    int paralelas = 0;
};

GrafoRed cargar_matpower(const string& ruta);

void escribir_lista_aristas(const GrafoRed& g, const string& ruta);
