#ifndef SETO_MISSOLVER_H
#define SETO_MISSOLVER_H

#include <cstdint>
#include <string>
#include <vector>

namespace seto {

enum class MisBackend {
    Auto,
    Cpu,
    Gpu
};

struct CsrGraph {
    int num_vertices = 0;
    std::vector<int> row_ptr;
    std::vector<int> col_idx;
};

/**
 * Build a symmetric 0-based CSR graph from 1-based Seto overlap clauses
 * (each clause has 2 negative 1-based region IDs: [-u, -v]).
 */
CsrGraph build_csr_from_clauses(int num_vertices,
                                const std::vector<std::vector<int32_t>*>& clauses);

void set_default_mis_backend(MisBackend backend) noexcept;
MisBackend default_mis_backend() noexcept;
void set_mis_ptx_path(const std::string& path);
bool is_cuda_mis_available();

/**
 * Solve the Maximal/Near-Maximum Independent Set cover in-memory.
 * Returns a list of SMs, where each SM is a vector of 1-based region IDs.
 */
std::vector<std::vector<int>> solve_mis_cover(
    const CsrGraph& graph,
    MisBackend backend = default_mis_backend());

/**
 * Enumerate all Maximal Independent Sets in-memory.
 * CPU uses Bron-Kerbosch with pivoting on C++20 bitsets;
 * GPU uses parallel CUDA BFS over bitmask frontiers (for n <= 64).
 */
std::vector<std::vector<int>> enumerate_all_mis(
    const CsrGraph& graph,
    MisBackend backend = default_mis_backend());

} // namespace seto

#endif // SETO_MISSOLVER_H
