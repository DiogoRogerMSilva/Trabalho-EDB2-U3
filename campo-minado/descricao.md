# Trabalho Prático: Conjuntos Disjuntos (Union-Find) em Jogos

Este repositório contém a implementação em **C++** desenvolvida para o projeto final da disciplina de Estrutura de Dados Básicas II. O objetivo é comparar empiricamente o desempenho de diferentes variações da estrutura de dados Union-Find aplicadas a cenários em grades bidimensionais (*grids* 2D).

---

## Estrutura do Projeto

O código foi modularizado para facilitar a integração e o trabalho em equipe:

* **`ConjuntoDisjunto.h`**: Implementação central da estrutura de dados de Conjuntos Disjuntos contendo as **4 variações** exigidas no trabalho e instrumentação de métricas (acessos à memória e altura das árvores).
* **`CampoMinado.h`**: Implementação do **Cenário 2 (Campo Minado)**, responsável por gerar o tabuleiro, simular a união de células seguras adjacentes e coletar os dados de desempenho.
* **`main.cpp`**: Arquivo principal (*main*) que executa a automação dos testes de benchmark variando o tamanho dos grids.
* **`Makefile`**: Arquivo de automação para compilação rápida com otimização máxima (`-O3`).

---

## Variações do Union-Find Implementadas

1. **`naive`**: Versão ingênua (sem união por tamanho e sem compressão de caminho).
2. **`size`**: Apenas União por Tamanho (*Linking by Size*).
3. **`path_comp`**: Apenas Compressão de Caminho (*Path Compression* de 2 passagens).
4. **`both`**: União por Tamanho combinada com Compressão de Caminho.

---

## Como Compilar e Executar

Certifique-se de ter um compilador compatível com C++ (como `g++`) instalado. Na raiz do projeto, utilize os comandos do **Makefile**:

* Para compilar e rodar o benchmark do Campo Minado:
```bash
make run
```
* Apenas para compilar:
```bash
make
```
* Para limpar os binários gerados:
```bash
make clean
```