
#include "cargador_matpower.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

using namespace std;

namespace {

string limpiar_linea(const string& linea, bool& cierra) {
    string s = linea;
    const size_t com = s.find('%');
    if (com != string::npos) s.erase(com);
    cierra = false;
    const size_t cb = s.find(']');
    if (cb != string::npos) {
        cierra = true;
        s.erase(cb);
    }
    for (char& c : s)
        if (c == ';' || c == '\r' || c == ',') c = ' ';
    return s;
}

bool es_inicio_matriz(const string& linea, const string& nombre) {

    string s = linea;
    s.erase(remove_if(s.begin(), s.end(), [](unsigned char c) { return isspace(c); }), s.end());
    const string patron = "mpc." + nombre + "=[";
    return s.compare(0, patron.size(), patron) == 0;
}

vector<vector<double>> leer_matriz(istream& in, const string& nombre) {
    vector<vector<double>> filas;
    string linea;
    bool cierra = false;
    while (!cierra && getline(in, linea)) {
        istringstream ss(limpiar_linea(linea, cierra));
        vector<double> fila;
        double x;
        while (ss >> x) fila.push_back(x);
        if (!ss.eof())
            throw runtime_error("Valor no numerico en la matriz mpc." + nombre + ": " + linea);
        if (!fila.empty()) filas.push_back(move(fila));
    }
    if (!cierra) throw runtime_error("La matriz mpc." + nombre + " no se cierra con ']'");
    return filas;
}

}

GrafoRed cargar_matpower(const string& ruta) {
    ifstream in(ruta);
    if (!in) throw runtime_error("No se pudo abrir el archivo: " + ruta);

    vector<vector<double>> buses, ramas;
    bool hay_bus = false, hay_branch = false;
    string linea;
    while (getline(in, linea)) {
        if (!hay_bus && es_inicio_matriz(linea, "bus")) {
            buses = leer_matriz(in, "bus");
            hay_bus = true;
        } else if (!hay_branch && es_inicio_matriz(linea, "branch")) {
            ramas = leer_matriz(in, "branch");
            hay_branch = true;
        }
    }
    if (!hay_bus || !hay_branch)
        throw runtime_error("El archivo no contiene las matrices mpc.bus y mpc.branch: " + ruta);

    GrafoRed g;
    for (const auto& f : buses) {
        const long long id = static_cast<long long>(f[0]);
        if (g.indice.count(id)) throw runtime_error("Barra duplicada en mpc.bus: " + to_string(id));
        g.indice[id] = g.n++;
        g.barra_id.push_back(id);
    }

    unordered_set<uint64_t> vistas;
    for (const auto& f : ramas) {
        ++g.ramas_totales;
        if (f.size() < 2) throw runtime_error("Fila de mpc.branch con menos de 2 columnas");

        if (f.size() >= 11 && f[10] == 0.0) {
            ++g.ramas_fuera_servicio;
            continue;
        }
        const long long fb = static_cast<long long>(f[0]);
        const long long tb = static_cast<long long>(f[1]);
        auto itf = g.indice.find(fb), itt = g.indice.find(tb);
        if (itf == g.indice.end() || itt == g.indice.end())
            throw runtime_error("Rama que referencia una barra inexistente: " +
                                     to_string(fb) + " - " + to_string(tb));
        int u = itf->second, v = itt->second;
        if (u == v) {
            ++g.lazos;
            continue;
        }
        if (u > v) swap(u, v);
        const uint64_t k = (static_cast<uint64_t>(u) << 32) | static_cast<uint32_t>(v);
        if (!vistas.insert(k).second) {
            ++g.paralelas;
            continue;
        }
        g.aristas.emplace_back(u, v);
    }
    return g;
}

void escribir_lista_aristas(const GrafoRed& g, const string& ruta) {
    ofstream out(ruta);
    if (!out) throw runtime_error("No se pudo escribir: " + ruta);
    out << g.n << ' ' << g.aristas.size() << '\n';
    for (const auto& [u, v] : g.aristas)
        out << g.barra_id[static_cast<size_t>(u)] << ' ' << g.barra_id[static_cast<size_t>(v)] << '\n';
}
