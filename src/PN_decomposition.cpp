/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include <include/GreedyRemoval.h>
#include "../include/PN_decomposition.h"
#include "../include/FCPN_Merge.h"

using namespace PBLib;
using namespace Minisat;
using namespace Utilities;

set<set<Region *> *> *PN_decomposition::search(int number_of_events,
                                                 const set<Region *>& regions,
                                                 const string& file,
                                                 Pre_and_post_regions_generator *pprg,
                                                 map<int, ER> *ER, map<int, int> *aliases){
    /* Possible algorithm for the creation of one FCPN with SAT:
     * ALGORITHM STEPS:
     * do
     *      1) (REMOVED) at least one region which covers each state: for each covered state by r1, r2, r3 create a clause (r1 v r2 v r3)
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
     *      5) maximization function: number of new regions used in the result -> max covering
     *      6) OPTIONAL: solve the SAT problem decreasing the value of the region sum -> starting value is the sum of all regions
     *      7) decode result
     * 8) while !EC: previous results clauses are added as results to avoid in future
     * 9) greedy FCPN removal
     */

    cout << "=========[FCPN/ACPN DECOMPOSITION MODULE]===============" << endl;
    auto regions_copy = new set<Region *>();
    for(auto reg: regions){
        regions_copy->insert(reg);
    }
    auto pre_regions_map = pprg->get_pre_regions();
    auto post_regions_map = pprg->get_post_regions();
    auto minimal_regions = new set<Region *>();
    for(auto reg: regions){
        minimal_regions->insert(reg);
    }
    auto regions_connected_to_labels = merge_2_maps(pre_regions_map,
                                                    post_regions_map);
    auto clauses = new vector<vector<int32_t> *>();
    auto splitting_constraint_clauses = new vector<vector<int32_t> *>();
    auto fcpn_set = new set<set<Region *> *>();
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
    for (auto reg: *regions_copy) {
        (*reg_map)[reg] = temp;
        regions_vector->push_back(reg);
        temp++;
    }

    for (auto reg: *minimal_regions) {
        not_used_regions->insert(reg);
    }

    vector<int32_t> *clause;

    //encoding: [1, k] regions range: k regions
    //encoding: [k+1, k+m+1] events range: m events
    int m = number_of_events;
    int k = regions_copy->size();
    bool excitation_closure;
    bool splitting_constraints_added = false;

    do {
        for (auto cl: *clauses) {
            delete cl;
        }
        clauses->clear();

        if (!splitting_constraints_added) {
            if (!splitting_constraint_clauses->empty()) {
                splitting_constraint_clauses->clear();
                cout << "removing splitting constraints" << endl;
            }
        }

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
                                clauses->push_back(clause);
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
                                    clauses->push_back(clause);
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
                clauses->push_back(clause);
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
                clauses->push_back(clause);
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
            clauses->push_back(clause);
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
            clauses->push_back(clause);
        }

        //STEP 5
        //cout << "STEP 5" << endl;
        vector<WeightedLit> literals_from_regions = {};
        literals_from_regions.reserve(k); //improves the speed
        for (int i = 0; i < k; i++) {
            if (not_used_regions->find((*regions_vector)[i]) != not_used_regions->end()) {
                literals_from_regions.emplace_back(1 + i, 1);
            } else {
                literals_from_regions.emplace_back(1 + i, 0);
            }
        }

        int current_value = 1;
        int min = 0;
        int max = k;

        PBConfig config = make_shared<PBConfigClass>();
        VectorClauseDatabase formula(config);
        PB2CNF pb2cnf(config);
        AuxVarManager auxvars(k + m + 2);
        for (auto cl: *clauses) {
            formula.addClause(*cl);
        }
        for (auto cl: *splitting_constraint_clauses) {
            formula.addClause(*cl);
        }
        Minisat::Solver solver;

        bool sat;
        string dimacs_file;
        bool exists_solution = false;

        auto last_solution = new set<int>();
        //iteration in the search of a correct assignment decreasing the total weight
        do {
            IncPBConstraint constraint(literals_from_regions, GEQ,
                                       current_value); //the sum have to be greater or equal to current_value
            pb2cnf.encodeIncInital(constraint, formula, auxvars);
            int num_clauses_formula = formula.getClauses().size();
            //cout << "formula 1" << endl;
            //formula.printFormula(cout);
            dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                            formula.getClauses(), results_to_avoid);
            sat = check_sat_formula_from_dimacs(solver, dimacs_file);
            if (sat) {
                exists_solution = true;
                /*
                if (decomposition_debug) {
                    cout << "SAT with value " << current_value << ": representing the number of new covered regions"
                         << endl;
                    cout << "Model: ";
                }*/
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
                /*
                if (decomposition_debug)
                    cout << endl;
                */
                min = current_value;
            } else {
                if (decomposition_debug) {
                    //cout << "----------" << endl;
                    cout << "UNSAT with value " << current_value << endl;
                    if (exists_solution) {
                        cout << "Model: ";
                        for (int i = 0; i < solver.nVars(); ++i) {
                            if (solver.model[i] != l_Undef) {
                                fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                        i + 1);
                            }
                        }
                        cout << endl;
                    }
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
                int min_size = (*new_temp_set)[0].size();
                int pos = 0;
                for (int i = 1; i < new_temp_set->size(); i++) {
                    if ((*new_temp_set)[i].size() < min_size) {
                        min_size = (*new_temp_set)[i].size();
                        pos = i;
                    }
                }
                clause = new vector<int32_t>();
                for (auto reg: (*new_temp_set)[pos]) {
                    //cout << "added a new constraint" << endl;
                    clause->push_back(-1 + reg_map->at(reg));
                }
                splitting_constraint_clauses->push_back(clause);
                delete temp_PN;
                splitting_constraints_added = true;
            } else {
                for (auto val: *last_solution) {
                    if (val > 0) {
                        /*if(decomposition_debug) {
                            cout << val << ": ";
                            println(*regions_vector->at(val - 1));
                        }*/
                        if (val <= k) {
                            if(temp_PN->find(regions_vector->at(val - 1)) != temp_PN->end())
                                if (not_used_regions->find(regions_vector->at(val - 1)) != not_used_regions->end())
                                    not_used_regions->erase(regions_vector->at(val - 1));
                        }
                    }
                }
                fcpn_set->insert(temp_PN);
                if(decomposition_debug)
                    cout << "adding new fcpn to solutions" << endl;
            }
            delete new_temp_set;
            results_to_avoid->push_back(*last_solution);

            if (!splitting_constraints_added) {
                auto used_regions_map = get_map_of_used_regions(fcpn_set, pprg->get_pre_regions());
                excitation_closure = is_excitation_closed(used_regions_map, ER);
                for (auto rec: *used_regions_map) {
                    delete rec.second;
                }
                delete used_regions_map;
                if (excitation_closure) {
                    cout << "ec ok" << endl;
                }
            }
            formula.clearDatabase();
        } else {
            cout << "no solution found" << endl;
            exit(0);
        }
        delete last_solution;
    } while (!excitation_closure);

    for(auto cl: *splitting_constraint_clauses){
        delete cl;
    }
    delete splitting_constraint_clauses;

    if(decomposition_debug) {
        for (auto pn: *fcpn_set) {
            cout << "PN:" << endl;
            println(pn);
        }
    }

    cout << "PNs before greedy: " << fcpn_set->size() << endl;

    places_after_initial_decomp = 0;
    for(auto pn: *fcpn_set){
        places_after_initial_decomp += pn->size();
    }

    cout << "Number of places before greedy: " << places_after_initial_decomp << endl;

    //STEP 9
    if(fcpn_set->size() > 1)
        GreedyRemoval::minimize(fcpn_set, pprg, ER, pre_regions_map);

    cout << (fcptnet ? "FCPN" : "ACPN") << " set size: " << fcpn_set->size() << endl;

    places_after_greedy = 0;
    for(auto pn: *fcpn_set){
        places_after_greedy += pn->size();
    }

    cout << "Number of places after greedy: " << places_after_greedy << endl;

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

    if(fcpn_set->size() == 1){
        no_merge = true;
    }

    if(!no_merge) {
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
            //todo: fare dei controlli, mi viene il nome con FCPN quando dovrebbe essere PN
            print_pn_dot_file(regions_mapping, map_of_FCPN_pre_regions->at(pn), map_of_FCPN_post_regions->at(pn),
                              aliases,
                              file, pn_counter);
            pn_counter++;
        }
        delete regions_mapping;
    }
    if(composition)
        PN_composition::compose(fcpn_set, map_of_FCPN_pre_regions, map_of_FCPN_post_regions, aliases, file);


    cout << "Checking if the set of PNs are really " << (fcptnet ? "FCPNs..." : "ACPNs...") << endl;

    for (auto rec_map: *region_ex_event_map) {
        delete rec_map.second;
    }
    //completely deleting the pointer to the map we delete also empty records
    delete region_ex_event_map;
    region_ex_event_map = new map<Region *, set<int> *>();

    int pn_counter = 0;
    bool check_not_passed = false;
    for (auto pn: *fcpn_set) {
        for (auto rec: *map_of_FCPN_pre_regions->at(pn)) {
            /*for (auto rec_map: *region_ex_event_map) {
                delete rec_map.second;
            }
            //completely deleting the pointer to the map we delete also empty records
            delete region_ex_event_map;
            region_ex_event_map = new map<Region *, set<int> *>();*/
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
                            if(fcptnet) {
                                cerr << "PN " << pn_counter << " is not an FCPN!!!" << endl;
                                if(!ignore_correctness) {
                                    exit(1);
                                }
                                check_not_passed = true;
                            }
                            else if(acpn){
                                if ((*region_ex_event_map)[r2]->size() > 1) {
                                    if((*region_ex_event_map)[r2]->size() != (*region_ex_event_map)[r]->size()) {
                                        cerr << "PN " << pn_counter << " is not an ACPN!!!" << endl;
                                        if(!ignore_correctness) {
                                            exit(1);
                                        }
                                        check_not_passed = true;
                                    }
                                    else{
                                        for(auto event_in_middle: *(*region_ex_event_map)[r]){
                                            if((*region_ex_event_map)[r2]->find(event_in_middle) == (*region_ex_event_map)[r2]->end()){
                                                cerr << "PN " << pn_counter << " is not an ACPN!!!" << endl;
                                                if(!ignore_correctness) {
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
    if(!check_not_passed)
        cout << "Check passed." << endl;

    auto maxAlphabet = getMaxAlphabet(map_of_FCPN_pre_regions, aliases);
    auto avgAlphabet = getAvgAlphabet(map_of_FCPN_pre_regions, aliases);

    cout << "MAX alphabet: " << maxAlphabet << endl;
    cout << "AVG alphabet: " << avgAlphabet << endl;

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

    delete minimal_regions;

    delete regions_copy;

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

    return fcpn_set;
}