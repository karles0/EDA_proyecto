"""Genera los graficos del benchmark a partir de resumen.csv.

Uso:  python3 herramientas/graficar.py [carpeta_resultados]
      (por defecto: resultados/; los PNG se guardan en resultados/graficos/)

Requiere solo matplotlib (ver herramientas/requirements.txt).
"""
import collections
import csv
import os
import statistics as st
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt              

COLOR = {"D-tree": "#1f4e9e", "HKS": "#12806e"}
ESTRUCTURAS = ["D-tree", "HKS"]
OPERACIONES = [("eliminar", "Eliminar línea"), ("consultar", "Consultar conectividad"), ("insertar", "Restaurar línea")]

plt.rcParams.update({
    "figure.dpi": 130, "savefig.dpi": 160, "font.size": 10, "axes.titlesize": 11,
    "axes.spines.top": False, "axes.spines.right": False, "axes.grid": True,
    "grid.alpha": 0.3, "legend.frameon": False,
})

def leer(ruta):
    with open(ruta, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))

def main(carpeta):
    salida = os.path.join(carpeta, "graficos")
    os.makedirs(salida, exist_ok=True)
    resumen = leer(os.path.join(carpeta, "resumen.csv"))
    n_de = {f["caso"]: int(f["n"]) for f in resumen}
    casos = sorted(n_de, key=lambda c: n_de[c])

    rep = collections.defaultdict(list)
    for f in resumen:
        rep[(f["caso"], f["estructura"], f["operacion"])].append(f)
    def med(caso, est, op, campo):
        return st.median(float(f[campo]) for f in rep[(caso, est, op)])
    def rango(caso, est, op, campo):
        v = [float(f[campo]) for f in rep[(caso, est, op)]]
        return min(v), max(v)

    x = list(range(len(casos)))
    ancho = 0.38

    fig, ax = plt.subplots(1, 3, figsize=(13, 3.9))
    for k, (op, titulo) in enumerate(OPERACIONES):
        for j, e in enumerate(ESTRUCTURAS):
            y = [med(c, e, op, "media_us") for c in casos]
            lo_hi = [rango(c, e, op, "media_us") for c in casos]
            err = [[yy - lo for yy, (lo, _) in zip(y, lo_hi)], [hi - yy for yy, (_, hi) in zip(y, lo_hi)]]
            pos = [i + (j - 0.5) * ancho for i in x]
            ax[k].bar(pos, y, ancho, color=COLOR[e], label=e, yerr=err, capsize=2, error_kw={"lw": 0.8})
        ax[k].set_title(titulo)
        ax[k].set_xticks(x, casos, rotation=20)
        ax[k].set_ylabel("tiempo medio por operación (µs)")
    ax[0].legend()
    fig.suptitle("Contingencias N-2: tiempo medio por operación (mediana de 5 repeticiones; barras de error = mín–máx)", fontsize=10)
    fig.tight_layout()
    fig.savefig(os.path.join(salida, "2_tiempo_por_operacion.png"))
    plt.close(fig)

    fig, ax = plt.subplots(1, 2, figsize=(11, 4))
    ns = [n_de[c] for c in casos]
    for e in ESTRUCTURAS:
        ax[0].plot(ns, [med(c, e, "eliminar", "media_us") for c in casos], "o-", color=COLOR[e], label=f"{e}: media")
        ax[0].plot(ns, [med(c, e, "eliminar", "p99_us") for c in casos], "s--", color=COLOR[e], alpha=0.7, label=f"{e}: percentil 99")
    ax[0].set_xscale("log"); ax[0].set_yscale("log")
    ax[0].set_title("Eliminar línea: crecimiento con el tamaño de la red")
    ax[0].set_xlabel("barras (n)"); ax[0].set_ylabel("µs")
    ax[0].legend(fontsize=8)
    razon_media = [med(c, "HKS", "eliminar", "media_us") / med(c, "D-tree", "eliminar", "media_us") for c in casos]
    razon_p99 = [med(c, "HKS", "eliminar", "p99_us") / med(c, "D-tree", "eliminar", "p99_us") for c in casos]
    ax[1].plot(ns, razon_media, "o-", color="#17222d", label="media")
    ax[1].plot(ns, razon_p99, "s--", color="#c8156f", label="percentil 99")
    ax[1].axhline(1.0, color="#6f7f8d", lw=1)
    ax[1].text(ns[0], 1.08, "por debajo de 1, HKS sería más rápido", fontsize=8, color="#5a6875")
    ax[1].set_xscale("log")
    ax[1].set_ylim(0, max(razon_media) * 1.15)
    ax[1].set_title("Cuántas veces más lento es HKS al eliminar")
    ax[1].set_xlabel("barras (n)"); ax[1].set_ylabel("tiempo HKS / tiempo D-tree")
    ax[1].legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(os.path.join(salida, "3_escalabilidad.png"))
    plt.close(fig)

    print(f"Gráficos guardados en {salida}/")
    for nombre in sorted(os.listdir(salida)):
        print("  ", nombre)

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "resultados")
