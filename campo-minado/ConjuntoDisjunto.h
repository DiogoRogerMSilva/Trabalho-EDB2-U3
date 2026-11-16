#ifndef CONJUNTO_DISJUNTO_H
#define CONJUNTO_DISJUNTO_H

#include <vector>
#include <string>
#include <utility>

using namespace std;

class ConjuntoDisjuntoGradeInstrumentado {
public:
    int tamanho_n;
    string modo;
    vector<int> pai;
    vector<int> tamanho_arvore;
    long long acessos_memoria;

    ConjuntoDisjuntoGradeInstrumentado(int n, const string& m) {
        tamanho_n = n;
        modo = m;
        pai.resize(n);
        tamanho_arvore.assign(n, 1);
        for (int i = 0; i < n; ++i) pai[i] = i;
        acessos_memoria = 0;
    }

    int buscar(int i) {
        if (modo == "naive" || modo == "size") {
            int atual = i;
            while (atual != pai[atual]) {
                acessos_memoria += 2; // Leitura de pai[atual] + verificação
                atual = pai[atual];
            }
            acessos_memoria += 1; // Última leitura ao sair do loop
            return atual;
        } 
        else if (modo == "path_comp" || modo == "both") {
            // 1ª Passagem: Encontra a raiz
            vector<int> caminho;
            int raiz = i;
            while (raiz != pai[raiz]) {
                acessos_memoria += 2;
                caminho.push_back(raiz);
                raiz = pai[raiz];
            }
            acessos_memoria += 1;

            // 2ª Passagem (Compressão de Caminho): Aponta todos direto para a raiz
            for (int no : caminho) {
                acessos_memoria += 2; // Leitura e Escrita
                pai[no] = raiz;
            }
            return raiz;
        }
        return i;
    }

    void unir(int i, int j) {
        int raiz_i = buscar(i);
        int raiz_j = buscar(j);

        if (raiz_i != raiz_j) {
            if (modo == "naive" || modo == "path_comp") {
                acessos_memoria += 2; // Leitura e Escrita em pai[raiz_i]
                pai[raiz_i] = raiz_j;
            } 
            else if (modo == "size" || modo == "both") {
                acessos_memoria += 4; // Leituras de tamanho_arvore
                if (tamanho_arvore[raiz_i] < tamanho_arvore[raiz_j]) {
                    acessos_memoria += 3; // Escritas (pai e tamanho_arvore)
                    pai[raiz_i] = raiz_j;
                    tamanho_arvore[raiz_j] += tamanho_arvore[raiz_i];
                } else {
                    acessos_memoria += 3;
                    pai[raiz_j] = raiz_i;
                    tamanho_arvore[raiz_i] += tamanho_arvore[raiz_j];
                }
            }
        }
    }

    pair<double, int> calcular_estatisticas_arvore() {
        int altura_maxima = 0;
        long long altura_total = 0;
        int n = pai.size();

        for (int i = 0; i < n; ++i) {
            int h = 0;
            int atual = i;
            while (atual != pai[atual]) {
                h++;
                atual = pai[atual];
            }
            if (h > altura_maxima) altura_maxima = h;
            altura_total += h;
        }
        double altura_media = (n > 0) ? (double)altura_total / n : 0.0;
        return {altura_media, altura_maxima};
    }
};

#endif