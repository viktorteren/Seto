#ifndef SETO_SATSOLVER_H
#define SETO_SATSOLVER_H

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

namespace seto {

enum class SatBackend {
    CaDiCaL,
    MiniSat
};

/**
 * In-memory incremental SAT solver wrapper supporting CaDiCaL and MiniSat.
 * Eliminates temporary DIMACS file I/O and enables safe parallel execution.
 */
class InMemorySatSolver {
public:
    explicit InMemorySatSolver(SatBackend backend = default_backend());
    ~InMemorySatSolver();

    InMemorySatSolver(const InMemorySatSolver&) = delete;
    InMemorySatSolver& operator=(const InMemorySatSolver&) = delete;

    static void set_default_backend(SatBackend backend) noexcept { default_backend_ = backend; }
    static SatBackend default_backend() noexcept { return default_backend_; }

    /**
     * Feed any newly appended clauses from `clauses` (and optional `avoid_clauses`)
     * and solve. Updates `last_model_` only when satisfiable.
     */
    bool solve(int num_vars,
               const std::vector<std::vector<int32_t>>& clauses,
               const std::vector<std::set<int>>* avoid_clauses = nullptr);

    /** Returns 1 (true), -1 (false), or 0 (undef) for 1-based variable `var`. */
    [[nodiscard]] int8_t model_value(int var_1based) const noexcept {
        if (var_1based <= 0 || static_cast<size_t>(var_1based) > last_model_.size()) {
            return 0;
        }
        return last_model_[static_cast<size_t>(var_1based - 1)];
    }

    [[nodiscard]] int n_vars() const noexcept { return static_cast<int>(last_model_.size()); }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::vector<int8_t> last_model_;
    size_t fed_clauses_ = 0;
    size_t fed_avoid_ = 0;

    static inline SatBackend default_backend_ = SatBackend::CaDiCaL;
};

} // namespace seto

#endif // SETO_SATSOLVER_H
