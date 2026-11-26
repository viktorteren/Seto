#include "../include/SatSolver.h"

#include "../libs/minisat/core/Solver.h"
#ifdef SETO_WITH_CADICAL
#include "../libs/cadical/src/cadical.hpp"
#endif

#include <algorithm>
#include <stdexcept>

namespace seto {

struct InMemorySatSolver::Impl {
    SatBackend backend;
    std::unique_ptr<Minisat::Solver> minisat;
#ifdef SETO_WITH_CADICAL
    std::unique_ptr<CaDiCaL::Solver> cadical;
#endif
    int minisat_vars = 0;

    explicit Impl(SatBackend b) : backend(b) {
#ifdef SETO_WITH_CADICAL
        if (backend == SatBackend::CaDiCaL) {
            cadical = std::make_unique<CaDiCaL::Solver>();
            cadical->set("quiet", 1);
            return;
        }
#endif
        backend = SatBackend::MiniSat;
        minisat = std::make_unique<Minisat::Solver>();
        minisat->verbosity = 0;
    }

    void ensure_minisat_var(int v_1based) {
        while (v_1based > minisat_vars) {
            minisat->newVar();
            ++minisat_vars;
        }
    }

    void add_clause(const std::vector<int32_t>& clause) {
        if (backend == SatBackend::MiniSat) {
            Minisat::vec<Minisat::Lit> lits;
            lits.capacity(static_cast<int>(clause.size()));
            for (int32_t lit : clause) {
                if (lit == 0) continue;
                int v = std::abs(lit);
                ensure_minisat_var(v);
                lits.push(lit > 0 ? Minisat::mkLit(v - 1, false) : Minisat::mkLit(v - 1, true));
            }
            minisat->addClause_(lits);
        }
#ifdef SETO_WITH_CADICAL
        else {
            for (int32_t lit : clause) {
                if (lit != 0) cadical->add(lit);
            }
            cadical->add(0);
        }
#endif
    }

    void add_avoid_clause(const std::set<int>& clause) {
        if (backend == SatBackend::MiniSat) {
            Minisat::vec<Minisat::Lit> lits;
            lits.capacity(static_cast<int>(clause.size()));
            for (int lit : clause) {
                if (lit == 0) continue;
                int v = std::abs(lit);
                ensure_minisat_var(v);
                // Negate literal to block previous solution, matching convert_to_dimacs
                lits.push(lit > 0 ? Minisat::mkLit(v - 1, true) : Minisat::mkLit(v - 1, false));
            }
            minisat->addClause_(lits);
        }
#ifdef SETO_WITH_CADICAL
        else {
            for (int lit : clause) {
                if (lit != 0) cadical->add(-lit);
            }
            cadical->add(0);
        }
#endif
    }
};

InMemorySatSolver::InMemorySatSolver(SatBackend backend)
    : impl_(std::make_unique<Impl>(backend)) {}

InMemorySatSolver::~InMemorySatSolver() = default;

bool InMemorySatSolver::solve(int num_vars,
                              const std::vector<std::vector<int32_t>>& clauses,
                              const std::vector<std::set<int>>* avoid_clauses) {
    if (impl_->backend == SatBackend::MiniSat) {
        impl_->ensure_minisat_var(num_vars);
    }

    for (size_t i = fed_clauses_; i < clauses.size(); ++i) {
        impl_->add_clause(clauses[i]);
    }
    fed_clauses_ = clauses.size();

    if (avoid_clauses != nullptr) {
        for (size_t i = fed_avoid_; i < avoid_clauses->size(); ++i) {
            impl_->add_avoid_clause((*avoid_clauses)[i]);
        }
        fed_avoid_ = avoid_clauses->size();
    }

    if (impl_->backend == SatBackend::MiniSat) {
        if (!impl_->minisat->simplify()) {
            return false;
        }
        Minisat::vec<Minisat::Lit> dummy;
        Minisat::lbool ret = impl_->minisat->solveLimited(dummy);
        using Minisat::lbool;
        if (ret == l_True) {
            const int nv = impl_->minisat->nVars();
            last_model_.assign(static_cast<size_t>(nv), 0);
            for (int i = 0; i < nv; ++i) {
                if (impl_->minisat->model[i] == l_True) {
                    last_model_[static_cast<size_t>(i)] = 1;
                } else if (impl_->minisat->model[i] == l_False) {
                    last_model_[static_cast<size_t>(i)] = -1;
                }
            }
            return true;
        }
        return false;
    }

#ifdef SETO_WITH_CADICAL
    const int res = impl_->cadical->solve();
    if (res == 10) {
        const int nv = std::max(num_vars, impl_->cadical->vars());
        last_model_.assign(static_cast<size_t>(nv), 0);
        for (int v = 1; v <= nv; ++v) {
            const int val = impl_->cadical->val(v);
            last_model_[static_cast<size_t>(v - 1)] = (val > 0) ? 1 : -1;
        }
        return true;
    }
    return false;
#else
    throw std::runtime_error("CaDiCaL backend requested but not compiled");
#endif
}

} // namespace seto
