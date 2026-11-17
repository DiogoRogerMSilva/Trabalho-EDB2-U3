"""
plot_results.py - gera os graficos do artigo a partir do CSV do benchmark.

Uso:
    python plot_results.py                          # le results_maze.csv
    python plot_results.py results_maze.csv         # ou outro arquivo
    python plot_results.py a.csv b.csv --out figs   # varios CSVs (ex.: maze + minesweeper)

Requer:  pip install pandas matplotlib

Para cada `scenario` presente no CSV (maze, minesweeper, ...), gera 4 figuras:
    <scenario>_tempo.png    tempo (mediana, ms) vs tamanho do grid
    <scenario>_acessos.png  acessos a memoria vs tamanho do grid
    <scenario>_altura.png   altura maxima da arvore vs tamanho do grid
    <scenario>_profundidade.png  profundidade media dos nos vs tamanho do grid
O eixo x mostra o grid como 32x32, 64x64, ... (escala log base 2).
e um resumo em <scenario>_resumo.csv (media e desvio por variacao e tamanho).

Cada ponto e a MEDIA entre as seeds; a barra de erro e o desvio padrao.
"""
import argparse
import os
import sys

import pandas as pd
import matplotlib
import matplotlib.ticker

matplotlib.use("Agg")  # nao precisa de janela; salva direto em arquivo
import matplotlib.pyplot as plt

# Rotulos legiveis para as 4 variacoes (a ordem define a ordem na legenda)
VARIANTS = [
    ("V1_sem_otimizacao", "V1 - sem otimização", "#d62728", "o"),
    ("V2_uniao_por_tamanho", "V2 - união por tamanho", "#1f77b4", "s"),
    ("V3_compressao_caminho", "V3 - compressão de caminho", "#2ca02c", "^"),
    ("V4_tamanho_mais_compressao", "V4 - tamanho + compressão", "#9467bd", "D"),
]

METRICS = [
    # (arquivo, coluna, titulo, rotulo do eixo y)
    ("tempo", "time_ms_median", "Tempo de execução", "Tempo (ms, mediana)"),
    ("acessos", "mem_accesses", "Acessos à memória (parent[] e size[])", "Acessos à memória"),
    ("altura", "final_max_height", "Altura máxima das árvores", "Altura máxima"),
    ("profundidade", "mean_node_depth", "Profundidade média dos nós", "Profundidade média"),
]


def carregar(paths):
    frames = []
    for p in paths:
        if not os.path.exists(p):
            sys.exit(f"Arquivo nao encontrado: {p}")
        frames.append(pd.read_csv(p))
    return pd.concat(frames, ignore_index=True)


def resumir(df):
    """Media e desvio padrao entre seeds, por (variacao, numero de celulas)."""
    cols = [m[1] for m in METRICS]
    g = df.groupby(["variant", "width", "height", "cells"])[cols]
    mean = g.mean().add_suffix("_mean")
    std = g.std(ddof=0).fillna(0).add_suffix("_std")
    n = g.size().rename("n_seeds")
    return pd.concat([mean, std, n], axis=1).reset_index()


def plotar(resumo, scenario, outdir):
    larguras = sorted(resumo["width"].unique())
    rotulos = []
    for w in larguras:
        h = int(resumo[resumo["width"] == w]["height"].iloc[0])
        rotulos.append(f"{int(w)}×{h}")

    # Variacoes que nao foram executadas em todos os tamanhos (ex.: V1 nos grids maiores)
    avisos = []
    for key, label, color, marker in VARIANTS:
        d = resumo[resumo["variant"] == key]
        if not d.empty and d["width"].max() < larguras[-1]:
            avisos.append(f"{label.split(' - ')[0]} não executada acima de {int(d['width'].max())}×{int(d['width'].max())}")

    for fname, col, titulo, ylabel in METRICS:
        fig, ax = plt.subplots(figsize=(7.5, 4.8))
        for key, label, color, marker in VARIANTS:
            d = resumo[resumo["variant"] == key].sort_values("width")
            if d.empty:
                continue
            ax.errorbar(
                d["width"], d[col + "_mean"], yerr=d[col + "_std"],
                label=label, color=color, marker=marker, markersize=5,
                linewidth=1.6, capsize=3,
            )
        ax.set_xscale("log", base=2)
        ax.set_yscale("log")
        ax.set_xticks(larguras)
        ax.set_xticklabels(rotulos, fontsize=8)
        ax.minorticks_off()
        ax.yaxis.set_minor_locator(matplotlib.ticker.LogLocator(subs="auto"))
        ax.set_xlabel("Tamanho do grid")
        ax.set_ylabel(ylabel)
        ax.set_title(f"{titulo} - cenário: {scenario}")
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize=8)
        if avisos:
            fig.text(0.01, 0.01, "; ".join(avisos) + " (custo proibitivo).", fontsize=7, color="#555555")
            fig.tight_layout(rect=(0, 0.04, 1, 1))
        else:
            fig.tight_layout()
        path = os.path.join(outdir, f"{scenario}_{fname}.png")
        fig.savefig(path, dpi=200)
        plt.close(fig)
        print("  gerado:", path)


def main():
    ap = argparse.ArgumentParser(description="Graficos do benchmark de union-find")
    ap.add_argument("csv", nargs="*", default=["results_maze.csv"])
    ap.add_argument("--out", default="graficos", help="pasta de saida (padrao: graficos)")
    args = ap.parse_args()

    df = carregar(args.csv)
    os.makedirs(args.out, exist_ok=True)

    for scenario, dfs in df.groupby("scenario"):
        print(f"Cenario: {scenario}  ({len(dfs)} linhas)")
        resumo = resumir(dfs)
        resumo_path = os.path.join(args.out, f"{scenario}_resumo.csv")
        resumo.to_csv(resumo_path, index=False)
        print("  gerado:", resumo_path)
        plotar(resumo, scenario, args.out)

    print("\nPronto. Figuras em:", os.path.abspath(args.out))


if __name__ == "__main__":
    main()
