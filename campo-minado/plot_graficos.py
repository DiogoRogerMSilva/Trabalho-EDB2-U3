import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Garante que o diretório de saída existe
output_dir = "graficos"
os.makedirs(output_dir, exist_ok=True)

df = pd.read_csv("resultados_campo_minado.csv")

# Estilos visuais padronizados com offset horizontal (dodge) para evitar sobreposição de curvas quase idênticas
config_estilos = {
    "naive": {
        "legenda": "Sem Otimização (Naive)",
        "cor": "#d62728",
        "marcador": "o",
        "facecolor": "#d62728",
        "edgecolor": "#8c1112",
        "linestyle": "-",
        "linewidth": 2.0,
        "markersize": 7,
        "offset_x_grid": 0.0,
        "offset_x_dens": 0.0
    },
    "path_comp": {
        "legenda": "Compressão de Caminho",
        "cor": "#ff7f0e",
        "marcador": "^",
        "facecolor": "#ff7f0e",
        "edgecolor": "#b35200",
        "linestyle": "-.",
        "linewidth": 2.0,
        "markersize": 7,
        "offset_x_grid": 0.0,
        "offset_x_dens": 0.0
    },
    "size": {
        "legenda": "União por Tamanho (Size)",
        "cor": "#1f77b4",
        "marcador": "s",
        "facecolor": "#9ecae1",
        "edgecolor": "#1f77b4",
        "linestyle": "--",
        "linewidth": 2.2,
        "markersize": 7.5,
        "offset_x_grid": -0.06,  # Pequeno deslocamento à esquerda para não sobrepor 'both'
        "offset_x_dens": -0.28
    },
    "both": {
        "legenda": "Ambas (Size + Path Comp.)",
        "cor": "#2ca02c",
        "marcador": "D",
        "facecolor": "#2ca02c",
        "edgecolor": "#004d1a",
        "linestyle": ":",
        "linewidth": 2.2,
        "markersize": 6.0,
        "offset_x_grid": +0.06,  # Pequeno deslocamento à direita para não sobrepor 'size'
        "offset_x_dens": +0.28
    }
}

modos = ["naive", "size", "path_comp", "both"]

metricas = [
    ("acessos_memoria_medios", "Total de Acessos", "Total de Acessos à Memória", "acessos"),
    ("altura_media", "Altura Média", "Altura Média das Árvores", "altura"),
    ("altura_maxima_media", "Profundidade Máxima", "Profundidade Máxima das Árvores", "profundidade"),
    ("tempo_medio_s", "Tempo Médio (s)", "Tempo Médio de Execução (s)", "tempo")
]

# ==============================================================================
# 1. ESCALABILIDADE (Fixando 15% de minas)
# ==============================================================================
df_15 = df[df["prob_mina"] == 0.15]
tamanhos = sorted(df_15["largura_grid"].unique())
labels_x = [f"{t}x{t}" for t in tamanhos]
pos_x = np.arange(len(tamanhos))

# 4 Gráficos Individuais de Escalabilidade
for col, ylabel, titulo, nome_curto in metricas:
    plt.figure(figsize=(7.8, 5.4))
    for modo in modos:
        sub = df_15[df_15["modo"] == modo].sort_values("largura_grid")
        cfg = config_estilos[modo]
        x_vals = pos_x + cfg["offset_x_grid"]
        plt.plot(
            x_vals,
            sub[col],
            label=cfg["legenda"],
            color=cfg["cor"],
            marker=cfg["marcador"],
            markerfacecolor=cfg["facecolor"],
            markeredgecolor=cfg["edgecolor"],
            markeredgewidth=1.3,
            linestyle=cfg["linestyle"],
            linewidth=cfg["linewidth"],
            markersize=cfg["markersize"],
            alpha=0.95
        )
    plt.title(f"{titulo} vs. Tamanho do Grid (15% Minas)", fontsize=12, fontweight="bold")
    plt.xlabel("Dimensão do Grid", fontsize=10)
    plt.ylabel(ylabel, fontsize=10)
    plt.xticks(pos_x, labels_x)
    plt.yscale("log")
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.legend(fontsize=9, loc="best")
    plt.tight_layout()
    caminho = os.path.join(output_dir, f"escalabilidade_{nome_curto}.png")
    plt.savefig(caminho, dpi=300)
    plt.close()
    print(f"Salvo: {caminho}")

# Gráfico Completo de Escalabilidade (Painel 2x2)
fig, axes = plt.subplots(2, 2, figsize=(14, 10))
for ax, (col, ylabel, titulo, _) in zip(axes.flatten(), metricas):
    for modo in modos:
        sub = df_15[df_15["modo"] == modo].sort_values("largura_grid")
        cfg = config_estilos[modo]
        x_vals = pos_x + cfg["offset_x_grid"]
        ax.plot(
            x_vals,
            sub[col],
            label=cfg["legenda"],
            color=cfg["cor"],
            marker=cfg["marcador"],
            markerfacecolor=cfg["facecolor"],
            markeredgecolor=cfg["edgecolor"],
            markeredgewidth=1.3,
            linestyle=cfg["linestyle"],
            linewidth=cfg["linewidth"],
            markersize=cfg["markersize"],
            alpha=0.95
        )
    ax.set_title(f"{titulo} vs. Tamanho do Grid", fontsize=11, fontweight="bold")
    ax.set_xlabel("Dimensão do Grid", fontsize=10)
    ax.set_ylabel(ylabel, fontsize=10)
    ax.set_xticks(pos_x)
    ax.set_xticklabels(labels_x)
    ax.set_yscale("log")
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.legend(fontsize=8, loc="best")

plt.tight_layout()
caminho_esc_comp = os.path.join(output_dir, "escalabilidade_completa.png")
plt.savefig(caminho_esc_comp, dpi=300)
plt.close()
print(f"Salvo: {caminho_esc_comp}")

# ==============================================================================
# 2. DENSIDADE DE MINAS (Grid fixo 512x512)
# ==============================================================================
df_512 = df[df["largura_grid"] == 512]

# 4 Gráficos Individuais de Densidade
for col, ylabel, titulo, nome_curto in metricas:
    plt.figure(figsize=(7.8, 5.4))
    for modo in modos:
        sub = df_512[df_512["modo"] == modo].sort_values("prob_mina")
        cfg = config_estilos[modo]
        x_vals = (sub["prob_mina"] * 100) + cfg["offset_x_dens"]
        plt.plot(
            x_vals,
            sub[col],
            label=cfg["legenda"],
            color=cfg["cor"],
            marker=cfg["marcador"],
            markerfacecolor=cfg["facecolor"],
            markeredgecolor=cfg["edgecolor"],
            markeredgewidth=1.3,
            linestyle=cfg["linestyle"],
            linewidth=cfg["linewidth"],
            markersize=cfg["markersize"],
            alpha=0.95
        )
    plt.title(f"{titulo} vs. Densidade de Minas (Grid 512x512)", fontsize=12, fontweight="bold")
    plt.xlabel("Densidade de Minas no Tabuleiro (%)", fontsize=10)
    plt.ylabel(ylabel, fontsize=10)
    plt.xticks([10, 15, 20, 25], ["10%", "15%", "20%", "25%"])
    plt.yscale("log")
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.legend(fontsize=9, loc="best")
    plt.tight_layout()
    caminho = os.path.join(output_dir, f"densidade_{nome_curto}.png")
    plt.savefig(caminho, dpi=300)
    plt.close()
    print(f"Salvo: {caminho}")

# Gráfico Completo de Densidade (Painel 2x2)
fig2, axes2 = plt.subplots(2, 2, figsize=(14, 10))
for ax, (col, ylabel, titulo, _) in zip(axes2.flatten(), metricas):
    for modo in modos:
        sub = df_512[df_512["modo"] == modo].sort_values("prob_mina")
        cfg = config_estilos[modo]
        x_vals = (sub["prob_mina"] * 100) + cfg["offset_x_dens"]
        ax.plot(
            x_vals,
            sub[col],
            label=cfg["legenda"],
            color=cfg["cor"],
            marker=cfg["marcador"],
            markerfacecolor=cfg["facecolor"],
            markeredgecolor=cfg["edgecolor"],
            markeredgewidth=1.3,
            linestyle=cfg["linestyle"],
            linewidth=cfg["linewidth"],
            markersize=cfg["markersize"],
            alpha=0.95
        )
    ax.set_title(f"{titulo} vs. Densidade de Minas", fontsize=11, fontweight="bold")
    ax.set_xlabel("Densidade de Minas no Tabuleiro (%)", fontsize=10)
    ax.set_ylabel(ylabel, fontsize=10)
    ax.set_xticks([10, 15, 20, 25])
    ax.set_xticklabels(["10%", "15%", "20%", "25%"])
    ax.set_yscale("log")
    ax.grid(True, which="both", linestyle="--", alpha=0.5)
    ax.legend(fontsize=8, loc="best")

plt.tight_layout()
caminho_dens_comp = os.path.join(output_dir, "densidade_completa.png")
plt.savefig(caminho_dens_comp, dpi=300)
plt.close()
print(f"Salvo: {caminho_dens_comp}")
