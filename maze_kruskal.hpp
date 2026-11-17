// maze_kruskal.hpp
// Cenário 1: geração de labirinto com o algoritmo de Kruskal (como na Seção 2.5.1
// do artigo de referência), usando o union-find como detector de ciclos.
//
// Modelo: cada célula do grid W x H é um vértice. Cada par de células vizinhas
// (direita/baixo) é uma aresta com peso aleatório. A árvore geradora mínima
// resultante é o labirinto: arestas aceitas = passagens abertas entre células.
#pragma once

#include <algorithm>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include "disjoint_set.hpp"

namespace uf {

struct Edge {
    u32 a, b, w;
};

// Gera e ORDENA as arestas do grid. Isso é feito uma única vez por (tamanho, seed)
// e a MESMA lista é usada pelas 4 variações. Assim a única coisa que muda entre
// elas é o union-find, o que torna a comparação justa.
// A ordenação fica fora da medição de tempo: ela é igual para todas as variações
// e não faz parte da estrutura que estamos avaliando.
inline std::vector<Edge> make_grid_edges(u32 W, u32 H, u64 seed) {
    std::mt19937_64 rng(seed);
    std::vector<Edge> edges;
    edges.reserve(2ull * W * H);
    for (u32 j = 0; j < H; ++j) {
        for (u32 i = 0; i < W; ++i) {
            const u32 id = j * W + i;
            if (i + 1 < W) edges.push_back({id, id + 1, static_cast<u32>(rng())});
            if (j + 1 < H) edges.push_back({id, id + W, static_cast<u32>(rng())});
        }
    }
    // Pesos de 32 bits => empates são raros; desempate por (a, b) mantém determinismo.
    std::sort(edges.begin(), edges.end(), [](const Edge& x, const Edge& y) {
        if (x.w != y.w) return x.w < y.w;
        if (x.a != y.a) return x.a < y.a;
        return x.b < y.b;
    });
    return edges;
}

struct KruskalResult {
    u32 accepted = 0;           // arestas aceitas (deve ser W*H - 1)
    u64 checksum = 0;           // só preenchido com Trace=true
    u32 peak_max_height = 0;    // maior altura vista nos checkpoints (Trace=true)
    std::vector<u32> accepted_idx;  // índices das arestas aceitas (Trace=true)
};

namespace detail {
// Rastreio (checksum, arestas aceitas, altura nos checkpoints): só age com Trace=true.
inline void trace_accept(KruskalResult&, u64, std::false_type) {}
inline void trace_accept(KruskalResult& r, u64 i, std::true_type) {
    r.checksum = (r.checksum ^ i) * 1099511628211ULL;
    r.accepted_idx.push_back(static_cast<u32>(i));
}
template <class DS>
void trace_checkpoint(KruskalResult&, DS&, u64, u64, std::false_type) {}
template <class DS>
void trace_checkpoint(KruskalResult& r, DS& ds, u64 i, u64 step, std::true_type) {
    if ((i + 1) % step == 0)
        r.peak_max_height = std::max(r.peak_max_height, ds.measure_heights().max_height);
}
}  // namespace detail

// Laço do Kruskal. Percorre TODAS as arestas ordenadas, como no Código 6 do artigo
// (sem parada antecipada em V-1 arestas), o que mantém a carga de find() alta.
//
// Trace=false -> laço mínimo, usado para medir TEMPO.
// Trace=true  -> também calcula checksum, guarda as arestas aceitas e mede a altura
//                em `checkpoints` pontos da execução (usado com Instrument=true).
template <bool Trace, class DS>
KruskalResult run_kruskal(DS& ds, const std::vector<Edge>& edges, u32 checkpoints = 20) {
    typedef std::integral_constant<bool, Trace> TraceTag;
    KruskalResult r;
    const u64 m = edges.size();
    const u64 step = std::max<u64>(1, m / checkpoints);
    if (Trace) r.checksum = 1469598103934665603ULL;

    for (u64 i = 0; i < m; ++i) {
        const Edge& e = edges[i];
        if (ds.unite(e.a, e.b)) {
            ++r.accepted;
            detail::trace_accept(r, i, TraceTag());
        }
        detail::trace_checkpoint(r, ds, i, step, TraceTag());
    }
    return r;
}

// Desenha o labirinto em ASCII (útil para validar visualmente e para os slides).
// Células em posições ímpares; cada aresta aceita abre a parede entre duas células.
inline std::string render_maze(u32 W, u32 H, const std::vector<Edge>& edges,
                               const std::vector<u32>& accepted_idx) {
    const u32 R = 2 * H + 1, C = 2 * W + 1;
    std::vector<std::string> g(R, std::string(C, '#'));
    for (u32 j = 0; j < H; ++j)
        for (u32 i = 0; i < W; ++i) g[2 * j + 1][2 * i + 1] = ' ';
    for (u32 idx : accepted_idx) {
        const Edge& e = edges[idx];
        const u32 ai = e.a % W, aj = e.a / W, bi = e.b % W, bj = e.b / W;
        g[aj + bj + 1][ai + bi + 1] = ' ';  // ponto médio entre as duas células
    }
    std::string out;
    for (const auto& row : g) {
        for (char ch : row) { out += ch; out += ch; }  // dobra a largura p/ proporção
        out += '\n';
    }
    return out;
}

}  // namespace uf
