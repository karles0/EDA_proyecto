
#pragma once

#include <cstddef>
#include <vector>

#include "adyacencia.hpp"

using namespace std;

class DTree {
public:
    explicit DTree(int n);

    bool insertar(int u, int v);

    bool eliminar(int u, int v);

    bool conectados(int u, int v);

    bool existe_arista(int u, int v) const;

    bool es_arista_arbol(int u, int v) const { return u != v && (padre_[u] == v || padre_[v] == u); }
    int num_vertices() const { return n_; }
    int num_componentes() const;
    long long num_aristas_arbol() const { return n_ - num_componentes(); }
    long long num_aristas_fuera() const { return nte_.num_aristas(); }
    size_t memoria_bytes() const;

private:
    int raiz(int x, int* profundidad = nullptr) const;
    int reroot(int w);
    int link(int n_u, int r_u, int r_v);
    int unlink(int n_v);
    void insertar_arista_arbol(int n_u, int n_v, int r_u, int r_v);
    void insertar_arista_no_arbol(int n_u, int n_v);
    void eliminar_arista_arbol(int n_u, int n_v);

    void agregar_hijo(int p, int c);
    void quitar_hijo(int p, int c);

    int n_;
    vector<int> padre_;
    vector<int> tam_;
    vector<vector<int>> hijos_;
    vector<int> pos_hijo_;
    AdyacenciaIndexada nte_;
};
