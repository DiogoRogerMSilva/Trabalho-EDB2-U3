// Union-Find (conjuntos disjuntos) com 4 variações escolhidas em tempo de compilação
// e instrumentação opcional (contadores de acesso à memória e medição de altura).
//
//   DisjointSet<BySize, Compress, Instrument>
//
//   BySize=false, Compress=false  -> V1: sem otimização (ligação arbitrária)
//   BySize=true,  Compress=false  -> V2: união por tamanho
//   BySize=false, Compress=true   -> V3: compressão de caminho
//   BySize=true,  Compress=true   -> V4: união por tamanho + compressão de caminho
//
// Por que template e não um `if` em tempo de execução?
//   A escolha da variação é feita em tempo de compilação. 
//   O código das variações não usadas e dos contadores
//   (quando Instrument=false) vira função vazia e some na otimização. Assim a
//   medição de TEMPO não é contaminada por contadores nem por desvios extras.
#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace uf {

using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Contadores de instrumentação. "Acesso à memória" = qualquer leitura ou escrita
// nos vetores parent[] e size[] (as únicas estruturas que a união-busca usa).
struct Stats {
    u64 parent_reads = 0;
    u64 parent_writes = 0;
    u64 size_reads = 0;
    u64 size_writes = 0;
    u64 find_calls = 0;
    u64 union_calls = 0;
    u64 path_steps = 0;  // arestas percorridas subindo até a raiz (1ª passada do find)

    u64 memory_accesses() const {
        return parent_reads + parent_writes + size_reads + size_writes;
    }
};

// Altura das árvores da floresta.
//  - altura de uma árvore = maior profundidade entre seus nós (raiz tem profundidade 0)
struct HeightStats {
    u32 max_height = 0;          // maior altura entre todas as árvores
    double mean_tree_height = 0; // média das alturas, uma por árvore
    double mean_node_depth = 0;  // média da profundidade de todos os nós
    u32 num_trees = 0;
};

template <bool BySize, bool Compress, bool Instrument = false>
class DisjointSet {
    // Tags usadas para escolher a variação em tempo de compilação (substitui `if constexpr`).
    typedef std::integral_constant<bool, Instrument> InstrTag;
    typedef std::integral_constant<bool, BySize> SizeTag;
    typedef std::integral_constant<bool, Compress> CompressTag;

public:
    explicit DisjointSet(u32 n) : parent_(n), size_(BySize ? n : 0, 1u) {
        for (u32 i = 0; i < n; ++i) parent_[i] = i;  // cada nó começa como sua própria raiz
    }

    // Retorna o representante (raiz) do conjunto de x.
    // Versão ITERATIVA de propósito: o artigo usa recursão, mas na variação sem
    // otimização a altura pode ficar grande em grids enormes e estourar a pilha.
    u32 find(u32 x) {
        bump(stats_.find_calls);

        // 1ª passada: sobe até a raiz.
        u32 root = x;
        for (;;) {
            const u32 p = rd_parent(root);
            if (p == root) break;
            root = p;
            bump(stats_.path_steps);
        }

        // 2ª passada (só nas variações com compressão): aponta todo o caminho para a raiz.
        compress_path(x, root, CompressTag());
        return root;
    }

    // Une os conjuntos de a e b. Retorna true se eles eram distintos (união feita),
    // false se já estavam no mesmo conjunto (a aresta formaria ciclo).
    bool unite(u32 a, u32 b) {
        bump(stats_.union_calls);

        const u32 ra = find(a);
        const u32 rb = find(b);
        if (ra == rb) return false;

        link_roots(ra, rb, SizeTag());
        return true;
    }

    // Mede a altura das árvores no estado ATUAL. Acessa parent_ diretamente
    // (sem passar pelos contadores), então não altera as estatísticas de acesso.
    HeightStats measure_heights() const {
        const u32 n = static_cast<u32>(parent_.size());
        constexpr u32 UNSET = 0xFFFFFFFFu;
        std::vector<u32> depth(n, UNSET), root_of(n, 0), path;

        for (u32 i = 0; i < n; ++i) {
            if (depth[i] != UNSET) continue;
            path.clear();
            u32 v = i;
            while (depth[v] == UNSET && parent_[v] != v) {
                path.push_back(v);
                v = parent_[v];
            }
            if (depth[v] == UNSET) {  // chegamos a uma raiz ainda não visitada
                depth[v] = 0;
                root_of[v] = v;
            }
            u32 d = depth[v];
            const u32 r = root_of[v];
            for (auto it = path.rbegin(); it != path.rend(); ++it) {
                depth[*it] = ++d;
                root_of[*it] = r;
            }
        }

        std::vector<u32> height(n, 0);  // altura indexada pela raiz
        u64 depth_sum = 0;
        for (u32 i = 0; i < n; ++i) {
            depth_sum += depth[i];
            u32& h = height[root_of[i]];
            h = std::max(h, depth[i]);
        }

        HeightStats hs;
        u64 height_sum = 0;
        for (u32 i = 0; i < n; ++i) {
            if (parent_[i] == i) {
                ++hs.num_trees;
                height_sum += height[i];
                hs.max_height = std::max(hs.max_height, height[i]);
            }
        }
        hs.mean_tree_height = hs.num_trees ? static_cast<double>(height_sum) / hs.num_trees : 0;
        hs.mean_node_depth = n ? static_cast<double>(depth_sum) / n : 0;
        return hs;
    }

    const Stats& stats() const { return stats_; }
    void reset_stats() { stats_ = Stats{}; }
    u32 size() const { return static_cast<u32>(parent_.size()); }

private:
    // ---- Contagem: só faz algo quando Instrument=true ----
    void bump(u64& counter) { bump_impl(counter, InstrTag()); }
    static void bump_impl(u64& counter, std::true_type) { ++counter; }
    static void bump_impl(u64&, std::false_type) {}

    // ---- Compressão de caminho: só faz algo quando Compress=true ----
    void compress_path(u32 x, u32 root, std::true_type) {
        while (x != root) {
            const u32 next = rd_parent(x);
            if (next != root) wr_parent(x, root);  // evita escrita redundante
            x = next;
        }
    }
    void compress_path(u32, u32, std::false_type) {}

    // ---- Ligação das raízes ----
    // BySize=true: a raiz da árvore menor passa a apontar para a da maior.
    void link_roots(u32 ra, u32 rb, std::true_type) {
        const u32 sa = rd_size(ra);
        const u32 sb = rd_size(rb);
        if (sa < sb) std::swap(ra, rb);  // ra passa a ser a raiz da árvore maior
        wr_parent(rb, ra);
        wr_size(ra, sa + sb);
    }
    // BySize=false: ligação arbitrária, nenhuma tentativa de balancear.
    void link_roots(u32 ra, u32 rb, std::false_type) { wr_parent(rb, ra); }

    // Todo acesso a parent_/size_ passa por estas 4 funções, que são o ÚNICO ponto
    // onde os contadores são incrementados. Isso garante que nenhum acesso fique sem contar.
    u32 rd_parent(u32 i) {
        bump(stats_.parent_reads);
        return parent_[i];
    }
    void wr_parent(u32 i, u32 v) {
        bump(stats_.parent_writes);
        parent_[i] = v;
    }
    u32 rd_size(u32 i) {
        bump(stats_.size_reads);
        return size_[i];
    }
    void wr_size(u32 i, u32 v) {
        bump(stats_.size_writes);
        size_[i] = v;
    }

    std::vector<u32> parent_;
    std::vector<u32> size_;  // vazio quando BySize=false
    Stats stats_;
};

}  // namespace uf
