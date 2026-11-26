#include "../include/MISSolver.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace seto {

namespace {

MisBackend g_default_mis_backend = MisBackend::Auto;
std::string g_ptx_path = "mis_kernels.ptx";

// Minimal CUDA Driver API types & function pointers (loaded dynamically from nvcuda.dll)
#ifdef _WIN32
using CUresult = int;
using CUdevice = int;
using CUcontext = void*;
using CUmodule = void*;
using CUfunction = void*;
using CUdeviceptr = uint64_t;
constexpr CUresult CUDA_SUCCESS_VAL = 0;

struct CudaDriver {
    HMODULE lib = nullptr;
    bool initialized = false;
    CUdevice dev = 0;
    CUcontext ctx = nullptr;
    CUmodule mod = nullptr;
    CUfunction fn_select = nullptr;
    CUfunction fn_commit = nullptr;
    CUfunction fn_check = nullptr;
    CUfunction fn_bfs_eval = nullptr;

    using pfn_cuInit = CUresult (*)(unsigned int);
    using pfn_cuDeviceGetCount = CUresult (*)(int*);
    using pfn_cuDeviceGet = CUresult (*)(CUdevice*, int);
    using pfn_cuCtxCreate = CUresult (*)(CUcontext*, unsigned int, CUdevice);
    using pfn_cuModuleLoadData = CUresult (*)(CUmodule*, const void*);
    using pfn_cuModuleGetFunction = CUresult (*)(CUfunction*, CUmodule, const char*);
    using pfn_cuMemAlloc = CUresult (*)(CUdeviceptr*, size_t);
    using pfn_cuMemFree = CUresult (*)(CUdeviceptr);
    using pfn_cuMemcpyHtoD = CUresult (*)(CUdeviceptr, const void*, size_t);
    using pfn_cuMemcpyDtoH = CUresult (*)(void*, CUdeviceptr, size_t);
    using pfn_cuMemsetD32 = CUresult (*)(CUdeviceptr, unsigned int, size_t);
    using pfn_cuLaunchKernel = CUresult (*)(CUfunction,
                                            unsigned int, unsigned int, unsigned int,
                                            unsigned int, unsigned int, unsigned int,
                                            unsigned int, void*, void**, void**);
    using pfn_cuCtxSynchronize = CUresult (*)();

    pfn_cuInit cuInit = nullptr;
    pfn_cuDeviceGetCount cuDeviceGetCount = nullptr;
    pfn_cuDeviceGet cuDeviceGet = nullptr;
    pfn_cuCtxCreate cuCtxCreate = nullptr;
    pfn_cuModuleLoadData cuModuleLoadData = nullptr;
    pfn_cuModuleGetFunction cuModuleGetFunction = nullptr;
    pfn_cuMemAlloc cuMemAlloc = nullptr;
    pfn_cuMemFree cuMemFree = nullptr;
    pfn_cuMemcpyHtoD cuMemcpyHtoD = nullptr;
    pfn_cuMemcpyDtoH cuMemcpyDtoH = nullptr;
    pfn_cuMemsetD32 cuMemsetD32 = nullptr;
    pfn_cuLaunchKernel cuLaunchKernel = nullptr;
    pfn_cuCtxSynchronize cuCtxSynchronize = nullptr;

    static CudaDriver& instance() {
        static CudaDriver drv;
        return drv;
    }

    bool ensure_init() {
        if (initialized) return mod != nullptr;
        initialized = true;

        lib = LoadLibraryA("nvcuda.dll");
        if (!lib) return false;

        auto load = [&](const char* name) -> FARPROC {
            return GetProcAddress(lib, name);
        };

        cuInit = reinterpret_cast<pfn_cuInit>(load("cuInit"));
        cuDeviceGetCount = reinterpret_cast<pfn_cuDeviceGetCount>(load("cuDeviceGetCount"));
        cuDeviceGet = reinterpret_cast<pfn_cuDeviceGet>(load("cuDeviceGet"));
        cuCtxCreate = reinterpret_cast<pfn_cuCtxCreate>(load("cuCtxCreate_v2"));
        cuModuleLoadData = reinterpret_cast<pfn_cuModuleLoadData>(load("cuModuleLoadData"));
        cuModuleGetFunction = reinterpret_cast<pfn_cuModuleGetFunction>(load("cuModuleGetFunction"));
        cuMemAlloc = reinterpret_cast<pfn_cuMemAlloc>(load("cuMemAlloc_v2"));
        cuMemFree = reinterpret_cast<pfn_cuMemFree>(load("cuMemFree_v2"));
        cuMemcpyHtoD = reinterpret_cast<pfn_cuMemcpyHtoD>(load("cuMemcpyHtoD_v2"));
        cuMemcpyDtoH = reinterpret_cast<pfn_cuMemcpyDtoH>(load("cuMemcpyDtoH_v2"));
        cuMemsetD32 = reinterpret_cast<pfn_cuMemsetD32>(load("cuMemsetD32_v2"));
        cuLaunchKernel = reinterpret_cast<pfn_cuLaunchKernel>(load("cuLaunchKernel"));
        cuCtxSynchronize = reinterpret_cast<pfn_cuCtxSynchronize>(load("cuCtxSynchronize"));

        if (!cuInit || !cuDeviceGetCount || !cuDeviceGet || !cuCtxCreate ||
            !cuModuleLoadData || !cuModuleGetFunction || !cuMemAlloc || !cuMemFree ||
            !cuMemcpyHtoD || !cuMemcpyDtoH || !cuMemsetD32 || !cuLaunchKernel || !cuCtxSynchronize) {
            return false;
        }

        if (cuInit(0) != CUDA_SUCCESS_VAL) return false;
        int count = 0;
        if (cuDeviceGetCount(&count) != CUDA_SUCCESS_VAL || count <= 0) return false;
        if (cuDeviceGet(&dev, 0) != CUDA_SUCCESS_VAL) return false;
        if (cuCtxCreate(&ctx, 0, dev) != CUDA_SUCCESS_VAL) return false;

        // Locate PTX file
        std::vector<std::string> candidates = {g_ptx_path, "../" + g_ptx_path};
        char exe_buf[MAX_PATH];
        DWORD len = GetModuleFileNameA(nullptr, exe_buf, MAX_PATH);
        if (len > 0) {
            std::filesystem::path exe_dir = std::filesystem::path(exe_buf).parent_path();
            candidates.insert(candidates.begin(), (exe_dir / "mis_kernels.ptx").string());
        }

        std::string ptx_src;
        for (const auto& c : candidates) {
            std::ifstream ifs(c, std::ios::binary);
            if (ifs) {
                std::ostringstream ss;
                ss << ifs.rdbuf();
                ptx_src = ss.str();
                break;
            }
        }
        if (ptx_src.empty()) return false;

        if (cuModuleLoadData(&mod, ptx_src.c_str()) != CUDA_SUCCESS_VAL) {
            mod = nullptr;
            return false;
        }
        if (cuModuleGetFunction(&fn_select, mod, "luby_select_candidates") != CUDA_SUCCESS_VAL ||
            cuModuleGetFunction(&fn_commit, mod, "luby_commit_and_eliminate") != CUDA_SUCCESS_VAL ||
            cuModuleGetFunction(&fn_check, mod, "luby_check_active") != CUDA_SUCCESS_VAL ||
            cuModuleGetFunction(&fn_bfs_eval, mod, "bfs_eval_is_batch") != CUDA_SUCCESS_VAL) {
            mod = nullptr;
            return false;
        }
        return true;
    }
};
#endif

/**
 * Fast CPU greedy-degree maximal independent set on active vertices (allowed[v] == 1).
 * Vertices with lowest degree in the active subgraph are picked first to maximize set size.
 */
std::vector<int> cpu_maximal_is(const CsrGraph& g,
                                  const std::vector<uint8_t>& allowed,
                                  const std::vector<int>& initial_seed = {}) {
    const int n = g.num_vertices;
    std::vector<uint8_t> state(static_cast<size_t>(n), 2); // 0=active, 1=in_mis, 2=excluded
    std::vector<int> mis;

    for (int v = 0; v < n; ++v) {
        if (allowed[static_cast<size_t>(v)]) {
            state[static_cast<size_t>(v)] = 0;
        }
    }

    for (int v : initial_seed) {
        if (v >= 0 && v < n && state[static_cast<size_t>(v)] == 0) {
            state[static_cast<size_t>(v)] = 1;
            mis.push_back(v);
            for (int e = g.row_ptr[v]; e < g.row_ptr[v + 1]; ++e) {
                int u = g.col_idx[e];
                if (state[static_cast<size_t>(u)] == 0) {
                    state[static_cast<size_t>(u)] = 2;
                }
            }
        }
    }

    std::vector<int> order;
    order.reserve(static_cast<size_t>(n));
    std::vector<int> deg(static_cast<size_t>(n), 0);
    for (int v = 0; v < n; ++v) {
        if (state[static_cast<size_t>(v)] == 0) {
            int d = 0;
            for (int e = g.row_ptr[v]; e < g.row_ptr[v + 1]; ++e) {
                if (state[static_cast<size_t>(g.col_idx[e])] == 0) ++d;
            }
            deg[static_cast<size_t>(v)] = d;
            order.push_back(v);
        }
    }

    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return deg[static_cast<size_t>(a)] < deg[static_cast<size_t>(b)];
    });

    for (int v : order) {
        if (state[static_cast<size_t>(v)] != 0) continue;
        state[static_cast<size_t>(v)] = 1;
        mis.push_back(v);
        for (int e = g.row_ptr[v]; e < g.row_ptr[v + 1]; ++e) {
            int u = g.col_idx[e];
            if (state[static_cast<size_t>(u)] == 0) {
                state[static_cast<size_t>(u)] = 2;
            }
        }
    }

    return mis;
}

#ifdef _WIN32
/**
 * GPU multi-start Luby MIS on RTX 4090 / CUDA Driver API.
 * Runs `num_restarts` random seeds and picks the largest independent set found.
 */
std::vector<int> gpu_maximal_is(const CsrGraph& g,
                                  const std::vector<uint8_t>& allowed,
                                  int num_restarts = 4) {
    auto& drv = CudaDriver::instance();
    if (!drv.ensure_init()) {
        return cpu_maximal_is(g, allowed);
    }

    const int n = g.num_vertices;
    if (n == 0) return {};

    std::vector<int> init_state(static_cast<size_t>(n), 2);
    for (int v = 0; v < n; ++v) {
        if (allowed[static_cast<size_t>(v)]) {
            init_state[static_cast<size_t>(v)] = 0;
        }
    }

    CUdeviceptr d_row = 0, d_col = 0, d_state = 0, d_cand = 0, d_flag = 0;
    const size_t row_bytes = g.row_ptr.size() * sizeof(int);
    const size_t col_bytes = std::max<size_t>(1, g.col_idx.size()) * sizeof(int);
    const size_t n_bytes = static_cast<size_t>(n) * sizeof(int);

    if (drv.cuMemAlloc(&d_row, row_bytes) != CUDA_SUCCESS_VAL ||
        drv.cuMemAlloc(&d_col, col_bytes) != CUDA_SUCCESS_VAL ||
        drv.cuMemAlloc(&d_state, n_bytes) != CUDA_SUCCESS_VAL ||
        drv.cuMemAlloc(&d_cand, n_bytes) != CUDA_SUCCESS_VAL ||
        drv.cuMemAlloc(&d_flag, sizeof(int)) != CUDA_SUCCESS_VAL) {
        if (d_row) drv.cuMemFree(d_row);
        if (d_col) drv.cuMemFree(d_col);
        if (d_state) drv.cuMemFree(d_state);
        if (d_cand) drv.cuMemFree(d_cand);
        if (d_flag) drv.cuMemFree(d_flag);
        return cpu_maximal_is(g, allowed);
    }

    drv.cuMemcpyHtoD(d_row, g.row_ptr.data(), row_bytes);
    if (!g.col_idx.empty()) {
        drv.cuMemcpyHtoD(d_col, g.col_idx.data(), g.col_idx.size() * sizeof(int));
    }

    const unsigned int block = 256;
    const unsigned int grid = (static_cast<unsigned int>(n) + block - 1) / block;

    std::vector<int> best_mis;
    std::vector<int> host_state(static_cast<size_t>(n));

    for (int r = 0; r < num_restarts; ++r) {
        drv.cuMemcpyHtoD(d_state, init_state.data(), n_bytes);
        uint32_t seed = 0x9e3779b9u * static_cast<uint32_t>(r + 1);

        for (int round = 0; round < n + 1; ++round) {
            seed += 0x85ebca6bu;
            int n_arg = n;
            void* args_sel[] = {&n_arg, &d_row, &d_col, &d_state, &d_cand, &seed};
            drv.cuLaunchKernel(drv.fn_select, grid, 1, 1, block, 1, 1, 0, nullptr, args_sel, nullptr);

            void* args_com[] = {&n_arg, &d_row, &d_col, &d_state, &d_cand, &d_flag};
            drv.cuLaunchKernel(drv.fn_commit, grid, 1, 1, block, 1, 1, 0, nullptr, args_com, nullptr);

            drv.cuMemsetD32(d_flag, 0, 1);
            void* args_chk[] = {&n_arg, &d_state, &d_flag};
            drv.cuLaunchKernel(drv.fn_check, grid, 1, 1, block, 1, 1, 0, nullptr, args_chk, nullptr);

            int active = 0;
            drv.cuMemcpyDtoH(&active, d_flag, sizeof(int));
            if (active == 0) break;
        }

        drv.cuMemcpyDtoH(host_state.data(), d_state, n_bytes);
        std::vector<int> cur_mis;
        for (int v = 0; v < n; ++v) {
            if (host_state[static_cast<size_t>(v)] == 1) {
                cur_mis.push_back(v);
            }
        }
        if (cur_mis.size() > best_mis.size()) {
            best_mis = std::move(cur_mis);
        }
    }

    drv.cuMemFree(d_row);
    drv.cuMemFree(d_col);
    drv.cuMemFree(d_state);
    drv.cuMemFree(d_cand);
    drv.cuMemFree(d_flag);

    return best_mis;
}
#endif

} // namespace

void set_default_mis_backend(MisBackend backend) noexcept {
    g_default_mis_backend = backend;
}

MisBackend default_mis_backend() noexcept {
    return g_default_mis_backend;
}

void set_mis_ptx_path(const std::string& path) {
    g_ptx_path = path;
}

bool is_cuda_mis_available() {
#ifdef _WIN32
    return CudaDriver::instance().ensure_init();
#else
    return false;
#endif
}

CsrGraph build_csr_from_clauses(int num_vertices,
                                const std::vector<std::vector<int32_t>*>& clauses) {
    CsrGraph g;
    g.num_vertices = num_vertices;
    if (num_vertices <= 0) {
        g.row_ptr.assign(1, 0);
        return g;
    }

    std::vector<std::vector<int>> adj(static_cast<size_t>(num_vertices));
    for (const auto* cl : clauses) {
        if (!cl || cl->size() < 2) continue;
        int u = -(*cl)[0] - 1;
        int v = -(*cl)[1] - 1;
        if (u >= 0 && u < num_vertices && v >= 0 && v < num_vertices && u != v) {
            adj[static_cast<size_t>(u)].push_back(v);
            adj[static_cast<size_t>(v)].push_back(u);
        }
    }

    g.row_ptr.assign(static_cast<size_t>(num_vertices + 1), 0);
    for (int i = 0; i < num_vertices; ++i) {
        auto& nbrs = adj[static_cast<size_t>(i)];
        std::sort(nbrs.begin(), nbrs.end());
        nbrs.erase(std::unique(nbrs.begin(), nbrs.end()), nbrs.end());
        g.row_ptr[static_cast<size_t>(i + 1)] =
            g.row_ptr[static_cast<size_t>(i)] + static_cast<int>(nbrs.size());
    }

    g.col_idx.reserve(static_cast<size_t>(g.row_ptr.back()));
    for (int i = 0; i < num_vertices; ++i) {
        const auto& nbrs = adj[static_cast<size_t>(i)];
        g.col_idx.insert(g.col_idx.end(), nbrs.begin(), nbrs.end());
    }

    return g;
}

std::vector<std::vector<int>> solve_mis_cover(const CsrGraph& graph, MisBackend backend) {
    const int n = graph.num_vertices;
    if (n <= 0) return {};

    bool use_gpu = false;
#ifdef _WIN32
    if (backend == MisBackend::Gpu) {
        use_gpu = is_cuda_mis_available();
    } else if (backend == MisBackend::Auto &&
               n >= 20000 &&
               graph.col_idx.size() >= 1000000) {
        // GPU only pays off over CPU CSR on very large/dense graphs
        use_gpu = is_cuda_mis_available();
    }
#endif

    std::vector<uint8_t> full_allowed(static_cast<size_t>(n), 1);
    std::vector<uint8_t> remaining(static_cast<size_t>(n), 1);

    auto find_mis = [&](const std::vector<uint8_t>& allowed) -> std::vector<int> {
#ifdef _WIN32
        if (use_gpu) {
            return gpu_maximal_is(graph, allowed);
        }
#endif
        return cpu_maximal_is(graph, allowed);
    };

    std::vector<std::vector<int>> final_fsms;
    std::vector<int> first_fsm = find_mis(full_allowed);
    for (int v : first_fsm) {
        remaining[static_cast<size_t>(v)] = 0;
    }
    final_fsms.push_back(std::move(first_fsm));

    std::vector<std::vector<int>> incomplete_fsms;
    while (std::any_of(remaining.begin(), remaining.end(), [](uint8_t x) { return x != 0; })) {
        std::vector<int> fsm = find_mis(remaining);
        if (fsm.empty()) break;
        for (int v : fsm) {
            remaining[static_cast<size_t>(v)] = 0;
        }
        incomplete_fsms.push_back(std::move(fsm));
    }

    // Extend each incomplete FSM to a maximal independent set of the full graph G
    for (const auto& seed : incomplete_fsms) {
        final_fsms.push_back(cpu_maximal_is(graph, full_allowed, seed));
    }

    // Convert 0-based internal indices back to 1-based Seto region IDs
    for (auto& fsm : final_fsms) {
        for (int& v : fsm) {
            v += 1;
        }
    }

    return final_fsms;
}

namespace {

/**
 * Bron-Kerbosch with pivoting (Tomita et al.) on C++20 bitsets.
 * Enumerates all Maximal Independent Sets (i.e. maximal cliques in complement graph).
 */
void bron_kerbosch_pivot(
    int words,
    const std::vector<std::vector<uint64_t>>& non_adj,
    std::vector<int>& R,
    std::vector<uint64_t> P,
    std::vector<uint64_t> X,
    std::vector<std::vector<int>>& out_mis
) {
    bool p_empty = true;
    bool x_empty = true;
    for (int w = 0; w < words; ++w) {
        if (P[static_cast<size_t>(w)] != 0) { p_empty = false; break; }
    }
    if (p_empty) {
        for (int w = 0; w < words; ++w) {
            if (X[static_cast<size_t>(w)] != 0) { x_empty = false; break; }
        }
        if (x_empty) {
            out_mis.push_back(R);
        }
        return;
    }

    // Choose pivot u in P U X maximizing |P & non_adj[u]|
    int best_u = -1;
    int best_deg = -1;
    for (int w = 0; w < words; ++w) {
        uint64_t ux = P[static_cast<size_t>(w)] | X[static_cast<size_t>(w)];
        while (ux) {
            int bit = std::countr_zero(ux);
            int u = w * 64 + bit;
            int cnt = 0;
            const auto& na = non_adj[static_cast<size_t>(u)];
            for (int k = 0; k < words; ++k) {
                cnt += std::popcount(P[static_cast<size_t>(k)] & na[static_cast<size_t>(k)]);
            }
            if (cnt > best_deg) {
                best_deg = cnt;
                best_u = u;
            }
            ux &= (ux - 1);
        }
    }

    // Candidates: P \ non_adj[best_u]
    std::vector<uint64_t> cands(static_cast<size_t>(words), 0);
    for (int w = 0; w < words; ++w) {
        cands[static_cast<size_t>(w)] = P[static_cast<size_t>(w)] & ~non_adj[static_cast<size_t>(best_u)][static_cast<size_t>(w)];
    }

    for (int w = 0; w < words; ++w) {
        uint64_t cw = cands[static_cast<size_t>(w)];
        while (cw) {
            int bit = std::countr_zero(cw);
            int v = w * 64 + bit;
            const auto& na_v = non_adj[static_cast<size_t>(v)];

            R.push_back(v);
            std::vector<uint64_t> next_P(static_cast<size_t>(words));
            std::vector<uint64_t> next_X(static_cast<size_t>(words));
            for (int k = 0; k < words; ++k) {
                next_P[static_cast<size_t>(k)] = P[static_cast<size_t>(k)] & na_v[static_cast<size_t>(k)];
                next_X[static_cast<size_t>(k)] = X[static_cast<size_t>(k)] & na_v[static_cast<size_t>(k)];
            }
            bron_kerbosch_pivot(words, non_adj, R, std::move(next_P), std::move(next_X), out_mis);
            R.pop_back();

            P[static_cast<size_t>(w)] &= ~(1ULL << bit);
            X[static_cast<size_t>(w)] |= (1ULL << bit);
            cw &= (cw - 1);
        }
    }
}

std::vector<std::vector<int>> enumerate_all_mis_cpu(const CsrGraph& g) {
    const int n = g.num_vertices;
    if (n <= 0) return {};
    const int words = (n + 63) / 64;

    // Build complement adjacency bitsets (non_adj[u] has bit v=1 iff (u,v) not in E and u != v)
    std::vector<std::vector<uint64_t>> non_adj(static_cast<size_t>(n),
                                               std::vector<uint64_t>(static_cast<size_t>(words), ~0ULL));
    for (int u = 0; u < n; ++u) {
        // Clear padding bits in last word
        int rem = n % 64;
        if (rem != 0) {
            non_adj[static_cast<size_t>(u)][static_cast<size_t>(words - 1)] = (1ULL << rem) - 1ULL;
        }
        // Clear self and neighbors
        non_adj[static_cast<size_t>(u)][static_cast<size_t>(u / 64)] &= ~(1ULL << (u % 64));
        for (int e = g.row_ptr[u]; e < g.row_ptr[u + 1]; ++e) {
            int v = g.col_idx[e];
            non_adj[static_cast<size_t>(u)][static_cast<size_t>(v / 64)] &= ~(1ULL << (v % 64));
        }
    }

    std::vector<uint64_t> P(static_cast<size_t>(words), ~0ULL);
    int rem = n % 64;
    if (rem != 0) {
        P[static_cast<size_t>(words - 1)] = (1ULL << rem) - 1ULL;
    }
    std::vector<uint64_t> X(static_cast<size_t>(words), 0ULL);
    std::vector<int> R;
    std::vector<std::vector<int>> results;

    bron_kerbosch_pivot(words, non_adj, R, std::move(P), std::move(X), results);
    return results;
}

#ifdef _WIN32
/**
 * Parallel CUDA BFS over bitmask frontiers for all-MIS enumeration (n <= 64).
 */
std::vector<std::vector<int>> enumerate_all_mis_gpu(const CsrGraph& g) {
    const int n = g.num_vertices;
    if (n <= 0 || n > 64) {
        return enumerate_all_mis_cpu(g);
    }
    auto& drv = CudaDriver::instance();
    if (!drv.ensure_init() || !drv.fn_bfs_eval) {
        return enumerate_all_mis_cpu(g);
    }

    std::vector<uint64_t> adj_masks(static_cast<size_t>(n), 0ULL);
    for (int u = 0; u < n; ++u) {
        for (int e = g.row_ptr[u]; e < g.row_ptr[u + 1]; ++e) {
            int v = g.col_idx[e];
            adj_masks[static_cast<size_t>(u)] |= (1ULL << v);
        }
    }

    CUdeviceptr d_adj = 0;
    if (drv.cuMemAlloc(&d_adj, static_cast<size_t>(n) * sizeof(uint64_t)) != CUDA_SUCCESS_VAL) {
        return enumerate_all_mis_cpu(g);
    }
    drv.cuMemcpyHtoD(d_adj, adj_masks.data(), static_cast<size_t>(n) * sizeof(uint64_t));

    std::vector<uint64_t> frontier;
    frontier.reserve(static_cast<size_t>(n));
    for (int v = 0; v < n; ++v) {
        frontier.push_back(1ULL << v);
    }

    std::vector<uint64_t> maximal_masks;
    std::unordered_set<uint64_t> visited;
    for (uint64_t m : frontier) visited.insert(m);

    while (!frontier.empty()) {
        int num_cands = static_cast<int>(frontier.size());
        CUdeviceptr d_cands = 0, d_flags = 0;
        if (drv.cuMemAlloc(&d_cands, static_cast<size_t>(num_cands) * sizeof(uint64_t)) != CUDA_SUCCESS_VAL ||
            drv.cuMemAlloc(&d_flags, static_cast<size_t>(num_cands) * sizeof(uint8_t)) != CUDA_SUCCESS_VAL) {
            if (d_cands) drv.cuMemFree(d_cands);
            if (d_flags) drv.cuMemFree(d_flags);
            drv.cuMemFree(d_adj);
            return enumerate_all_mis_cpu(g);
        }

        drv.cuMemcpyHtoD(d_cands, frontier.data(), static_cast<size_t>(num_cands) * sizeof(uint64_t));
        const unsigned int block = 256;
        const unsigned int grid = (static_cast<unsigned int>(num_cands) + block - 1) / block;
        int n_arg = n;
        void* args[] = {&num_cands, &n_arg, &d_adj, &d_cands, &d_flags};
        drv.cuLaunchKernel(drv.fn_bfs_eval, grid, 1, 1, block, 1, 1, 0, nullptr, args, nullptr);

        std::vector<uint8_t> flags(static_cast<size_t>(num_cands));
        drv.cuMemcpyDtoH(flags.data(), d_flags, static_cast<size_t>(num_cands) * sizeof(uint8_t));
        drv.cuMemFree(d_cands);
        drv.cuMemFree(d_flags);

        std::vector<uint64_t> next_frontier;
        for (int i = 0; i < num_cands; ++i) {
            uint64_t m = frontier[static_cast<size_t>(i)];
            if (flags[static_cast<size_t>(i)] == 1) {
                maximal_masks.push_back(m);
            } else if (flags[static_cast<size_t>(i)] == 2) {
                // Expand only with vertices > highest set bit to avoid duplicate paths in BFS
                int last_v = 63 - std::countl_zero(m);
                for (int u = last_v + 1; u < n; ++u) {
                    if ((m & adj_masks[static_cast<size_t>(u)]) == 0ULL) {
                        uint64_t nm = m | (1ULL << u);
                        if (visited.insert(nm).second) {
                            next_frontier.push_back(nm);
                        }
                    }
                }
            }
        }
        frontier = std::move(next_frontier);
    }

    drv.cuMemFree(d_adj);

    std::vector<std::vector<int>> results;
    results.reserve(maximal_masks.size());
    for (uint64_t m : maximal_masks) {
        std::vector<int> mis;
        while (m) {
            int v = std::countr_zero(m);
            mis.push_back(v);
            m &= (m - 1);
        }
        results.push_back(std::move(mis));
    }
    return results;
}
#endif

} // namespace

std::vector<std::vector<int>> enumerate_all_mis(const CsrGraph& graph, MisBackend backend) {
    std::vector<std::vector<int>> res;
#ifdef _WIN32
    if (backend == MisBackend::Gpu && graph.num_vertices <= 64 && is_cuda_mis_available()) {
        res = enumerate_all_mis_gpu(graph);
    } else {
        res = enumerate_all_mis_cpu(graph);
    }
#else
    (void)backend;
    res = enumerate_all_mis_cpu(graph);
#endif
    for (auto& mis : res) {
        for (int& v : mis) {
            v += 1;
        }
    }
    return res;
}

} // namespace seto
