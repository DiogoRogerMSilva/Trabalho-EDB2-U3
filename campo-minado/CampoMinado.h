#ifndef CAMPO_MINADO_H
#define CAMPO_MINADO_H

#include "ConjuntoDisjunto.h"
#include <vector>
#include <chrono>
#include <random>
#include <string>

using namespace std;

struct ResultadoSimulacao {
    string modo;
    double tempo_segundos;
    long long acessos_memoria;
    double altura_media;
    int altura_maxima;
};

inline ResultadoSimulacao executar_simulacao_campo_minado(int largura_grid, int altura_grid, double probabilidade_mina, const string& modo) {
    int total_celulas = largura_grid * altura_grid;
    ConjuntoDisjuntoGradeInstrumentado ds(total_celulas, modo);

    // Gera o tabuleiro (0 = seguro, 1 = mina)
    vector<vector<int>> grid(altura_grid, vector<int>(largura_grid));
    mt19937 gerador(42); // Semente fixa para reprodutibilidade
    uniform_real_distribution<> distribuicao(0.0, 1.0);

    for (int r = 0; r < altura_grid; ++r) {
        for (int c = 0; c < largura_grid; ++c) {
            grid[r][c] = (distribuicao(gerador) < probabilidade_mina) ? 1 : 0;
        }
    }

    auto inicio = chrono::high_resolution_clock::now();

    // Varre o grid unindo células seguras adjacentes (Direita e Abaixo)
    for (int r = 0; r < altura_grid; ++r) {
        for (int c = 0; c < largura_grid; ++c) {
            if (grid[r][c] == 1) continue; // Pula minas

            int id_atual = r * largura_grid + c;

            // Vizinho da direita
            if (c + 1 < largura_grid && grid[r][c + 1] == 0) {
                int id_direita = r * largura_grid + (c + 1);
                ds.unir(id_atual, id_direita);
            }
            // Vizinho de baixo
            if (r + 1 < altura_grid && grid[r + 1][c] == 0) {
                int id_baixo = (r + 1) * largura_grid + c;
                ds.unir(id_atual, id_baixo);
            }
        }
    }

    auto fim = chrono::high_resolution_clock::now();
    chrono::duration<double> tempo_decorrido = fim - inicio;

    auto estatisticas = ds.calcular_estatisticas_arvore();

    return {modo, tempo_decorrido.count(), ds.acessos_memoria, estatisticas.first, estatisticas.second};
}

#endif