
#include "dtree.hpp"

#include <cassert>
#include <climits>

using namespace std;

DTree::DTree(int n)
    : n_(n),
      padre_(static_cast<size_t>(n), -1),
      tam_(static_cast<size_t>(n), 1),
      hijos_(static_cast<size_t>(n)),
      pos_hijo_(static_cast<size_t>(n), -1),
      nte_(n) {}

void DTree::agregar_hijo(int p, int c) {
    padre_[c] = p;
    pos_hijo_[c] = static_cast<int>(hijos_[p].size());
    hijos_[p].push_back(c);
}

void DTree::quitar_hijo(int p, int c) {
    auto& h = hijos_[p];
    const int i = pos_hijo_[c];
    assert(i >= 0 && i < static_cast<int>(h.size()) && h[i] == c);
    const int ultimo = h.back();
    h[i] = ultimo;
    pos_hijo_[ultimo] = i;
    h.pop_back();
    pos_hijo_[c] = -1;
    padre_[c] = -1;
}

int DTree::raiz(int x, int* profundidad) const {
    int d = 0;
    while (padre_[x] != -1) {
        x = padre_[x];
        ++d;
    }
    if (profundidad) *profundidad = d;
    return x;
}

bool DTree::existe_arista(int u, int v) const {
    if (u == v) return false;
    return padre_[u] == v || padre_[v] == u || nte_.contiene(u, v);
}

int DTree::num_componentes() const {
    int c = 0;
    for (int v = 0; v < n_; ++v)
        if (padre_[v] == -1) ++c;
    return c;
}

int DTree::reroot(int w) {
    if (padre_[w] == -1) return w;

    int ch = w;
    int cur = padre_[w];
    quitar_hijo(cur, w);
    while (cur != -1) {
        const int g = padre_[cur];
        if (g != -1) quitar_hijo(g, cur);
        agregar_hijo(ch, cur);
        ch = cur;
        cur = g;
    }

    while (padre_[ch] != -1) {
        const int p = padre_[ch];
        tam_[ch] -= tam_[p];
        tam_[p] += tam_[ch];
        ch = p;
    }
    return w;
}

int DTree::link(int n_u, int r_u, int r_v) {
    assert(padre_[r_v] == -1 && padre_[r_u] == -1);
    agregar_hijo(n_u, r_v);
    const int total = tam_[r_u] + tam_[r_v];
    int nueva_raiz = -1;
    for (int c = n_u; c != -1; c = padre_[c]) {
        tam_[c] += tam_[r_v];
        if (nueva_raiz == -1 && 2 * tam_[c] > total) nueva_raiz = c;
    }
    if (nueva_raiz != -1 && nueva_raiz != r_u) r_u = reroot(nueva_raiz);
    return r_u;
}

int DTree::unlink(int n_v) {
    assert(padre_[n_v] != -1);
    int c = n_v;
    while (padre_[c] != -1) {
        c = padre_[c];
        tam_[c] -= tam_[n_v];
    }
    quitar_hijo(padre_[n_v], n_v);
    return c;
}

bool DTree::conectados(int u, int v) {
    int r_u = u, c_u = -1;
    while (padre_[r_u] != -1) {
        c_u = r_u;
        r_u = padre_[r_u];
    }
    if (c_u != -1 && 2 * tam_[c_u] > tam_[r_u]) r_u = reroot(c_u);

    int r_v = v, c_v = -1;
    while (padre_[r_v] != -1) {
        c_v = r_v;
        r_v = padre_[r_v];
    }

    const bool resultado = (r_u == r_v);
    if (c_v != -1 && 2 * tam_[c_v] > tam_[r_v]) reroot(c_v);
    return resultado;
}

void DTree::insertar_arista_arbol(int n_u, int n_v, int r_u, int r_v) {
    if (tam_[r_u] < tam_[r_v])
        link(n_v, r_v, reroot(n_u));
    else
        link(n_u, r_u, reroot(n_v));
}

void DTree::insertar_arista_no_arbol(int n_u, int n_v) {
    int d_u = 0, d_v = 0;
    const int r = raiz(n_u, &d_u);
    raiz(n_v, &d_v);

    int l, h;
    if (d_u <= d_v) { l = n_u; h = n_v; }
    else            { l = n_v; h = n_u; }
    const int delta = (d_u <= d_v) ? d_v - d_u : d_u - d_v;

    if (delta < 2) {
        nte_.agregar(n_u, n_v);
        return;
    }
    int c = h;
    for (int i = 1; i <= delta - 2; ++i) c = padre_[c];

    nte_.agregar(c, padre_[c]);
    unlink(c);
    link(l, r, reroot(h));
}

void DTree::eliminar_arista_arbol(int n_u, int n_v) {
    const int ch = (padre_[n_v] == n_u) ? n_v : n_u;
    const int r = unlink(ch);

    int r_s, r_l;
    if (tam_[ch] < tam_[r]) { r_s = ch; r_l = r; }
    else                    { r_s = r;  r_l = ch; }

    const int S = tam_[r_s];
    int candidato_centroide = -1;
    int mejor_x = -1, mejor_y = -1, mejor_prof = INT_MAX;

    vector<int> cola;
    cola.reserve(static_cast<size_t>(S));
    cola.push_back(r_s);
    for (size_t i = 0; i < cola.size(); ++i) {
        const int x = cola[i];
        if (x != r_s && candidato_centroide == -1 && 2 * tam_[x] > S) candidato_centroide = x;
        for (const int y : nte_.vecinos(x)) {
            int prof = 0;
            if (raiz(y, &prof) == r_s) continue;
            if (prof < mejor_prof) {
                mejor_prof = prof;
                mejor_x = x;
                mejor_y = y;
            }
        }
        for (const int c : hijos_[x]) cola.push_back(c);
    }

    if (mejor_x == -1) {
        if (candidato_centroide != -1) reroot(candidato_centroide);
        return;
    }
    nte_.quitar(mejor_x, mejor_y);
    insertar_arista_arbol(mejor_x, mejor_y, r_s, r_l);
}

bool DTree::insertar(int u, int v) {
    assert(u >= 0 && u < n_ && v >= 0 && v < n_);
    if (u == v || existe_arista(u, v)) return false;

    if (conectados(u, v)) {
        insertar_arista_no_arbol(u, v);
    } else {
        const int r_u = raiz(u), r_v = raiz(v);
        insertar_arista_arbol(u, v, r_u, r_v);
    }
    return true;
}

bool DTree::eliminar(int u, int v) {
    assert(u >= 0 && u < n_ && v >= 0 && v < n_);
    if (u == v) return false;
    if (padre_[u] == v || padre_[v] == u) {
        eliminar_arista_arbol(u, v);
        return true;
    }
    if (nte_.contiene(u, v)) {
        nte_.quitar(u, v);
        return true;
    }
    return false;
}

size_t DTree::memoria_bytes() const {
    size_t b = sizeof(*this);
    b += padre_.capacity() * sizeof(int) + tam_.capacity() * sizeof(int) + pos_hijo_.capacity() * sizeof(int);
    b += hijos_.capacity() * sizeof(vector<int>);
    for (const auto& h : hijos_) b += h.capacity() * sizeof(int);
    b += nte_.memoria_bytes() - sizeof(nte_);
    return b;
}
