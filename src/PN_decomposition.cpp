/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include <include/GreedyRemoval.h>
#include "../include/PN_decomposition.h"
#include "../include/FCPN_Merge.h"
#include "include/BDD_encoder.h"
#include "include/Merge.h"
#include "include/SM_composition.h"


using namespace PBLib;
using namespace Minisat;
using namespace Utilities;

set<set<Region *> *> *PN_decomposition::search(int number_of_events,
                                                 const set<Region *>& regions,
                                                 const string& file,
                                                 Pre_and_post_regions_generator *pprg,
                                                 map<int, ER> *ER,
                                                 map<int, int> *aliases,
                                                 set<set<Region *>*>* SMs){
    /* Possible algorithm for the creation of one FCPN with SAT:
     * ALGORITHM STEPS:
     * do
     *      1) (REMOVED) at least one region which covers each state: for each covered state by r1, r2, r3 create a
     *      clause (r1 v r2 v r3)
     *      2) FCPN constraint -> given the regions of a PN these cannot violate the constraint
     *      ALGORITHM:
     *          for each ev
     *              for each r=pre(ev) -> place/region
     *                  if r have more than one exiting event
     *                      for each couple (r, pre(ev))
     *                          if r != pre(ev)
     *                              create clause (!r v !pre(ev))
     *      2b) ACPN constraint:
     *      ALGORITHM:
     *          for each ev
     *              for each r=pre(ev) -> place/region
     *                  if r have more than one exiting event
     *                      for each couple (r, pre(ev))
     *                          if r != pre(ev)
     *                              if pre(ev) have more than one exiting event and these events are not all in common with another place
     *                                  create clause (!r v !pre(ev))
     *      3) (REMOVED -> automatic given a set of regions the events are added later) complete PN structure:
     *          given a sequence r1 -> a -> r2 we have the clause with the bound (r1 and r2 => a) that is (!r1 v !r2 v a)
     *      4) for the next constraint I have to add also constraint related to all connected events of a region
     *          if r is connected to e and e' then r -> (e and e')   becomes !r v (r and e') and then (!r v e) and (!r v e')
     *      4b) really hard new constraint if e has as pre-regions r1 and r2  and as post-regions r3 and r4
     *          e -> (r1 v r2) and e -> (r3 v r4)   we will have clauses (!e v r1 v r2) and (!e v r3 v r4)
     *      4c) safeness: once we have found a set of FCPNs we can check if these are safe, if we find an unsafe FCPN ww add
     *          a new constraint: given an unsafe FCPN containing places p1, ..., pn and not containing places q1, ..., qm
     *          we create a constraint (!p1 or ... or !pn or q1 or ... or qm)
     *      4d) safeness using SMs: once we find an unsafe FCPN we search for an SM, in this way the component is surely
     *          safe. The SM should have also only one token. The constraint for SM structure is:
     *              given an event e
     *                  for each couple of pre-regions/post-regions r1 and r2
     *                      create clause (!r1 v !r2)
     *          The constraint for the initial paces is the following:
     *              Given the set of initial places
     *                  for each couple of initial places r1 and r2
     *                      create clause (!r1 v !r2)
     *          In this way we cannot have a couple of initial places in the same SM but e also have to create a clause
     *          which forces the usage o one initial place: the clauses containing all initial places
     *      5) maximization function: number of new regions used in the result -> max covering
     *      6) OPTIONAL: solve the SAT problem decreasing the value of the region sum -> starting value is the sum of all regions
     *      7) decode result
     * 8) while !EC: previous results clauses are added as results to avoid in future
     * 9) greedy FCPN removal
     */
    auto tStart_partial = clock();
    cout << "=========[FCPN/ACPN DECOMPOSITION MODULE]===============" << endl;
    map<Region *, int> *regions_alias_mapping;
    regions_alias_mapping = get_regions_map(pprg->get_pre_regions());
    auto pre_regions_map = pprg->get_pre_regions();
    auto post_regions_map = pprg->get_post_regions();
    auto minimal_regions = new set<Region *>();
    for(auto reg: regions){
        minimal_regions->insert(reg);
    }
    auto regions_connected_to_labels = merge_2_maps(pre_regions_map,
                                                    post_regions_map);
    auto clauses = new vector<vector<int32_t> *>();
    auto clauses_pre = new vector<vector<int32_t> *>();
    auto structure_clauses = new vector<vector<int32_t> *>();
    auto splitting_constraint_clauses = new vector<vector<int32_t> *>();
    auto fcpn_set = new set<set<Region *> *>(); //todo: transform into a vector
    auto unsafe_fcpns = new set<set<Region *> *>();
    auto not_used_regions = new set<Region *>();
    //create map (region, exiting events)
    auto region_ex_event_map = new map<Region *, set<int> *>();
    auto region_ent_event_map = new map<Region *, set<int> *>();
    for (auto rec: *pre_regions_map) {
        auto ev = rec.first;
        for (auto reg: *rec.second) {
            if (region_ex_event_map->find(reg) == region_ex_event_map->end()) {
                (*region_ex_event_map)[reg] = new set<int>();
            }
            (*region_ex_event_map)[reg]->insert(ev);
        }
    }
    for (auto rec: *post_regions_map) {
        auto ev = rec.first;
        for (auto reg: *rec.second) {
            if (region_ent_event_map->find(reg) == region_ent_event_map->end()) {
                (*region_ent_event_map)[reg] = new set<int>();
            }
            (*region_ent_event_map)[reg]->insert(ev);
        }
    }

    auto results_to_avoid = new vector<set<int>>();
    auto reg_map = new map<Region *, int>();
    auto regions_vector = new vector<Region *>();
    int temp = 0;
    for (auto reg: regions) {
        (*reg_map)[reg] = temp;
        regions_vector->push_back(reg);
        temp++;
    }

    auto region_counter_map = new map<Region *, int>();
    for(auto reg: regions){
        (*region_counter_map)[reg] = 0;
    }

    //creation of clauses used to check if the new constraints can avoid the creation of previously fuond SMs
    /*
    vector<vector<int32_t> *> *clauses_avoiding_created_SMs;
    if(decomposition) {
        clauses_avoiding_created_SMs = new vector<vector<int32_t> *>();
        for (auto SM: *SMs) {
            auto clause = new vector<int32_t>();
            for (auto reg: *SM) {
                clause->push_back(-(*reg_map)[reg] - 1);
            }
            clauses_avoiding_created_SMs->push_back(clause);
        }
    }*/

    for (auto reg: *minimal_regions) {
        not_used_regions->insert(reg);
    }

    vector<int32_t> *clause;

    //encoding: [1, k] regions range: k regions
    //encoding: [k+1, k+m+1] events range: m events
    int m = number_of_events;
    int k = regions.size();
    bool excitation_closure;
    bool splitting_constraints_added = false;

    //STEP 2 and 2b
    //cout << "STEP 2" << endl;
    /*
     * ALGORITHM:
     *      for each ev
     *          for each r=pre(ev) -> place/region
     *              if r has multiple outgoing edges
     *                  for each couple (r, pre(ev))
     *                      if r != pre(ev)
     *                          //in case of 2b check if pre(ev) has multiple outgoing edges
     *                              create clause (!r v !pre(ev))
     */
    //for each ev
    for (auto rec: *pre_regions_map) {
        //auto ev = rec.first;
        auto set_of_regions = rec.second;
        for (auto r: *set_of_regions) {
            if ((*region_ex_event_map)[r]->size() > 1) {
                for (auto r2: *set_of_regions) {
                    if (r != r2) {
                        if(fcptnet){
                            clause = new vector<int32_t>();
                            clause->push_back(-(*reg_map)[r] - 1);
                            clause->push_back(-(*reg_map)[r2] - 1);
                            /*
                            if(decomposition){
                                for(auto cl: *clauses_avoiding_created_SMs){
                                    if(contains(clause, cl)){
                                        cerr << "PROBLEM STEP 2" << endl;
                                    }
                                }
                            }*/
                            structure_clauses->push_back(clause);
                            //print_clause(clause);
                        }
                        //ACPN case
                        else if((*region_ex_event_map)[r2]->size() > 1){
                            bool symmetric_choice = true;
                            for(auto event: *(*region_ex_event_map)[r2]){
                                if((*region_ex_event_map)[r]->find(event) == (*region_ex_event_map)[r]->end()){
                                    symmetric_choice = false;
                                    break;
                                }
                            }
                            if(!symmetric_choice){
                                clause = new vector<int32_t>();
                                clause->push_back(-(*reg_map)[r] - 1);
                                clause->push_back(-(*reg_map)[r2] - 1);
                                structure_clauses->push_back(clause);
                            }
                        }
                    }
                }
            }
        }
    }

    //STEP 4
    for (auto rec:*region_ex_event_map) {
        auto reg = rec.first;
        for (auto ev: *rec.second) {
            int region_encoding = 1 + reg_map->at(reg);
            auto ev_encoding = k + 2 + ev;
            clause = new vector<int32_t>();
            clause->push_back(-region_encoding);
            clause->push_back(ev_encoding);
            /*
            if(decomposition){
                for(auto cl: *clauses_avoiding_created_SMs){
                    if(contains(clause, cl)){
                        cerr << "PROBLEM STEP 4" << endl;
                    }
                }
            }*/
            clauses_pre->push_back(clause);
        }
    }

    for (auto rec:*region_ent_event_map) {
        auto reg = rec.first;
        int region_encoding = 1 + reg_map->at(reg);
        for (auto ev: *rec.second) {
            auto ev_encoding = k + 2 + ev;
            clause = new vector<int32_t>();
            clause->push_back(-region_encoding);
            clause->push_back(ev_encoding);
            /*
            if(decomposition){
                for(auto cl: *clauses_avoiding_created_SMs){
                    if(contains(clause, cl)){
                        cerr << "PROBLEM STEP 4" << endl;
                    }
                }
            }*/
            clauses_pre->push_back(clause);
        }
    }

    //STEP 4b
    for (auto rec: *pre_regions_map) {
        auto ev = rec.first;
        auto ev_encoding = k + 2 + ev;
        clause = new vector<int32_t>();
        clause->push_back(-ev_encoding);
        for (auto reg: *rec.second) {
            int region_encoding = 1 + reg_map->at(reg);
            clause->push_back(region_encoding);
        }
        /*
        if(decomposition){
            for(auto cl: *clauses_avoiding_created_SMs){
                if(contains(clause, cl)){
                    cerr << "PROBLEM STEP 4b" << endl;
                }
            }
        }*/
        clauses_pre->push_back(clause);
    }
    for (auto rec: *post_regions_map) {
        auto ev = rec.first;
        auto ev_encoding = k + 2 + ev;
        clause = new vector<int32_t>();
        clause->push_back(-ev_encoding);
        for (auto reg: *rec.second) {
            int region_encoding = 1 + reg_map->at(reg);
            clause->push_back(region_encoding);
        }
        /*
        if(decomposition){
            for(auto cl: *clauses_avoiding_created_SMs){
                if(contains(clause, cl)){
                    cerr << "PROBLEM STEP 4b" << endl;
                }
            }
        }*/
        clauses_pre->push_back(clause);
    }

    //STEP 4d
    vector<vector<int32_t> *> *SM_clauses;
    if(safe_components_SM){
        SM_clauses = new vector<vector<int32_t> *>();
        set<Region*> initial_regions;
        for(auto reg: regions){
            if(is_initial_region(reg)){
                initial_regions.insert(reg);
            }
        }
        clause = new vector<int32_t>();
        //at least one initial place
        for(auto reg: initial_regions){
            int region_encoding = 1 + reg_map->at(reg);
            clause->push_back(region_encoding);
        }
        /*
        if(decomposition){
            for(auto cl: *clauses_avoiding_created_SMs){
                if(contains(clause, cl)){
                    cerr << "PROBLEM STEP 4d" << endl;
                }
            }
        }*/
        SM_clauses->push_back(clause);
        for(auto rec: *pre_regions_map){
            for(auto reg1: *rec.second){
                for(auto  reg2: *rec.second){
                    if(reg1 != reg2) {
                        clause = new vector<int32_t>();
                        int region_encoding = 1 + reg_map->at(reg1);
                        clause->push_back(-region_encoding);
                        region_encoding = 1 + reg_map->at(reg2);
                        clause->push_back(-region_encoding);
                        /*
                        if(decomposition){
                            for(auto cl: *clauses_avoiding_created_SMs){
                                if(contains(clause, cl)){
                                    cerr << "PROBLEM STEP 4d" << endl;
                                }
                            }
                        }*/
                        SM_clauses->push_back(clause);
                    }
                }
            }
        }
        for(auto rec: *post_regions_map){
            for(auto reg1: *rec.second){
                for(auto  reg2: *rec.second){
                    if(reg1 != reg2) {
                        clause = new vector<int32_t>();
                        int region_encoding = 1 + reg_map->at(reg1);
                        clause->push_back(-region_encoding);
                        region_encoding = 1 + reg_map->at(reg2);
                        clause->push_back(-region_encoding);
                        /*
                        if(decomposition){
                            for(auto cl: *clauses_avoiding_created_SMs){
                                if(contains(clause, cl)){
                                    cerr << "PROBLEM STEP 4d" << endl;
                                }
                            }
                        }*/
                        SM_clauses->push_back(clause);
                    }
                }
            }
        }
    }

    bool last_result_unsafe = false;
    splitting_constraints_added = false;
    bool deadlock_achieved = false;
    int dd_counter = 0;
    int unsafe_components_counter = 0;
    double t_now;
    bool time_out = false;

    do {
        t_now = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;
        if(t_now > 3600){
            time_out = true;
            break;
        }
        for (auto cl: *clauses) {
            delete cl;
        }
        clauses->clear();

        if ((no_reset && deadlock_achieved) || (!no_reset && (deadlock_achieved || !splitting_constraints_added))) {
            if (!splitting_constraint_clauses->empty()) {
                splitting_constraint_clauses->clear();
                //cout << "removing splitting constraints" << endl;
            }
        }

        //STEP 5
        //cout << "STEP 5" << endl;
        vector<WeightedLit> literals_from_regions = {};
        literals_from_regions.reserve(k); //improves the speed
        for (int i = 0; i < k; i++) {
            if(!region_counter) {
                if (not_used_regions->find((*regions_vector)[i]) != not_used_regions->end()) {
                    literals_from_regions.emplace_back(1 + i, 1);
                } else {
                    literals_from_regions.emplace_back(1 + i, 0);
                }
            }
            else{
                if (not_used_regions->find((*regions_vector)[i]) != not_used_regions->end()) {
                    if(region_counter_map->at((*regions_vector)[i]) > 5){
                        literals_from_regions.emplace_back(1 + i, 0);
                    }
                    else{
                        literals_from_regions.emplace_back(1 + i, 1);
                    }
                } else {
                    literals_from_regions.emplace_back(1 + i, 0);
                }
            }
        }

        int current_value = 1;
        int min = 0;
        int max = k;

        PBConfig config = make_shared<PBConfigClass>();
        VectorClauseDatabase formula(config);
        PB2CNF pb2cnf(config);
        AuxVarManager auxvars(k + m + 2);
        for (auto cl: *clauses_pre) {
            formula.addClause(*cl);
        }
        for (auto cl: *clauses) {
            formula.addClause(*cl);
        }
        for (auto cl: *splitting_constraint_clauses) {
            formula.addClause(*cl);
        }
        if(safe_components_SM) {
            //the check on splitting constraints is right but in some cases with this constraint the result is reached
            // a way slower because without it, we directly search for SMs improving the search speed
            if (last_result_unsafe /*&& !splitting_constraints_added*/) {
                for (auto cl: *SM_clauses) {
                    formula.addClause(*cl);
                }
                //cout << "next search is an SM" << endl;
            }
            else{
                //cout << "next FCPN" << endl;
                for (auto cl: *structure_clauses) {
                    formula.addClause(*cl);
                }
            }
        }
        else{
            for (auto cl: *structure_clauses) {
                formula.addClause(*cl);
            }
        }
        Minisat::Solver solver;

        bool sat;
        string dimacs_file;
        bool exists_solution = false;

        auto last_solution = new set<int>();

        IncPBConstraint constraint(literals_from_regions, GEQ,
                                   current_value); //the sum have to be greater or equal to current_value
        pb2cnf.encodeIncInital(constraint, formula, auxvars);
        //iteration in the search of a correct assignment decreasing the total weight
        do {
            int num_clauses_formula = formula.getClauses().size();
            //cout << "formula 1" << endl;
            //formula.printFormula(cout);
            dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                            formula.getClauses(), results_to_avoid);
            sat = check_sat_formula_from_dimacs(solver, dimacs_file);
            if (sat) {
                //cout << "SAT" << endl;
                exists_solution = true;

                if (decomposition_debug) {
                    cout << "SAT with value " << current_value << ": representing the number of new covered regions"
                         << endl;
                    //cout << "Model: ";
                }
                last_solution->clear();
                for (int i = 0; i < solver.nVars(); ++i) {
                    if (solver.model[i] != l_Undef) {
                        /*
                        if (decomposition_debug) {
                            fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                    i + 1);
                        }*/
                        if (i < k) {
                            if (solver.model[i] == l_True) {
                                last_solution->insert(i + 1);
                            } else {
                                last_solution->insert(-i - 1);
                            }
                        }
                    }
                }
                min = current_value;
            } else {
                if (decomposition_debug) {
                    //cout << "----------" << endl;

                    cout << "UNSAT with value " << current_value << endl;/*
                    if (exists_solution) {
                        cout << "Model: ";
                        for (int i = 0; i < solver.nVars(); ++i) {
                            if (solver.model[i] != l_Undef) {
                                fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                        i + 1);
                            }
                        }
                        cout << endl;
                    }*/
                }
                max = current_value;
            }
            current_value = (min + max) / 2;
        } while ((max - min) > 1);

        if (!no_fcpn_min) {
            //STEP 6
            if (decomposition_debug)
                cout << "TRYING TO DECREASE THE NUMBER OF REGIONS" << endl;


            int current_value2 = 0;
            for (auto val: *last_solution) {
                if (val > 0) {
                    current_value2++;
                }
            }
            current_value2--;
            int min2 = 0;
            int max2 = current_value2;

            vector<WeightedLit> sum_of_regions = {};
            sum_of_regions.reserve(k);
            for (int i = 0; i < k; i++) {
                sum_of_regions.emplace_back(1 + i, 1);
            }


            int num_clauses_formula;
            //cout << "formula" << endl;
            //formula.printFormula(cout);

            Minisat::Solver *solver2;

            //the sum have to be equal to current_value

            PBConfig config2 = make_shared<PBConfigClass>();
            VectorClauseDatabase formula2(config2);
            AuxVarManager auxvars2(k + m + 2);
            PBConstraint constraint3(literals_from_regions, BOTH,
                                     current_value, current_value);
            /*
            do {
                solver2 = new Minisat::Solver();
                auxvars2.resetAuxVarsTo(k + m + 2);
                formula2.clearDatabase();
                for (auto cl: *clauses) {
                    formula2.addClause(*cl);
                }
                pb2cnf.encode(constraint3, formula2, auxvars2);

                if (decomposition_debug)
                    cout << "values: " << current_value << ", " << current_value2 << endl;
                PBConstraint constraint2(sum_of_regions, LEQ,
                                         current_value2); //the sum have to be lesser or equal to current_value2
                pb2cnf.encode(constraint2, formula2, auxvars2);

                num_clauses_formula = formula2.getClauses().size();

                if (decomposition_debug)
                    cout << "Formula size: " << formula2.getClauses().size() << endl;

                dimacs_file = convert_to_dimacs(file, auxvars2.getBiggestReturnedAuxVar(),
                                                num_clauses_formula,
                                                formula2.getClauses(), results_to_avoid);
                sat = check_sat_formula_from_dimacs(*solver2, dimacs_file);

                if (sat) {
                    if (decomposition_debug) {
                        cout << "(Decreasing) SAT with values " << current_value << ", " << current_value2 << endl;
                        cout << "Model: ";
                    }
                    last_solution->clear();
                    for (int i = 0; i < solver2->nVars(); ++i) {
                        if (solver2->model[i] != l_Undef) {
                            if (decomposition_debug) {
                                fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver2->model[i] == l_True) ? "" : "-",
                                        i + 1);
                            }
                            if (i < k) {
                                if (solver2->model[i] == l_True) {
                                    last_solution->insert(i + 1);
                                } else {
                                    last_solution->insert(-i - 1);
                                }
                            }
                        }
                    }
                    if (decomposition_debug)
                        cout << endl;
                    max2 = current_value2;
                } else {
                    if (decomposition_debug) {
                        cout << "(Decreasing) UNSAT with values " << current_value << ", " << current_value2 << endl;
                    }
                    min2 = current_value2;
                }
                current_value2 = (min2 + max2) / 2;
                delete solver2;
            } while ((max2 - min2) > 1);
            */
        }

        if (exists_solution) {
            //STEP 7
            auto temp_PN = new set<Region *>();
            for (auto r_index: *last_solution) {
                if (r_index > 0) {
                    temp_PN->insert((*regions_vector)[r_index - 1]);
                }
            }
            splitting_constraints_added = false;
            //todo: don't forget the memory leaks -> have to check with valgrind
            auto new_temp_set = split_not_connected_regions(temp_PN, regions_connected_to_labels);
            if (new_temp_set->size() > 1) {
                //change of strategy: take the biggest PN as candidate, iff there is the safeness check this PN will
                // be used
                if(deadlock_achieved){
                    //todo probably here it's better to take as candidate the PN with the best coverage of unused regions
                    int max_size = (*new_temp_set)[0].size();
                    int pos = 0;
                    //searching the smallest PN
                    for (int i = 1; i < new_temp_set->size(); i++) {
                        if ((*new_temp_set)[i].size() > max_size) {
                            max_size = (*new_temp_set)[i].size();
                            pos = i;
                        }
                    }
                    delete temp_PN;
                    temp_PN = new set<Region *>();
                    for(auto reg: (*new_temp_set)[pos]){
                        temp_PN->insert(reg);
                    }
                    splitting_constraints_added = false;
                    //cout << "temp big PN" << endl;
                }
                else {
                    //cout << "set of regions containing more PNs" << endl;
                    int min_size = (*new_temp_set)[0].size();
                    int pos = 0;
                    //searching the smallest PN
                    for (int i = 1; i < new_temp_set->size(); i++) {
                        if ((*new_temp_set)[i].size() < min_size) {
                            min_size = (*new_temp_set)[i].size();
                            pos = i;
                        }
                    }
                    //cout << "avoiding small PN:" << endl;
                    //println((*new_temp_set)[pos]);
                    //print_SM_on_file((*new_temp_set)[pos], "DEBUG_small.txt");
                    /*
                    if (decomposition) {
                        for (auto SM: *SMs) {
                            if (*SM == (*new_temp_set)[pos]) {
                                cout << "small forbidden PN is actually a good SM:" << endl;
                                //todo cambiare i vincoli, se nego le piccole SM poi non arrivo ad avere una soluzione forse
                                println((*new_temp_set)[pos]);
                                break;
                            }
                        }
                    }*/
                    //add clause which avoids this PN as a solution
                    clause = new vector<int32_t>();
                    for (auto reg: (*new_temp_set)[pos]) {
                        //cout << "added a new constraint" << endl;
                        clause->push_back(-1 - reg_map->at(reg));
                    }
                    splitting_constraint_clauses->push_back(clause);
                    delete temp_PN;
                    splitting_constraints_added = true;
                    //without adding the following line the search of an SM continues until it finds one valid
                    // bigger chance to find an FCPN but slows down the computation being a BFS respect to the DFS
                    //last_result_unsafe = false;
                }
            }
            if(!splitting_constraints_added){
                //cout << "set with one PN" << endl;
                bool safe = true;
                if(safe_components || safe_components_SM){
                    safe = safeness_check(temp_PN, pre_regions_map, post_regions_map, regions_alias_mapping);
                }
                if((!safe_components && !safe_components_SM) || (safe_components && safe) || (safe_components_SM && safe)) {
                    for (auto val: *last_solution) {
                        if (val > 0) {
                            /*if(decomposition_debug) {
                                cout << val << ": ";
                                println(*regions_vector->at(val - 1));
                            }*/
                            if (val <= k) {
                                if (temp_PN->find(regions_vector->at(val - 1)) != temp_PN->end())
                                    if (not_used_regions->find(regions_vector->at(val - 1)) != not_used_regions->end())
                                        not_used_regions->erase(regions_vector->at(val - 1));
                            }
                        }
                    }
                    if(count_SMs && safe_components_SM){
                        if(last_result_unsafe)
                            cout << "found SM" << endl;
                        else
                            cout << "found FCPN" << endl;
                    }
                    fcpn_set->insert(temp_PN);
                    cout << "adding new FCPN to solution (size: " << temp_PN->size() << ")" << endl;
                    if (decomposition_debug) {
                        println(temp_PN);
                    }
                    last_result_unsafe = false;
                    //cout <<"SAFE or SAFENESS NOT CHECKED" << endl;
                    deadlock_achieved = false;
                }
                else{
                    unsafe_components_counter++;
                    if(region_counter){
                        for(auto reg: *temp_PN){
                            region_counter_map->at(reg)++;
                        }
                    }
                    /*
                    bool equal = false;
                    for(auto fcpn: *unsafe_fcpns){
                        if(contains(fcpn, temp_PN)){
                            if(contains(temp_PN, fcpn)){
                                equal = true;
                                break;
                            }
                        }
                    }
                    if(equal){
                        cout << "adding an FCPN added previously" << endl;
                    }*/
                    unsafe_fcpns->insert(temp_PN);
                    //cout << "NOT SAFE PN" << endl;
                    //println(temp_PN);
                    //if(decomposition_debug) {
                    //cout << "avoiding the following UNSAFE PN permanently:" << endl;
                    //println(temp_PN);
                    //print_SM_on_file(*temp_PN, "DEBUG_unsafe.txt");
                    /*
                    if(decomposition) {
                        for (auto SM: *SMs) {
                            if(*SM == *temp_PN){
                                cout << "unsafe forbidden PN is actually a good SM:" << endl;
                                println(*temp_PN);
                                break;
                            }
                        }
                    }*/
                    //}
                    last_result_unsafe = true;
                    //cout << "UNSAFE (adding constraint)" << endl;
                    clause = new vector<int32_t>();
                    for(auto reg: *temp_PN){
                        clause->push_back(-1 - reg_map->at(reg));
                    }
                    //print_clause(clause);
                    clauses_pre->push_back(clause);
                }
            }
            delete new_temp_set;

            //fortunately this case never occurs
            /*if(contains(results_to_avoid, *last_solution)){
                cout << "adding an already existing PN" << endl;
            }*/

            results_to_avoid->push_back(*last_solution);
            //cout << "results to avoid size: " << results_to_avoid->size() << endl;

            if (!splitting_constraints_added &&
                (!safe_components || (safe_components && !last_result_unsafe)) &&
                (!safe_components_SM || (safe_components_SM && !last_result_unsafe))) {
                auto used_regions_map = get_map_of_used_regions(fcpn_set, pprg->get_pre_regions());
                excitation_closure = is_excitation_closed(used_regions_map, ER);
                //cout << "EC checked" << endl;
                for (auto rec: *used_regions_map) {
                    delete rec.second;
                }
                delete used_regions_map;
                /*
                if (excitation_closure) {
                    cout << "ec ok" << endl;
                }*/
                if(region_counter){
                    for(auto rec: *region_counter_map){
                        rec.second = 0;
                    }
                }
            }
            formula.clearDatabase();
        } else {
            if(deadlock_achieved) {
                cout << "no solution found" << endl;
                exit(0);
            }
            deadlock_achieved = true;
            dd_counter++;
            cout << "deadlock, changing method (" << dd_counter << ")" << endl;

        }
        delete last_solution;
    } while (!excitation_closure);


    if(time_out){
        cout << "TIME OUT !!!" << endl;
        cout << "Found " << unsafe_components_counter << " unsafe FCPNs" << endl;
        cout << "Found " << fcpn_set->size() << " safe FCPNs" << endl;
        int counter = 0;
        for(auto fcpn: *fcpn_set){
            cout << "FCPN " << counter << endl;
            counter++;
            println(*fcpn);
        }
        auto map_of_FCPN_pre_regions = new map < set<Region *> *, map<int, set<Region*> *> * > ();
        auto map_of_FCPN_post_regions = new map < set<Region *> *, map<int, set<Region*> *> * > ();

        for (auto FCPN: *fcpn_set) {
            (*map_of_FCPN_pre_regions)[FCPN] = new map < int, set<Region*> * > ();
            for (auto rec: *pprg->get_pre_regions()) {
                for (auto reg: *rec.second) {
                    if (FCPN->find(reg) != FCPN->end()) {
                        if((*map_of_FCPN_pre_regions)[FCPN]->find(rec.first) == (*map_of_FCPN_pre_regions)[FCPN]->end()){
                            (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first] = new set<Region *>();
                        }
                        (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first]->insert(reg);
                    }
                }
            }
            (*map_of_FCPN_post_regions)[FCPN] = Pre_and_post_regions_generator::create_post_regions_for_FCPN((*map_of_FCPN_pre_regions)[FCPN]);
        }
        int pn_counter = 0;
        auto regions_mapping = get_regions_map(pprg->get_pre_regions());
        for (auto pn: *fcpn_set) {
            print_pn_dot_file(regions_mapping, map_of_FCPN_pre_regions->at(pn), map_of_FCPN_post_regions->at(pn),
                              aliases,
                              file, pn_counter);
            pn_counter++;
        }
        exit(0);
    }
    else{
        cout << "Found " << unsafe_components_counter << " unsafe FCPNs" << endl;
        auto map_of_FCPN_pre_regions = new map < set<Region *> *, map<int, set<Region*> *> * > ();
        auto map_of_FCPN_post_regions = new map < set<Region *> *, map<int, set<Region*> *> * > ();

        for (auto FCPN: *unsafe_fcpns) {
            (*map_of_FCPN_pre_regions)[FCPN] = new map < int, set<Region*> * > ();
            for (auto rec: *pprg->get_pre_regions()) {
                for (auto reg: *rec.second) {
                    if (FCPN->find(reg) != FCPN->end()) {
                        if((*map_of_FCPN_pre_regions)[FCPN]->find(rec.first) == (*map_of_FCPN_pre_regions)[FCPN]->end()){
                            (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first] = new set<Region *>();
                        }
                        (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first]->insert(reg);
                    }
                }
            }
            (*map_of_FCPN_post_regions)[FCPN] = Pre_and_post_regions_generator::create_post_regions_for_FCPN((*map_of_FCPN_pre_regions)[FCPN]);
        }
        int pn_counter = 0;
        auto regions_mapping = get_regions_map(pprg->get_pre_regions());
        for (auto pn: *unsafe_fcpns) {
            print_pn_dot_file(regions_mapping, map_of_FCPN_pre_regions->at(pn), map_of_FCPN_post_regions->at(pn),
                              aliases,
                              file, pn_counter, true);
            pn_counter++;
        }
        //exit(0);
    }

    for(auto cl: *splitting_constraint_clauses){
        delete cl;
    }
    delete splitting_constraint_clauses;

    if(decomposition_debug) {
        for (auto pn: *fcpn_set) {
            cout << "PN: (size:" << pn->size() << ")" << endl;
            println(pn);
        }
    }

    cout << "PNs before greedy: " << fcpn_set->size() << endl;
    /*
    for(auto pn: *fcpn_set){
        cout << "FCPN:" << endl;
        println(pn);
    }*/

    places_after_initial_decomp = 0;
    for(auto pn: *fcpn_set){
        places_after_initial_decomp += pn->size();
    }

    cout << "Number of places before greedy: " << places_after_initial_decomp << endl;

    //STEP 9
    if(fcpn_set->size() > 1) {
        if(decomposition) {
            cout << "PNs before greedy (with the addition of SMs): " << fcpn_set->size() << endl;
        }
        if(greedy_exact)
            GreedyRemoval::minimize_sat(fcpn_set, SMs, ER, pre_regions_map, file);
        else
            GreedyRemoval::minimize(fcpn_set, pprg, ER, pre_regions_map);
    }

    cout << (fcptnet ? "FCPN" : "ACPN") << " set size: " << fcpn_set->size() << endl;



    places_after_greedy = 0;
    for(auto pn: *fcpn_set){
        places_after_greedy += pn->size();
    }

    cout << "Number of places after greedy (before merge): " << places_after_greedy << endl;

    auto map_of_FCPN_pre_regions = new map < set<Region *> *, map<int, set<Region*> *> * > ();
    auto map_of_FCPN_post_regions = new map < set<Region *> *, map<int, set<Region*> *> * > ();

    for (auto FCPN: *fcpn_set) {
        (*map_of_FCPN_pre_regions)[FCPN] = new map < int, set<Region*> * > ();
        for (auto rec: *pprg->get_pre_regions()) {
            for (auto reg: *rec.second) {
                if (FCPN->find(reg) != FCPN->end()) {
                    if((*map_of_FCPN_pre_regions)[FCPN]->find(rec.first) == (*map_of_FCPN_pre_regions)[FCPN]->end()){
                        (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first] = new set<Region *>();
                    }
                    (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first]->insert(reg);
                }
            }
        }
        (*map_of_FCPN_post_regions)[FCPN] = Pre_and_post_regions_generator::create_post_regions_for_FCPN((*map_of_FCPN_pre_regions)[FCPN]);
    }

    if(count_SMs && safe_components_SM){
        int num_SMs = count_number_SMs(map_of_FCPN_pre_regions, map_of_FCPN_post_regions, fcpn_set);
        cout << "number of SMs: " << num_SMs << endl;
    }

    if(fcpn_set->size() == 1){
        no_merge = true;
    }

    if(only_safeness_check){
        for(auto pn: *fcpn_set){
            bool safe = safeness_check(pn, pre_regions_map, post_regions_map/*, region_ent_event_map*/);
            if(safe){
                cout << "SAFENESS CHECK PASSED!!!" << endl;
            }
            else{
                cerr << "SAFENESS CHECK NOT PASSED!!!" << endl;
            }
        }
    }

    if(!no_merge) {
        //todo: bug with pparb_2_3 -> creates a PN with an event without pre-regions, even if this UNSAFE pn combined
        // with others is still in bisimulation with original LTS
        auto merge = new FCPN_Merge(fcpn_set, number_of_events, map_of_FCPN_pre_regions, map_of_FCPN_post_regions, file,
                                    aliases);
        delete merge;
        /*
        auto used_regions_map = get_map_of_used_regions(fcpn_set, pre_regions_map);
        excitation_closure = is_excitation_closed(used_regions_map, ER);
        if(excitation_closure){
            cout << "EC OK" << endl;
        }
        else{
            cerr << "EC not satisfied -> WRONG DECOMPOSITION" << endl;
            exit(1);
        }
        for(auto rec: *used_regions_map){
            delete rec.second;
        }
        delete used_regions_map;*/
    }
    else if(output){
        int pn_counter = 0;
        auto regions_mapping = get_regions_map(pprg->get_pre_regions());
        for (auto pn: *fcpn_set) {
            print_pn_dot_file(regions_mapping, map_of_FCPN_pre_regions->at(pn), map_of_FCPN_post_regions->at(pn),
                              aliases,
                              file, pn_counter);
            pn_counter++;
        }
        delete regions_mapping;
    }
    if(composition)
        PN_composition::compose(fcpn_set, map_of_FCPN_pre_regions, map_of_FCPN_post_regions, aliases, file);

    if (check_structure)
        check_EC_and_structure(ER, map_of_FCPN_pre_regions, map_of_FCPN_post_regions, pre_regions_map, region_ex_event_map, fcpn_set);

    maxAlphabet = getMaxAlphabet(map_of_FCPN_pre_regions, aliases);
    avgAlphabet = getAvgAlphabet(map_of_FCPN_pre_regions, aliases);

    delete regions_vector;

    for (auto rec: *region_ex_event_map) {
        delete rec.second;
    }
    delete region_ex_event_map;

    for (auto rec: *region_ent_event_map) {
        delete rec.second;
    }
    delete region_ent_event_map;

    for (auto rec: *regions_connected_to_labels) {
        delete rec.second;
    }
    delete regions_connected_to_labels;

    delete results_to_avoid;

    delete reg_map;
    delete not_used_regions;
    for (auto cl: *clauses) {
        delete cl;
    }
    delete clauses;
    for (auto cl: *clauses_pre) {
        delete cl;
    }
    delete clauses_pre;
    for (auto cl: *structure_clauses) {
        delete cl;
    }
    delete structure_clauses;
    if(SM_clauses){
        for(auto cl: *SM_clauses){
            delete cl;
        }
        delete SM_clauses;
    }

    delete minimal_regions;


    for(auto rec: *map_of_FCPN_pre_regions){
        for(auto rec1: *rec.second){
            delete rec1.second;
        }
        delete rec.second;
    }
    delete map_of_FCPN_pre_regions;
    for(auto rec: *map_of_FCPN_post_regions){
        for(auto rec1: *rec.second){
            delete rec1.second;
        }
        delete rec.second;
    }
    delete map_of_FCPN_post_regions;

    delete region_counter_map;

    return fcpn_set;
}

set<set<Region *> *> *PN_decomposition::search_k(int number_of_events,
                                               set<Region *> *regions,
                                               const string& file,
                                               Pre_and_post_regions_generator *pprg,
                                               map<int, ER> *ER,
                                               map<int, int> *aliases,
                                               BDD_encoder *be) {

    //encoding: [1, m] symbolic events range: m events
    //encoding: [m+1, k+m] symbolic FCPN regions range: k regions, each region represent a set of regions in different
    // FCPNs
    //encoding: [k+m+1, k+2m] first FCPN range: m events
    //encoding: [k+2m+1, 2k+2m] first FCPN regions range: k regions
    //encoding: [2k+2m+1, 2k+3m] first FCPN range: m events
    //encoding: [2k+3m+1, 3k+3m] first FCPN regions range: k regions
    // ... and so on, regions+events for each FCPN

    /*
     * 0) I want to satisfy all events: not necessary but speed-up the computation
     * 1) EC clauses
     * 2) FCPN constraint
     * 2b) SM constraint
     * 2c) Safeness for SMs (in case of FCPNs I can have two initial regions in the same FCPN and still have a safe FCPN
     *      but the FCPN could be also unsafe): given an SM at most one region containing initial sate can take part of it:
     *      given the set of regions create couples of clauses between regions with non empty intersection for each SM
     *      (!r1 v !r2)
     * 3) binding between symbolic region and all it's representations in different FCPNs:
     *      I can have a symbolic region sr1 and two FCPNs, if I want to satisfy sr1 I will have sr1 -> r1 v r1'
     *      it will become (!sr1 v r1 v r1')
     * 4) for the next constraint I have to add also constraint related to all connected events of a region
     *    if r is connected to e and e' then r -> (e and e')   becomes !r v (e and e') and then (!r v e) and (!r v e')
     * 5) in order to create visually great FCPNs: if e has as pre-regions r1 and r2  and as post-regions r3 and r4
     *          e -> (r1 v r2) and e -> (r3 v r4)   we will have clauses (!e v r1 v r2) and (!e v r3 v r4)
     * 6) binding between symbolic events and FCPN events [maybe this constraint can be removed because we have
     *      EC constraint???]
     * 7) TODO: (optional) minimization function: number of used regions
     * 8) decoding
     * 9) safeness: once we have found a set of FCPNs we can check if these are safe, if we find an unsafe FCPN ww add
     * a new constraint: given an unsafe FCPN containing places p1, ..., pn and not containing places q1, ..., qm
     * we create a constraint (!p1 or ... or !pn or q1 or ... or qm)
     * 10) OPTIMAL RESULT SEARCH: if we find a suboptimal result with a set of regions which corresponds to two
     *          FCPNs add this set as forbidden one
     * //todo: core dump with safeness on vme_write
     */

    cout << "=========[k-PN DECOMPOSITION MODULE (WITH BDD)]===============" << endl;

    map<Region *, int> *regions_alias_mapping;
    auto regions_alias_mapping_inverted = new map<int,Region *>();
    regions_alias_mapping = get_regions_map(pprg->get_pre_regions());
    for(auto rec: *regions_alias_mapping){
        (*regions_alias_mapping_inverted)[rec.second] = rec.first;
    }

    set<set<int>> *ECClauses;
    auto solution = new vector<int>();

    bool solution_found = false;
    int num_FCPNs_try = 1;
    auto regions_copy = new set<Region *>();
    auto regions_vector = new vector<Region *>();
    for (auto reg: *regions) {
        regions_copy->insert(reg);
        regions_vector->push_back(reg);
    }
    auto pre_regions_map = pprg->get_pre_regions();
    auto post_regions_map = pprg->get_post_regions();

    auto regions_connected_to_labels = merge_2_maps(pre_regions_map,
                                                    post_regions_map);
    auto initial_regions = new vector<Region *>();
    for(auto reg: *regions){
        if(is_initial_region(reg))
            initial_regions->push_back(reg);
    }
    auto clauses = new vector<vector<int32_t> *>();
    auto fcpn_set = new set<set<Region *> *>(); //todo: transform into a vector
    //create map (region, exiting events)
    auto region_ex_event_map = new map<Region *, set<int> *>();
    auto region_ent_event_map = new map<Region *, set<int> *>();
    for (auto rec: *pre_regions_map) {
        auto ev = rec.first;
        for (auto reg: *rec.second) {
            if (region_ex_event_map->find(reg) == region_ex_event_map->end()) {
                (*region_ex_event_map)[reg] = new set<int>();
            }
            (*region_ex_event_map)[reg]->insert(ev);
        }
    }
    for (auto rec: *post_regions_map) {
        auto ev = rec.first;
        for (auto reg: *rec.second) {
            if (region_ent_event_map->find(reg) == region_ent_event_map->end()) {
                (*region_ent_event_map)[reg] = new set<int>();
            }
            (*region_ent_event_map)[reg]->insert(ev);
        }
    }

    vector<int32_t> *clause;


    int m = number_of_events;
    int k = regions_copy->size();
    bool excitation_closure;
    bool splitting_constraints_added = false;

    if (decomposition_debug) {
        cout << "m: " << m << endl;
        cout << "k: " << k << endl;
    }


    if (decomposition_debug) {
        cout << "Pre-regions" << endl;
        for (auto rec: *pre_regions_map) {
            auto ev = rec.first;
            if(ev < num_events_before_label_splitting)
                cout << "event: " << ev << " (" << aliases_map_number_name->at(ev) << ")" <<endl;
            else
                cout << "event: " << ev << " (" << aliases_map_number_name->at(aliases->at(ev)) << ")" <<endl;
            for (auto reg: *rec.second) {
                cout << "r" << regions_alias_mapping->at(reg) << ": ";
                println(*reg);
            }
        }

        cout << "Post-regions" << endl;
        for (auto rec: *post_regions_map) {
            auto ev = rec.first;
            cout << "event: " << ev << endl;
            for (auto reg: *rec.second) {
                cout << "r" << regions_alias_mapping->at(reg) << ": ";
                println(*reg);
            }
        }

        cout << "-----" << endl;
    }

    auto cache = new set<set<Region *>>();

    auto clauses_pre = new vector<vector<int32_t> *>();

    auto forbidden_pns = new set<set<Region *>>();

    //STEP 0
    for (int i = 0; i < num_events_after_splitting; ++i) {
        clause = new vector<int32_t>();
        clause->push_back(i + 1);
        clauses_pre->push_back(clause);
    }

    //STEP 1
    if(decomposition_debug)
        cout << "STEP 1" << endl;
    be->encode(regions, regions_alias_mapping, pre_regions_map);

    if(decomposition_debug) {
        cout << "regions alias mapping:" << endl;
        for(auto rec: *regions_alias_mapping){
            cout << rec.second << ": ";
            println(*rec.first);
        }
    }
    auto ECTmpClauses = be->get_clauses();

    for (auto reg_set: *ECTmpClauses) {
        clause = new vector<int32_t>();
        for (auto reg: *reg_set) {
            //adding a symbolic region
            if(reg >= 0) {
                clause->push_back(m + reg);
            }
            else{
                clause->push_back(-m + reg);
            }
        }

        clauses_pre->push_back(clause);
        /*
        if (decomposition_debug) {
            cout << "-----------------------------" << endl;
            print_clause(clause);
        }*/
    }

    //step 2c pair search
    vector<pair<Region *, Region *> *> *intersecting_pairs;
    if(decomposition) {
        intersecting_pairs = new vector<pair<Region *, Region *> *>();
        for (int i = 0; i < initial_regions->size(); ++i) {
            for (int s = i + 1; s < initial_regions->size(); ++s) {
                auto r1 = (*initial_regions)[i];
                auto r2 = (*initial_regions)[s];
                if (!empty_regions_intersection(r1, r2)) {
                    intersecting_pairs->push_back(new pair(r1, r2));
                }
            }
        }
    }

    auto results_to_avoid = new vector<set<Region *>>();

    bool dont_increase_counter;
    bool suboptimal_result_found = false;

    do {
        bool safe = true;
        dont_increase_counter = false;
        for (auto cl: *clauses) {
            delete cl;
        }
        clauses->clear();

        cache->clear();

        if (decomposition_debug) {
            if (fcptnet)
                cout << "STEP 2: FCPN constraint" << endl;
            else
                cout << "STEP 2b: SM constraint" << endl;
        }
        //SM constraint
        /*
        ALORITHM:
        *          for each ev
        *              if (|pre(ev) > 1|)
        *                  for each (r1,r2) \in pre(ev) -> place/region
        *                      create clause  (!r1 v ! r2)
        *              if (|post(ev) > 1|)
        *                  for each (r1,r2) \in post(ev) -> place/region
        *                      create clause  (!r1 v ! r2)
        */
        if(!fcptnet && !acpn){
            for (auto rec: *pre_regions_map) {
                //auto ev = rec.first;
                if(rec.second->size() > 1){
                    auto reg_vec = new vector<Region *>(rec.second->begin(),rec.second->end());
                    for(int i=0;i< reg_vec->size();++i){
                        for(int s=i+1;s<reg_vec->size();++s){
                            auto r1 = reg_vec->at(i);
                            auto r2 = reg_vec->at(s);
                            clause = new vector<int32_t>();
                            int offset = k_search_region_offset(m, k, num_FCPNs_try);
                            clause->push_back(-(*regions_alias_mapping)[r1] - offset);
                            clause->push_back(-(*regions_alias_mapping)[r2] - offset);
                            clauses_pre->push_back(clause);

                            if (decomposition_debug) {
                                print_clause(clause);
                                cout << "conflict" << endl;
                                println(*r1);
                                println(*r2);
                            }
                        }
                    }
                    delete reg_vec;
                }
            }
        }
        else {
            //STEP 2: FCPN constraint
            /*
             * ALGORITHM:
             *      for each ev
             *          for each r=pre(ev) -> place/region
             *              if r has multiple outgoing edges
             *                  for each couple (r, pre(ev))
             *                      if r != pre(ev)
             *                          //in case of 2b check if pre(ev) has multiple outgoing edges
             *                              for each FCPN
             *                                  if not added previously
             *                                      create clause (!r v !pre(ev))
             */
            //for each ev
            for (auto rec: *pre_regions_map) {
                //auto ev = rec.first;
                auto set_of_regions = rec.second;
                for (auto r: *set_of_regions) {
                    if ((*region_ex_event_map)[r]->size() > 1) {
                        for (auto r2: *set_of_regions) {
                            if (r != r2) {
                                if (fcptnet) {
                                    auto check = new set<Region *>();
                                    check->insert(r);
                                    check->insert(r2);
                                    if (cache->find(*check) == cache->end()) {
                                        cache->insert(*check);
                                        clause = new vector<int32_t>();
                                        int offset = k_search_region_offset(m, k, num_FCPNs_try);
                                        clause->push_back(-(*regions_alias_mapping)[r] - offset);
                                        clause->push_back(-(*regions_alias_mapping)[r2] - offset);
                                        clauses_pre->push_back(clause);
                                        /*
                                        if (decomposition_debug) {
                                            print_clause(clause);
                                            cout << "conflict" << endl;
                                            println(*r);
                                            println(*r2);
                                        }*/
                                    }
                                    /*
                                    else{
                                        cout << "cache hit" << endl;
                                    }*/
                                    delete check;
                                }
                                //ACPN case
                                /*else if((*region_ex_event_map)[r2]->size() > 1){
                                    bool symmetric_choice = true;
                                    for(auto event: *(*region_ex_event_map)[r2]){
                                        if((*region_ex_event_map)[r]->find(event) == (*region_ex_event_map)[r]->end()){
                                            symmetric_choice = false;
                                            break;
                                        }
                                    }
                                    if(!symmetric_choice){
                                        clause = new vector<int32_t>();
                                        clause->push_back(-(*reg_map)[r] - 1);
                                        clause->push_back(-(*reg_map)[r2] - 1);
                                        clauses_pre->push_back(clause);
                                    }
                                }*/
                                //SM case
                            }
                        }
                    }
                }
            }
        }

        if(decomposition && !no_bounds) {
            if (decomposition_debug)
                cout << "STEP 2c: safeness" << endl;

            //STEP 2c clause creation
            if(decomposition) {
                for (auto reg_pair: *intersecting_pairs) {
                    auto r1 = reg_pair->first;
                    auto r2 = reg_pair->second;
                    if (!empty_regions_intersection(r1, r2)) {
                        clause = new vector<int32_t>();
                        int offset = k_search_region_offset(m, k, num_FCPNs_try);
                        clause->push_back(-(*regions_alias_mapping)[r1] - offset);
                        clause->push_back(-(*regions_alias_mapping)[r2] - offset);
                        //cout << "clause: " << -(*regions_alias_mapping)[r1] - offset << " " << -(*regions_alias_mapping)[r2] - offset << endl;
                        clauses_pre->push_back(clause);
                    }
                }
            }
        }


        //STEP 3
        //ENCODING/BINDING symbolic regions and effective regions
        //if we have t FCPNs
        //symbolic sr0 -> r0 v r0'  => !sr0 v r0 v r0'
        if (decomposition_debug)
            cout << "STEP 3: REGION BINDINGS" << endl;
        //FIRST IMPLICATION
        for (int i = 0; i < k; ++i) {
            clause = new vector<int32_t>();
            //symbolic region
            clause->push_back(-m - 1 - i);
            for (int index = 0; index < num_FCPNs_try; ++index) {
                clause->push_back(i + k_search_region_offset(m, k, index + 1));
            }
            /*
            if (decomposition_debug)
                print_clause(clause);*/
            clauses->push_back(clause);
        }


        if (decomposition_debug)
            cout << "STEP 4" << endl;

        //STEP 4
        for (int i = 0; i < num_FCPNs_try; ++i) {
            for (auto rec: *region_ex_event_map) {
                auto reg = rec.first;
                for (auto ev: *rec.second) {
                    //reg_map contains values in [0, k-1]
                    int reg_offset = k_search_region_offset(m, k, i + 1);
                    int region_encoding = regions_alias_mapping->at(reg) + reg_offset;
                    int ev_offset = k_search_event_offset(m, k, i + 1);
                    auto ev_encoding = ev + ev_offset;
                    clause = new vector<int32_t>();
                    clause->push_back(-region_encoding);
                    clause->push_back(ev_encoding);
                    clauses->push_back(clause);

                    if (decomposition_debug)
                        print_clause(clause);
                }
            }
            for (auto rec: *region_ent_event_map) {
                auto reg = rec.first;
                for (auto ev: *rec.second) {
                    //reg_map contains values in [0, k-1]
                    int reg_offset = k_search_region_offset(m, k, i + 1);
                    int region_encoding = regions_alias_mapping->at(reg) + reg_offset;
                    int ev_offset = k_search_event_offset(m, k, i + 1);
                    auto ev_encoding = ev + ev_offset;
                    clause = new vector<int32_t>();
                    clause->push_back(-region_encoding);
                    clause->push_back(ev_encoding);
                    clauses->push_back(clause);
                    /*
                    if (decomposition_debug)
                        print_clause(clause);*/
                }
            }
        }

        if (decomposition_debug)
            cout << "STEP 5" << endl;
        //STEP 5
        for (auto rec: *pre_regions_map) {
            auto ev = rec.first;
            int ev_offset = k_search_event_offset(m, k, num_FCPNs_try);
            auto ev_encoding = ev + ev_offset;
            int reg_offset = k_search_region_offset(m, k, num_FCPNs_try);
            clause = new vector<int32_t>();
            clause->push_back(-ev_encoding);
            for (auto reg: *rec.second) {
                int region_encoding = regions_alias_mapping->at(reg) + reg_offset;
                clause->push_back(region_encoding);
            }
            clauses_pre->push_back(clause);
            if (decomposition_debug)
                print_clause(clause);
        }
        for (auto rec: *post_regions_map) {
            auto ev = rec.first;
            int ev_offset = k_search_event_offset(m, k, num_FCPNs_try);
            auto ev_encoding = ev + ev_offset;
            int reg_offset = k_search_region_offset(m, k, num_FCPNs_try);
            clause = new vector<int32_t>();
            clause->push_back(-ev_encoding);
            for (auto reg: *rec.second) {
                int region_encoding = regions_alias_mapping->at(reg) + reg_offset;
                clause->push_back(region_encoding);
            }
            clauses_pre->push_back(clause);
            if (decomposition_debug)
                print_clause(clause);
        }

        //STEP 6
        //ENCODING/BINDING symbolic events and effective events
        //if we have t FCPNs
        //symbolic se0-> e0 v e0'  => !se0 v e0 v e0'
        if (decomposition_debug)
            cout << "STEP 6: EVENT BINDINGS" << endl;
        //FIRST IMPLICATION
        for (int i = 0; i < m; ++i) {
            clause = new vector<int32_t>();
            clause->push_back(-i - 1);
            for (int index = 0; index < num_FCPNs_try; ++index) {
                int ev_offset = k_search_event_offset(m, k, index + 1);
                clause->push_back(i + ev_offset);
            }
            /*
            if (decomposition_debug)
                print_clause(clause);*/
            clauses->push_back(clause);
        }

        //STEP 9
        if(decomposition_debug && safe_components)
            cout << "STEP 9: FORBIDDEN FCPNS" << endl;
        //I have to add to clauses the encoding of the forbidden pns for each of k pns
        if(safe_components){
            for(auto pn: *forbidden_pns){
                if(decomposition_debug)
                    cout << "m = " << m << "; k = " << k << endl;
                for(int i=1; i <= num_FCPNs_try;++i){
                    clause = new vector<int32_t>();
                    int reg_offset = k_search_region_offset(m, k, i);
                    if(decomposition_debug)
                        cout << "reg offset: " << reg_offset << endl;
                    for(auto reg: *regions){
                        //cout << "region:";
                        //println(*reg);
                        int region_encoding = regions_alias_mapping->at(reg) + reg_offset;
                        if(pn.find(reg) != pn.end()){
                            //add literal with !p
                            clause->push_back(-region_encoding);
                            //cout << "encoding: " << -region_encoding << endl;
                        }
                        else{
                            //add literal with q
                            clause->push_back(region_encoding);
                            //cout << "encoding: " << region_encoding << endl;
                        }
                    }
                    if (decomposition_debug) {
                        cout << "forbidden PN encoding num " << i << endl;
                        print_clause(clause);
                    }
                    clauses->push_back(clause);
                }
            }
        }
        //STEP 10
        if(optimal){
            if(results_to_avoid != nullptr){
                for (const auto &res: *results_to_avoid) {
                    for (int i = 0; i < num_FCPNs_try; ++i) {
                        clause = new vector<int32_t>();
                        int reg_offset = k_search_region_offset(m, k, i);
                        for (auto reg: res) {
                            int region_encoding = regions_alias_mapping->at(reg) + reg_offset;
                            clause->push_back(-region_encoding);
                        }
                        clauses->push_back(clause);
                    }
                }
            }
        }

        //SAT computation
        PBConfig config = make_shared<PBConfigClass>();
        VectorClauseDatabase formula(config);
        //PB2CNF pb2cnf(config);
        AuxVarManager auxvars((m + k) * (num_FCPNs_try + 1) + 1);
        for (auto cl: *clauses_pre) {
            formula.addClause(*cl);
        }
        for (auto cl: *clauses) {
            formula.addClause(*cl);
        }
        /*
        for (auto cl: *splitting_constraint_clauses) {
            formula.addClause(*cl);
        }*/
        Minisat::Solver solver;

        bool sat;
        string dimacs_file;

        //iteration in the search of a correct assignment decreasing the total weight

        int num_clauses_formula = formula.getClauses().size();
        /*
        if (decomposition_debug) {
            cout << "formula:" << endl;
            formula.printFormula(cout);
        }*/

        dimacs_file = convert_to_dimacs(file, (k + m) * (num_FCPNs_try+1), num_clauses_formula,
                                        formula.getClauses());
        sat = check_sat_formula_from_dimacs(solver, dimacs_file);
        if (sat) {
            solution->clear();
            solution_found = true;
            if (decomposition_debug) {
                if(!fcptnet && !acpn){
                    cout << "SAT with " << num_FCPNs_try << " SMs!!!!" << endl;
                }
                else {
                    cout << "SAT with " << num_FCPNs_try << " FCPNs!!!!" << endl;
                }

                cout << "solver n vars: " << solver.nVars() << endl;
            }
            for (int i = 0; i < solver.nVars(); ++i) {
                if (solver.model[i] != l_Undef) {

                    if (decomposition_debug) {
                        fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                i + 1);
                    }
                    if (solver.model[i] == l_True) {
                        solution->push_back(i + 1);
                    } else {
                        solution->push_back(-i - 1);
                    }

                } else {
                    cerr << "undef" << endl;
                }

            }
            if(decomposition_debug)
                cout << endl;

            //STEP 8
            if(decomposition_debug)
                cout << "STEP 8" << endl;
            for (int i = 0; i < num_FCPNs_try; ++i) {
                auto temp_PN = new set<Region *>();
                //cout << "pn " << i << endl;
                for (int index = 0; index < k; ++index) {
                    int temp1;
                    if(i == 0) {
                        temp1 = m + k + m +index+1;
                    }
                    else{
                        temp1 = m + k + m + index + i*(m+k)+1;
                    }
                    //cout << temp1 << endl;
                    auto temp = solution->at(temp1-1);
                    //cout << temp << endl;
                    while (temp > m + k) {
                        temp = temp - (m + k);
                    }
                    temp = temp - m;
                    //cout << "temp: " << temp << endl;
                    if (temp > 0) {
                        temp_PN->insert(regions_alias_mapping_inverted->at(temp-1));
                        //cout << "decoded region" << endl;
                        //println(*regions_alias_mapping_inverted->at(temp-1));
                    }
                }
                fcpn_set->insert(temp_PN);
                //I don't have to check safeness if I have only one FCPN since in this case EC is enough for safeness
                if(safe_components && (num_FCPNs_try > 1)) {
                    safe = safeness_check(temp_PN, pre_regions_map, post_regions_map);
                    if (!safe) {
                        solution_found = false;
                        dont_increase_counter = true;
                        //cout << "not safe PN" << endl;
                        if(decomposition_debug){
                            cout << "not safe PN" << endl;
                            cout << endl;
                            println(temp_PN);
                            /*
                            cout << "BYE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                            exit(1);*/
                        }
                        if(forbidden_pns->find(*temp_PN) != forbidden_pns->end()){
                            cerr << "adding already forbidden PN" << endl;
                        }
                        forbidden_pns->insert(*temp_PN);

                        fcpn_set->clear();
                        delete temp_PN;
                        break;
                    }
                }
            }
        } else {
            if (decomposition_debug) {
                if (num_FCPNs_try == 1) {
                    cout << "UNSAT with 1 FCPN" << endl;
                } else {
                    cout << "UNSAT with " << num_FCPNs_try << " FCPNs" << endl;
                }
            }
            if(num_FCPNs_try == regions->size()){
                cout << "probably there is no solution" << endl;
                exit(0);
            }
        }
        if(solution_found){
            auto to_add = vector<set<Region *>*>();
            auto to_remove = vector<set<Region *>*>();
            for(auto temp_PN: *fcpn_set){
                auto new_temp_set = split_not_connected_regions(temp_PN, regions_connected_to_labels);
                if(new_temp_set->size() > 1){
                    suboptimal_result_found = true;
                    if(optimal){
                        solution_found = false;
                        results_to_avoid->push_back(*temp_PN);
                        dont_increase_counter = true;
                    }
                    if(solution_found) {
                        to_remove.push_back(temp_PN);
                        for (const auto &PN: *new_temp_set) {
                            auto tmp_PN = new set<Region *>();
                            for (auto reg: PN) {
                                tmp_PN->insert(reg);
                            }
                            to_add.push_back(tmp_PN);
                        }
                    }
                }
                delete new_temp_set;
            }
            if(solution_found) {
                for (auto PN: to_remove) {
                    fcpn_set->erase(PN);
                }
                for (auto PN: to_add) {
                    fcpn_set->insert(PN);
                }
            }
            else{
                fcpn_set->clear();
            }
        }
        if(!dont_increase_counter)
            num_FCPNs_try++;
    } while (!solution_found);

    delete cache;

    delete  results_to_avoid;

    if(!optimal && suboptimal_result_found){
        cout << "Suboptimal result found." << endl;
    }

    if(safe_components){
        cout << "Forbidden " << (forbidden_pns->size()-fcpn_set->size()) << " FCPNs." << endl;
    }

    if (decomposition_debug) {
        for (auto FCPN: *fcpn_set) {
            if(!decomposition)
                cout << "FCPN:" << endl;
            else
                cout << "SM:" << endl;
            println_simplified(FCPN, regions_alias_mapping);
        }
    }

    //this part is used only for SMs, happens when a set of regions represents a lot of SMs, therefore also using SAT
    // solver the result has redundant SMs
    if(decomposition)
        GreedyRemoval::minimize(fcpn_set, pprg, ER, pre_regions_map);

    map<set<Region *> *, map<int, set<Region *> *> *> *map_of_FCPN_pre_regions;
    map<set<Region *> *, map<int, set<Region *> *> *> *map_of_FCPN_post_regions;
    map<set<Region *> *, map<int, Region *> *> *map_of_SM_pre_regions;
    map<set<Region *> *, map<int, Region *> *> *map_of_SM_post_regions;
    if(fcptnet){
        map_of_FCPN_pre_regions = new map<set<Region *> *, map<int, set<Region *> *> *>();
        map_of_FCPN_post_regions = new map<set<Region *> *, map<int, set<Region *> *> *>();
    }
    else if(decomposition){
        map_of_SM_pre_regions = new map<set<Region *> *, map<int, Region *> *>();
        map_of_SM_post_regions = new map<set<Region *> *, map<int, Region *> *>();
    }

    if(fcptnet) {
        for (auto FCPN: *fcpn_set) {
            (*map_of_FCPN_pre_regions)[FCPN] = new map<int, set<Region *> *>();
            for (auto rec: *pprg->get_pre_regions()) {
                for (auto reg: *rec.second) {
                    if (FCPN->find(reg) != FCPN->end()) {
                        if ((*map_of_FCPN_pre_regions)[FCPN]->find(rec.first) ==
                            (*map_of_FCPN_pre_regions)[FCPN]->end()) {
                            (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first] = new set<Region *>();
                        }
                        (*(*map_of_FCPN_pre_regions)[FCPN])[rec.first]->insert(reg);
                    }
                }
            }
            (*map_of_FCPN_post_regions)[FCPN] = Pre_and_post_regions_generator::create_post_regions_for_FCPN(
                    (*map_of_FCPN_pre_regions)[FCPN]);
        }
    }
    else if(decomposition){
        for (auto SM: *fcpn_set) {
            (*map_of_SM_pre_regions)[SM] = new map<int, Region *>();
            for (auto rec: *pprg->get_pre_regions()) {
                for (auto reg: *rec.second) {
                    if (SM->find(reg) != SM->end()) {
                        (*(*map_of_SM_pre_regions)[SM])[rec.first]=reg;
                    }
                }
            }
            (*map_of_SM_post_regions)[SM] = Pre_and_post_regions_generator::create_post_regions_for_SM(
                    (*map_of_SM_pre_regions)[SM]);
        }
    }

    places_after_initial_decomp = 0;
    for(auto pn: *fcpn_set){
        places_after_initial_decomp += pn->size();
    }

    if(only_safeness_check){
        bool safe_set = true;
        for(auto pn: *fcpn_set){
            bool safe = safeness_check(pn, pre_regions_map, post_regions_map/*, region_ent_event_map*/);
            if(!safe){
                safe_set = false;
                break;
            }
        }
        if(safe_set){
            cout << "SAFENESS CHECK PASSED!!!" << endl;
        }
        else{
            cerr << "SAFENESS CHECK NOT PASSED!!!" << endl;
        }
    }

    cout << "Number of places before merge: " << places_after_initial_decomp << endl;

    if (!no_merge && !decomposition) {
        auto merge = new FCPN_Merge(fcpn_set, number_of_events, map_of_FCPN_pre_regions, map_of_FCPN_post_regions, file,
                                    aliases);
        delete merge;
    }
    else if(decomposition && !no_merge){
        auto merge = new Merge(fcpn_set,
                          number_of_events,
                          map_of_SM_pre_regions,
                          map_of_SM_post_regions,
                          file, regions_alias_mapping);
        delete merge;
    }

    if (composition) {
        if (decomposition) {
            for(auto SM: *fcpn_set){
                int count = 0;
                for(auto reg: *SM){
                    if(is_initial_region(reg))
                        count++;
                }
                if(count > 1)
                    cerr << "SM with multiple initial places" << endl;
            }
            SM_composition::compose(fcpn_set, map_of_SM_pre_regions, map_of_SM_post_regions, aliases, file);
        }
        else
            PN_composition::compose(fcpn_set, map_of_FCPN_pre_regions, map_of_FCPN_post_regions, aliases, file);
    }

    if(output){
        int pn_counter = 0;
        for (auto pn: *fcpn_set) {
            if(decomposition){
                print_sm_dot_file(regions_alias_mapping, map_of_SM_pre_regions->at(pn),
                                  map_of_SM_post_regions->at(pn), aliases, file, pn_counter);
            }
            else {
                print_pn_dot_file(regions_alias_mapping, map_of_FCPN_pre_regions->at(pn),
                                  map_of_FCPN_post_regions->at(pn), aliases, file, pn_counter);
            }
            pn_counter++;
        }
    }
    delete regions_alias_mapping;

    if (check_structure && !decomposition){
        check_EC_and_structure(ER, map_of_FCPN_pre_regions, map_of_FCPN_post_regions, pre_regions_map,
                                   region_ex_event_map, fcpn_set);
    }



    if(fcptnet) {
        maxAlphabet = getMaxAlphabet(map_of_FCPN_pre_regions, aliases);
        avgAlphabet = getAvgAlphabet(map_of_FCPN_pre_regions, aliases);
    }
    else if(decomposition){
        maxAlphabet = getMaxAlphabet(map_of_SM_pre_regions, aliases);
        avgAlphabet = getAvgAlphabet(map_of_SM_pre_regions, aliases);
    }


    for(auto rec: *region_ent_event_map){
        delete rec.second;
    }
    delete region_ent_event_map;

    for(auto rec: *region_ex_event_map){
        delete rec.second;
    }
    delete region_ex_event_map;

    if(!decomposition) {
        for (auto rec: *map_of_FCPN_post_regions) {
            for (auto rec1: *rec.second) {
                delete rec1.second;
            }
            delete rec.second;
        }
        delete map_of_FCPN_post_regions;

        for (auto rec: *map_of_FCPN_pre_regions) {
            for (auto rec1: *rec.second) {
                delete rec1.second;
            }
            delete rec.second;
        }
        delete map_of_FCPN_pre_regions;
    }
    else{
        for (auto rec: *map_of_SM_post_regions) {
            delete rec.second;
        }
        delete map_of_SM_post_regions;

        for (auto rec: *map_of_SM_pre_regions) {
            delete rec.second;
        }
        delete map_of_SM_pre_regions;
    }

    for(auto val:*clauses_pre){
        delete val;
    }
    delete clauses_pre;
    for(auto val:*clauses){
        delete val;
    }
    delete clauses;
    delete solution;
    delete regions_copy;
    delete regions_alias_mapping_inverted;
    for (auto rec: *regions_connected_to_labels) {
        delete rec.second;
    }
    delete regions_connected_to_labels;
    delete regions_vector;
    delete forbidden_pns;
    delete initial_regions;

    return fcpn_set;
}

int PN_decomposition::k_search_event_offset(int num_events, int num_regions, int num_FCPNs){
    return 1+num_FCPNs*(num_events+num_regions);
}
int PN_decomposition::k_search_region_offset(int num_events, int num_regions, int num_FCPNs){
    return num_events+1+num_FCPNs*(num_events+num_regions);
}


void PN_decomposition::check_EC_and_structure(map<int, ER> *ER,
                                              map<set<Region *> *, map<int, set<Region *> *> *>* map_of_FCPN_pre_regions,
                                              map<set<Region *> *, map<int, set<Region *> *> *>* map_of_FCPN_post_regions,
                                              map<int, set<Region *>*> *pre_regions_map,
                                              map<Region *, set<int> *>* region_ex_event_map,
                                              set<set<Region *> *>* pn_set) {
    cout << "EC check" << endl;

    auto new_pre_regions_map = new map<int, set<Region *>*>();
    for(auto rec: *pre_regions_map){
        auto event = rec.first;
        (*new_pre_regions_map)[event] = new set<Region *>();
        for(auto reg: *rec.second){
            bool found = false;
            for(auto pn_rec: *map_of_FCPN_pre_regions){
                if((*pn_rec.second).find(event) != (*pn_rec.second).end()) {
                    if ((*pn_rec.second)[event]->find(reg) != (*pn_rec.second)[event]->end()) {
                        found = true;
                        break;
                    }
                }
            }
            if(found){
                (*new_pre_regions_map)[event]->insert(reg);
            }
        }
    }

    bool ec = is_excitation_closed(new_pre_regions_map, ER);
    if(!ec){
        cerr << "EXCITATION CLOSURE NOT SATISFIED!!!" << endl;
        exit(1);
    }
    if(decomposition){
        cout << "Checking if the set of PNs contains really SMs..." << endl; //todo
    }
    else if(fcptnet){
        cout << "Checking if the set of PNs contains really FCPNs..." << endl;
    }
    else{
        cout << "Checking if the set of PNs contains really ACPNs..." << endl;
    }

    int pn_counter = 0;
    bool check_not_passed = false;
    if(decomposition){
        for (auto pn: *pn_set) {
            for (auto rec: *map_of_FCPN_pre_regions->at(pn)) {
                if(rec.second->size() > 1){
                    cerr << "PN " << pn_counter << " is not an SM!!!" << endl;
                    if (!ignore_correctness) {
                        exit(1);
                    }
                    check_not_passed = true;
                }
            }
            for (auto rec: *map_of_FCPN_post_regions->at(pn)) {
                if(rec.second->size() > 1){
                    cerr << "PN " << pn_counter << " is not an SM!!!" << endl;
                    if (!ignore_correctness) {
                        exit(1);
                    }
                    check_not_passed = true;
                }
            }
            pn_counter++;
        }
    }
    else {
        for (auto pn: *pn_set) {
            for (auto rec: *map_of_FCPN_pre_regions->at(pn)) {
                auto ev = rec.first;
                for (auto reg: *rec.second) {
                    if (region_ex_event_map->find(reg) == region_ex_event_map->end()) {
                        (*region_ex_event_map)[reg] = new set<int>();
                    }
                    (*region_ex_event_map)[reg]->insert(ev);
                }

                auto set_of_regions = rec.second;
                for (auto r: *set_of_regions) {
                    if ((*region_ex_event_map)[r]->size() > 1) {
                        for (auto r2: *set_of_regions) {
                            if (r != r2) {
                                if (fcptnet) {
                                    cerr << "PN " << pn_counter << " is not an FCPN!!!" << endl;
                                    if (!ignore_correctness) {
                                        exit(1);
                                    }
                                    check_not_passed = true;
                                } else if (acpn) {
                                    if ((*region_ex_event_map)[r2]->size() > 1) {
                                        if ((*region_ex_event_map)[r2]->size() !=
                                            (*region_ex_event_map)[r]->size()) {
                                            cerr << "PN " << pn_counter << " is not an ACPN!!!" << endl;
                                            if (!ignore_correctness) {
                                                exit(1);
                                            }
                                            check_not_passed = true;
                                        } else {
                                            for (auto event_in_middle: *(*region_ex_event_map)[r]) {
                                                if ((*region_ex_event_map)[r2]->find(event_in_middle) ==
                                                    (*region_ex_event_map)[r2]->end()) {
                                                    cerr << "PN " << pn_counter << " is not an ACPN!!!" << endl;
                                                    if (!ignore_correctness) {
                                                        exit(1);
                                                    }
                                                    check_not_passed = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            pn_counter++;
        }
    }
    if (!check_not_passed)
        cout << "Check passed." << endl;
}

int PN_decomposition::count_number_SMs(map<set<Region *> *, map<int, set<Region *> *> *>* map_of_FCPN_pre_regions,
                                       map<set<Region *> *, map<int, set<Region *> *> *>* map_of_FCPN_post_regions,
                                       set<set<Region *> *>* pn_set) {

    int num_SMs = 0;

    //int pn_counter = 0;
    for (auto pn: *pn_set) {
        bool check_not_passed = false;
        for (auto rec: *map_of_FCPN_pre_regions->at(pn)) {
            if(rec.second->size() > 1){
                //cerr << "PN " << pn_counter << " is not an SM!!!" << endl;
                check_not_passed = true;
                break;
            }
        }
        if(!check_not_passed) {
            for (auto rec: *map_of_FCPN_post_regions->at(pn)) {
                if (rec.second->size() > 1) {
                    //cerr << "PN " << pn_counter << " is not an SM!!!" << endl;
                    check_not_passed = true;
                    break;
                }
            }
        }
        if(!check_not_passed)
            num_SMs++;
        //pn_counter++;
    }
    return num_SMs;
}