// Roda as 4 variações do union-find no cenário do labirinto (Kruskal) e grava um CSV.
//
// Uso:
//   ./benchmark_maze                                  (padrão: 32,64,...,1024; 3 seeds; 5 reps; V1 até lado 512, com 3 reps)
//   ./benchmark_maze --sizes 100,500,2000 --seeds 5 --reps 7 --naive-max 256 --naive-reps 2 --csv resultados.csv (se quiser mudar os parâmetros ou não usar o CSV padrão)
//   ./benchmark_maze --print 12                       (desenha um labirinto 12x12 e sai. Apenas para visualização, não faz benchmark.)
//
// O CSV tem a coluna `scenario`. O passo 2 (Campo Minado) pode acrescentar linhas
// com scenario=minesweeper no mesmo formato e usar o mesmo script de gráficos.

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "disjoint_set.hpp"
#include "maze_kruskal.hpp"

using namespace uf;
using Clock = std::chrono::steady_clock;

struct Config {
    std::vector<u32> sizes{32, 64, 128, 256, 512, 1024};  // grids quadrados N x N
    u32 naive_max = 512;  // a V1 só roda em grids com lado <= naive_max (ver nota em main)
    u32 naive_reps = 3;   // a V1 é lenta nos grids grandes: usa no máximo este nº de repetições
    u32 seeds = 3;
    u32 reps = 5;
    u64 base_seed = 42;
    std::string csv = "results_maze.csv";
    u32 print_size = 0;
};

static std::vector<u32> parse_list(const std::string& s) {
    std::vector<u32> v;
    std::stringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, ',')) v.push_back(static_cast<u32>(std::strtoul(tok.c_str(), nullptr, 10)));
    return v;
}

static Config parse_args(int argc, char** argv) {
    Config c;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto next = [&]() -> std::string {
            if (i + 1 >= argc) { std::cerr << "faltou valor para " << a << "\n"; std::exit(1); }
            return argv[++i];
        };
        if (a == "--sizes") c.sizes = parse_list(next());
        else if (a == "--seeds") c.seeds = static_cast<u32>(std::strtoul(next().c_str(), nullptr, 10));
        else if (a == "--reps") c.reps = static_cast<u32>(std::strtoul(next().c_str(), nullptr, 10));
        else if (a == "--seed") c.base_seed = std::strtoull(next().c_str(), nullptr, 10);
        else if (a == "--naive-reps") c.naive_reps = static_cast<u32>(std::strtoul(next().c_str(), nullptr, 10));
        else if (a == "--naive-max") c.naive_max = static_cast<u32>(std::strtoul(next().c_str(), nullptr, 10));
        else if (a == "--csv") c.csv = next();
        else if (a == "--print") c.print_size = static_cast<u32>(std::strtoul(next().c_str(), nullptr, 10));
        else { std::cerr << "argumento desconhecido: " << a << "\n"; std::exit(1); }
    }
    return c;
}

// Executa UMA variação em UMA instância (tamanho, seed) e escreve uma linha no CSV.

template <bool BySize, bool Compress>
static void bench_variant(const char* name, u32 reps, u32 size, u64 seed,
                          const std::vector<Edge>& edges, bool& have_ref, u64& ref_checksum,
                          std::ofstream& csv) {
    const u32 W = size, H = size, n = W * H;

    // --- Execução 1: INSTRUMENTADA (contadores + alturas). Não é usada para tempo. ---

    DisjointSet<BySize, Compress, true> di(n);
    KruskalResult tr = run_kruskal<true>(di, edges);
    const HeightStats hs = di.measure_heights();
    const Stats& st = di.stats();

    // Verificações de corretude.
    if (tr.accepted != n - 1) {
        std::cerr << "ERRO: " << name << " aceitou " << tr.accepted << " arestas, esperado " << n - 1 << "\n";
        std::exit(2);
    }
    // Kruskal aceita exatamente as mesmas arestas, não importa o union-find usado.
    // Se o checksum diferir entre variações, há bug em alguma delas.

    if (!have_ref) { ref_checksum = tr.checksum; have_ref = true; }
    else if (ref_checksum != tr.checksum) {
        std::cerr << "ERRO: " << name << " produziu um labirinto diferente das outras variações\n";
        std::exit(3);
    }

    // --- Execução 2: TEMPO (sem contadores). 1 rodada de aquecimento + `reps` medidas. ---

    std::vector<double> ms;
    for (u32 r = 0; r < reps + 1; ++r) {
        DisjointSet<BySize, Compress, false> d(n);  // construção fora do cronômetro
        const auto t0 = Clock::now();
        KruskalResult res = run_kruskal<false>(d, edges);
        const auto t1 = Clock::now();
        if (res.accepted != n - 1) { std::cerr << "ERRO na rodada de tempo\n"; std::exit(2); }
        if (r > 0) ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    std::sort(ms.begin(), ms.end());
    const double med = ms[ms.size() / 2], mn = ms.front();

    const u32 peak = std::max(tr.peak_max_height, hs.max_height);

    csv << "maze," << name << ',' << W << ',' << H << ',' << n << ',' << edges.size() << ','
        << seed << ',' << reps << ',' << med << ',' << mn << ','
        << st.find_calls << ',' << st.union_calls << ',' << st.path_steps << ','
        << st.parent_reads << ',' << st.parent_writes << ',' << st.size_reads << ',' << st.size_writes << ','
        << st.memory_accesses() << ',' << hs.max_height << ',' << peak << ','
        << hs.mean_tree_height << ',' << hs.mean_node_depth << ',' << tr.accepted << '\n';

    std::cout << "  " << std::left << std::setw(27) << name << std::right << " tempo=" << std::fixed
              << std::setprecision(3) << std::setw(10) << med << " ms | acessos=" << std::setw(12)
              << st.memory_accesses() << " | altura max=" << std::setw(6) << hs.max_height
              << " | prof. media=" << hs.mean_node_depth << "\n";
}

int main(int argc, char** argv) {
    const Config cfg = parse_args(argc, argv);

    if (cfg.print_size > 0) {  // modo visual: desenha um labirinto pequeno
        const u32 s = cfg.print_size;
        auto edges = make_grid_edges(s, s, cfg.base_seed);
        DisjointSet<true, true, false> d(s * s);
        KruskalResult r = run_kruskal<true>(d, edges);
        std::cout << render_maze(s, s, edges, r.accepted_idx);
        return 0;
    }

    std::ofstream csv(cfg.csv);
    csv << std::setprecision(10);
    csv << "scenario,variant,width,height,cells,edges,seed,reps,time_ms_median,time_ms_min,"
           "find_calls,union_calls,path_steps,parent_reads,parent_writes,size_reads,size_writes,"
           "mem_accesses,final_max_height,peak_max_height,mean_tree_height,mean_node_depth,accepted_edges\n";

    for (u32 size : cfg.sizes) {
        for (u32 s = 0; s < cfg.seeds; ++s) {
            const u64 seed = cfg.base_seed + s;
            std::cout << "[grid " << size << "x" << size << " | seed " << seed << "]\n";
            const auto edges = make_grid_edges(size, size, seed);  // gerada 1x, usada pelas 4 variações
            bool have_ref = false;
            u64 ref = 0;

            // Sem balanceamento a altura cresce de forma explosiva (no teste, ~10 mil níveis
            // num grid 500x500, ~19 s por rodada, e o custo cresce ~ com o quadrado do nº de células). Por isso a V1 é pulada acima de --naive-max;
            // [IMPORTANTE]: isso é um RESULTADO, não um bug. A V1 SEM otimização é propositalmente lenta e com árvores altas.

            if (size <= cfg.naive_max)
                bench_variant<false, false>("V1_sem_otimizacao", std::min(cfg.reps, cfg.naive_reps), size, seed, edges, have_ref, ref, csv);
            else
                std::cout << "  V1_sem_otimizacao      (pulada: grid acima de --naive-max=" << cfg.naive_max << ")\n";
            bench_variant<true,  false>("V2_uniao_por_tamanho", cfg.reps, size, seed, edges, have_ref, ref, csv);
            bench_variant<false, true >("V3_compressao_caminho", cfg.reps, size, seed, edges, have_ref, ref, csv);
            bench_variant<true,  true >("V4_tamanho_mais_compressao", cfg.reps, size, seed, edges, have_ref, ref, csv);
        }
    }
    std::cout << "\nResultados gravados em " << cfg.csv << "\n";
    return 0;
}
