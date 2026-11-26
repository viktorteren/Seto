#include <cstdint>

extern "C" {

__device__ __forceinline__ uint32_t hash_u32(uint32_t x, uint32_t seed) {
    x ^= seed;
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

/**
 * Luby's parallel MIS - Phase 1:
 * Each active vertex (state[v] == 0) checks if its priority is strictly
 * greater than all active neighbors' priorities.
 */
__global__ void luby_select_candidates(
    int n,
    const int* __restrict__ row_ptr,
    const int* __restrict__ col_idx,
    const int* __restrict__ state,
    int* __restrict__ cand,
    uint32_t seed
) {
    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if (v >= n) return;
    if (state[v] != 0) {
        cand[v] = 0;
        return;
    }

    uint32_t pv = hash_u32(static_cast<uint32_t>(v), seed);
    int start = row_ptr[v];
    int end = row_ptr[v + 1];
    bool win = true;

    for (int e = start; e < end; ++e) {
        int u = col_idx[e];
        if (state[u] == 0) {
            uint32_t pu = hash_u32(static_cast<uint32_t>(u), seed);
            if (pu > pv || (pu == pv && u > v)) {
                win = false;
                break;
            }
        }
    }
    cand[v] = win ? 1 : 0;
}

/**
 * Luby's parallel MIS - Phase 2:
 * Winners enter MIS (state = 1) and eliminate their neighbors (state = 2).
 */
__global__ void luby_commit_and_eliminate(
    int n,
    const int* __restrict__ row_ptr,
    const int* __restrict__ col_idx,
    int* __restrict__ state,
    const int* __restrict__ cand,
    int* __restrict__ active_flag
) {
    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if (v >= n) return;

    if (cand[v] == 1) {
        state[v] = 1;
        int start = row_ptr[v];
        int end = row_ptr[v + 1];
        for (int e = start; e < end; ++e) {
            int u = col_idx[e];
            if (state[u] == 0) {
                state[u] = 2;
            }
        }
    }
}

/**
 * Check if any vertex still has state == 0.
 */
__global__ void luby_check_active(
    int n,
    const int* __restrict__ state,
    int* __restrict__ active_flag
) {
    int v = blockIdx.x * blockDim.x + threadIdx.x;
    if (v < n && state[v] == 0) {
        *active_flag = 1;
    }
}

/**
 * Parallel BFS level evaluation for all-MIS enumeration (n <= 64).
 * Each thread checks a candidate subset mask `cand_masks[idx]` against adjacency bitmasks `adj_masks[0..n-1]`:
 * - is_indep = true iff (cand_mask & adj_masks[v]) == 0 for all v in cand_mask
 * - is_maximal = is_indep && no vertex u outside cand_mask has (cand_mask & adj_masks[u]) == 0
 * Outputs `out_flags[idx]`: 1 = maximal IS, 2 = independent (can be expanded), 0 = not independent.
 */
__global__ void bfs_eval_is_batch(
    int num_cands,
    int n,
    const uint64_t* __restrict__ adj_masks,
    const uint64_t* __restrict__ cand_masks,
    uint8_t* __restrict__ out_flags
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= num_cands) return;

    uint64_t mask = cand_masks[idx];
    // Check independence
    uint64_t conflict = 0;
    uint64_t tmp = mask;
    while (tmp) {
        int v = __ffsll(static_cast<long long>(tmp)) - 1;
        conflict |= adj_masks[v];
        tmp &= (tmp - 1);
    }
    if ((mask & conflict) != 0ULL) {
        out_flags[idx] = 0; // Not independent
        return;
    }

    // Check maximality: if any u not in mask has no neighbor in mask, mask can still grow
    uint64_t all_n = (n == 64) ? ~0ULL : ((1ULL << n) - 1ULL);
    uint64_t covered_or_in = mask | conflict;
    if ((covered_or_in & all_n) == all_n) {
        out_flags[idx] = 1; // Maximal independent set
    } else {
        out_flags[idx] = 2; // Independent, extendable
    }
}

} // extern "C"
