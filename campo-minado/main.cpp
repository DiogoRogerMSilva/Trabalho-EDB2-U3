#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include "CampoMinado.h"

using namespace std;

int main() {
    vector<string> modos = {"naive", "size", "path_comp", "both"};
    vector<int> tamanhos_grid = {32, 64, 128, 256, 512}; // Tamanhos progressivos para os testes
    double prob_mina = 0.15; // 15% de minas

    cout << fixed << setprecision(6);
    cout << "BENCHMARK DE CONJUNTO DISJUNTO - CENARIO CAMPO MINADO (EM PORTUGUES)\n";
    cout << "---------------------------------------------------------------------------------\n";
    cout << left << setw(15) << "Modo" 
              << setw(12) << "Grid" 
              << setw(15) << "Tempo (s)" 
              << setw(20) << "Acessos Memoria" 
              << setw(15) << "Altura Media\n";
    cout << "---------------------------------------------------------------------------------\n";

    for (int tamanho : tamanhos_grid) {
        for (const string& modo : modos) {
            ResultadoSimulacao res = executar_simulacao_campo_minado(tamanho, tamanho, prob_mina, modo);
            cout << left << setw(15) << res.modo 
                      << setw(12) << (to_string(tamanho) + "x" + to_string(tamanho))
                      << setw(15) << res.tempo_segundos 
                      << setw(20) << res.acessos_memoria 
                      << setw(15) << res.altura_media << "\n";
        }
        cout << "---------------------------------------------------------------------------------\n";
    }

    return 0;
}