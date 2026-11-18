#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include "CampoMinado.h"

using namespace std;

int main() {
    vector<string> modos = {"naive", "size", "path_comp", "both"};
    vector<int> tamanhos_grid = {32, 64, 128, 256, 512};
    vector<double> probabilidades_mina = {0.10, 0.15, 0.20, 0.25}; // 10%, 15%, 20% e 25% de minas

    const int NUM_EXECUCOES = 30;
    const string nome_arquivo_csv = "resultados_campo_minado.csv";

    ofstream arquivo_csv(nome_arquivo_csv);
    if (!arquivo_csv.is_open()) {
        cerr << "Erro ao criar o arquivo CSV!\n";
        return 1;
    }

    arquivo_csv << "modo,largura_grid,total_celulas,prob_mina,tempo_medio_s,acessos_memoria_medios,altura_media,altura_maxima_media\n";

    cout << fixed << setprecision(6);
    cout << "BENCHMARK DE CONJUNTO DISJUNTO - CENARIO CAMPO MINADO\n";
    cout << "---------------------------------------------------------------------------------\n";
    cout << left << setw(12) << "Modo" 
              << setw(10) << "Grid"
              << setw(12) << "Minas (%)"
              << setw(15) << "Tempo Med (s)" 
              << setw(18) << "Acessos Memoria" 
              << setw(15) << "Altura Media"
              << setw(15) << "Altura Max\n";
    cout << "---------------------------------------------------------------------------------\n";

    for (double prob_mina : probabilidades_mina) {
        for (int tamanho : tamanhos_grid) {
            for (const string& modo : modos) {
                double soma_tempo = 0.0;
                long long soma_acessos = 0;
                double soma_altura_media = 0.0;
                double soma_altura_max = 0.0;

                for (int exec = 0; exec < NUM_EXECUCOES; ++exec) {
                    unsigned int semente_atual = 42 + exec;
                    ResultadoSimulacao res = executar_simulacao_campo_minado(
                        tamanho, tamanho, prob_mina, modo, semente_atual
                    );

                    soma_tempo += res.tempo_segundos;
                    soma_acessos += res.acessos_memoria;
                    soma_altura_media += res.altura_media;
                    soma_altura_max += res.altura_maxima;
                }

                double tempo_medio = soma_tempo / NUM_EXECUCOES;
                long long acessos_medios = soma_acessos / NUM_EXECUCOES;
                double altura_media_final = soma_altura_media / NUM_EXECUCOES;
                double altura_max_final = soma_altura_max / NUM_EXECUCOES;
                int total_celulas = tamanho * tamanho;

                // Exibe no terminal
                cout << left << setw(12) << modo
                          << setw(10) << (to_string(tamanho) + "x" + to_string(tamanho))
                          << setw(12) << (int)(prob_mina * 100)
                          << setw(15) << tempo_medio
                          << setw(18) << acessos_medios
                          << setw(15) << altura_media_final
                          << setw(15) << altura_max_final << "\n";

                // Salva a linha no CSV
                arquivo_csv << modo << ","
                            << tamanho << ","
                            << total_celulas << ","
                            << prob_mina << ","
                            << fixed << setprecision(8) << tempo_medio << ","
                            << acessos_medios << ","
                            << setprecision(6) << altura_media_final << ","
                            << setprecision(2) << altura_max_final << "\n";
            }
            cout << "--------------------------------------------------------------------------------------------\n";
        }
    }

    return 0;
}