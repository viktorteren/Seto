/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include <include/Pre_and_post_regions_generator.h>
#include "../include/GreedyRemoval.h"

using namespace Utilities;
using namespace Minisat;
using namespace PBLib;

void GreedyRemoval::minimize(set<set<Region *>*> *SMs,
                             Pre_and_post_regions_generator *pprg,
                             map<int, ER> *new_ER,
                             map<int, set<Region *> *> *pre_regions){
    vector<set<Region *> *> SMs_to_remove;
    bool excitation_closure;
    auto SMs_descending = new set<set<Region *>*>();
    auto SMs_ascending = new set<set<Region *>*>();
    for(auto SM: *SMs){
        SMs_descending->insert(SM);
        SMs_ascending->insert(SM);
    }

    for (auto SM: *SMs_ascending) {
        /*if (decomposition_debug) {
            cout << "check if can remove the SM: " << endl;
            println(*SM);
        }*/
        auto tmp_SMs = new set<set<Region *> *>();
        //tmp_SM prende tutto tranne SM
        for (auto set: *SMs_ascending) {
            if (set != SM)
                tmp_SMs->insert(set);
        }

        auto used_regions_map = get_map_of_used_regions(tmp_SMs, pprg->get_pre_regions());

        excitation_closure = is_excitation_closed(used_regions_map, new_ER);
        for (auto rec: *used_regions_map) {
            delete rec.second;
        }
        delete used_regions_map;
        if (excitation_closure) {
            SMs_to_remove.push_back(SM);
            /*if (decomposition_debug) {
                cout << "ok without SM:" << endl;
                println(*SM);
            }*/
        }
        delete tmp_SMs;
    }

    //int num_SMs = SMs_ascending->size();
    //int num_candidates = SMs_to_remove.size();

    vector<set<Region *>*> SMs_to_remove_descending;
    SMs_to_remove_descending.reserve(SMs_to_remove.size());
    for (auto SM: SMs_to_remove) {
        SMs_to_remove_descending.push_back(SM);
    }

    //use of the heuristics which removes biggest SMs first -> local minimality not guaranteed

    //sorting of the SMs in increasing size order
    sort(SMs_to_remove.begin(), SMs_to_remove.end(), [](set<Region *> *lhs, set<Region *> *rhs) {
        return lhs->size() < rhs->size();
    });
    while (!SMs_to_remove.empty()) {
        auto tmp_SMs = new set<set<Region *> *>();
        auto SM = SMs_to_remove.at(0); //always take the first element: the biggest SM
        //tmp_SM takes all except SM
        for (auto set: *SMs_ascending) {
            if (set != SM)
                tmp_SMs->insert(set);
        }

        set<int> new_used_regions_tmp;
        for (auto tmp_SM: *tmp_SMs) {
            for (auto region: *tmp_SM) {
                new_used_regions_tmp.insert((*aliases_region_pointer_inverted)[region]);
            }
        }
        auto new_used_regions_map_tmp = new map < int, set<Region *>
        * > ();
        for (auto rec: *pre_regions) {
            (*new_used_regions_map_tmp)[rec.first] = new set < Region * > ();
        }

        for (auto reg: new_used_regions_tmp) {
            for (auto rec: *pre_regions) {
                if (rec.second->find((*aliases_region_pointer)[reg]) != rec.second->end()) {
                    (*new_used_regions_map_tmp)[rec.first]->insert((*aliases_region_pointer)[reg]);
                }
            }
        }

        excitation_closure = is_excitation_closed(new_used_regions_map_tmp, new_ER);
        for (auto rec: *new_used_regions_map_tmp) {
            delete rec.second;
        }
        delete new_used_regions_map_tmp;
        if (excitation_closure) {
            if (decomposition_debug) {
                cout << "removed PN/SM" << endl;
                println(*SM);
            }
            //removal of the SM
            //delete *SMs->find(SM); //removes the regions of the SM
            SMs_ascending->erase(SM); //removes the pointer for the regions of the SM
        }
        SMs_to_remove.erase(SMs_to_remove.begin());
        delete tmp_SMs;
    }

    /*
    if (((unsigned int) num_SMs - num_candidates) == SMs->size()) {
        cout << "All candidate PNs/SMs has been removed" << endl;
    }*/

    //sorting of the SMs in decreasing size order
    sort(SMs_to_remove_descending.begin(), SMs_to_remove_descending.end(), [](set<Region *> *lhs, set<Region *> *rhs) {
        return lhs->size() > rhs->size();
    });
    while (!SMs_to_remove_descending.empty()) {
        auto tmp_SMs = new set<set<Region *> *>();
        auto SM = SMs_to_remove_descending.at(0); //always take the first element: the biggest SM
        //tmp_SM takes all except SM
        for (auto set: *SMs_descending) {
            if (set != SM)
                tmp_SMs->insert(set);
        }

        set<int> new_used_regions_tmp;
        for (auto tmp_SM: *tmp_SMs) {
            for (auto region: *tmp_SM) {
                new_used_regions_tmp.insert((*aliases_region_pointer_inverted)[region]);
            }
        }
        auto new_used_regions_map_tmp = new map < int, set<Region *>
        * > ();
        for (auto rec: *pre_regions) {
            (*new_used_regions_map_tmp)[rec.first] = new set < Region * > ();
        }

        for (auto reg: new_used_regions_tmp) {
            for (auto rec: *pre_regions) {
                if (rec.second->find((*aliases_region_pointer)[reg]) != rec.second->end()) {
                    (*new_used_regions_map_tmp)[rec.first]->insert((*aliases_region_pointer)[reg]);
                }
            }
        }

        excitation_closure = is_excitation_closed(new_used_regions_map_tmp, new_ER);
        for (auto rec: *new_used_regions_map_tmp) {
            delete rec.second;
        }
        delete new_used_regions_map_tmp;
        if (excitation_closure) {
            if (decomposition_debug) {
                cout << "removed PN/SM" << endl;
                println(*SM);
            }
            //removal of the SM
            //delete *SMs_descending->find(SM); //removes the regions of the SM
            SMs_descending->erase(SM); //removes the pointer for the regions of the SM
        }
        SMs_to_remove_descending.erase(SMs_to_remove_descending.begin());
        delete tmp_SMs;
    }

    if(SMs_ascending->size() > SMs_descending->size()){
        for(set<Region *> *SM: *SMs){
            if(SMs_descending->find(SM) == SMs_descending->end()){
                delete *SMs->find(SM);
                SMs->erase(SM);
            }
        }
    }
    else{
        for(auto SM: *SMs){
            if(SMs_ascending->find(SM) == SMs_ascending->end()){
                delete *SMs->find(SM);
                SMs->erase(SM);
            }
        }
    }
    delete SMs_ascending;
    delete SMs_descending;

    /*
    if (((unsigned int) num_SMs - num_candidates) == SMs->size()) {
        cout << "All candidate PNs/SMs has been removed" << endl;
    }*/
}

void GreedyRemoval::minimize_sat(set<set<Region *>*> *SMs,
                                 map<int, ER> *ER,
                                 map<int, set<Region *> *> *pre_regions,
                             const string& file){
    cout << "[EXACT SEARCH]=====================" << endl;
    /* Encodings:
     * SMs: [1, number of SMs = K]
     * places: [number of SMs + 1 = K + 1, number of places*number of SMs + number of SMs = N*K+K = K*(N+1)]
     */

    /*
     * Constraints:
     * 1) minimization of the number of SMs
     * 2) at least one occurrence of each region among different PNs
     * 3) given an SM all it's region have to take part of the result if the SM takes part of the result and vice versa
     *      the clauses for A => (B and C) are converted into (!A v B) and (!A v C)
     *      the clauses for (B and C) => A are converted into (!B v A) and (!C v A)
     */

    auto clauses = new vector<vector<int32_t> *>();

    // create the map between SMs and integers from 1 to K
    map<SM *, int> FCPNs_map;
    map<int, SM *> FCPNs_map_inverted;
    int counter = 1;
    for (auto FCPN: *SMs) {
        FCPNs_map[FCPN] = counter;
        FCPNs_map_inverted[counter] = FCPN;
        counter++;
    }

    // create the map between regions used in the FCPNs and integers from 1 to N -> one index for all different
    //      instances of the same region
    map<Region *, int> regions_map_for_sat;
    counter = 1;
    if(decomposition_debug)
        cout << "all different regions" << endl;
    for (auto FCPN: *SMs) {
        for (auto reg: *FCPN) {
            if (regions_map_for_sat.find(reg) == regions_map_for_sat.end()) {
                regions_map_for_sat[reg] = counter;
                counter++;
                //println(*reg);
            }
        }
    }

    // STEP 2 using encoding for regions
    int K = SMs->size();
    int N = counter-1;
    if(decomposition_debug)
        cout << "N = " << N << endl;
    for (auto vec: *clauses) {
        delete vec;
    }
    clauses->clear();
    vector<int32_t> *clause;
    for (auto rec: regions_map_for_sat) {
        auto region = rec.first;
        auto region_counter = rec.second;
        clause = new vector<int32_t>();
        for (auto FCPN: *SMs) {
            int SM_counter = FCPNs_map[FCPN];
            if (FCPN->find(region) != FCPN->end()) {
                clause->push_back(K+N*(SM_counter - 1) + region_counter+1); //encoding of the place,
                                                                    // region counter is always greater of 0,
                                                                    // the same for SM_counter
            }
        }
        clauses->push_back(clause); //all same places from different PNs
    }

    //STEP 3
    for (auto rec: regions_map_for_sat) {
        auto region = rec.first;
        auto region_counter = rec.second;
        for (auto FCPN: *SMs) {
            int SM_counter = FCPNs_map[FCPN];
            if (FCPN->find(region) != FCPN->end()) {
                clause = new vector<int32_t>();
                clause->push_back(K+N*(SM_counter - 1) + region_counter+1); //place
                clause->push_back(-SM_counter);
                clauses->push_back(clause); //(!A v B)
                clause = new vector<int32_t>();
                clause->push_back(-(K+N*(SM_counter - 1) + region_counter+1)); //place
                clause->push_back(SM_counter);
                clauses->push_back(clause); //(A v !B)
            }
        }
    }

    //STEP 1
    vector<WeightedLit> literals_from_SMs = {};
    literals_from_SMs.reserve(K); //improves the speed
    for (int i = 1; i <= K; i++) {
        literals_from_SMs.emplace_back(i, 1);
    }

    PBConfig config = make_shared<PBConfigClass>();
    VectorClauseDatabase formula(config);
    PB2CNF pb2cnf(config);
    AuxVarManager auxvars(K*(N+1)+2);
    for (auto cl: *clauses) {
        formula.addClause(*cl);
    }
    Minisat::Solver solver;
    bool sat;
    string dimacs_file;

    int current_value = K;
    auto last_solution = new set<int>();
    IncPBConstraint constraint(literals_from_SMs, LEQ,
                               current_value);
    pb2cnf.encodeIncInital(constraint, formula, auxvars);
    //iteration in the search of a correct assignment decreasing the total weight
    do {
        int num_clauses_formula = formula.getClauses().size();
        //cout << "formula 1" << endl;
        //formula.printFormula(cout);
        dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                        formula.getClauses());
        sat = check_sat_formula_from_dimacs(solver, dimacs_file);
        if (sat) {
            if (decomposition_debug) {
                cout << "SAT with value " << current_value << ": representing the number of PNs"
                     << endl;
                cout << "Model: ";
            }
            last_solution->clear();
            for (int i = 0; i < solver.nVars(); ++i) {
                if (solver.model[i] != l_Undef) {
                    /*
                    if (decomposition_debug) {
                        fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                i + 1);
                    }*/
                    if (i < K) {
                        if (solver.model[i] == l_True) {
                            last_solution->insert(i + 1);
                        } else {
                            last_solution->insert(-i - 1);
                        }
                    }
                }
            }
            current_value--;
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
            break;
        }
        constraint.encodeNewLeq(current_value,formula,auxvars);
    } while (true);

    vector<set<Region *>*> to_remove;

    for(auto PN: *SMs){
        int encoded_value = FCPNs_map[PN];
        if (solver.model[encoded_value - 1] == l_False) {
            /*if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;*/
            to_remove.push_back(PN);
        }
    }

    for(auto PN: to_remove){
        SMs->erase(PN);
    }

    //-----------------------------EC CHECK----------------------------------------------------------------

    set<int> new_used_regions_tmp;
    for (auto tmp_SM: *SMs) {
        for (auto region: *tmp_SM) {
            new_used_regions_tmp.insert((*aliases_region_pointer_inverted)[region]);
        }
    }
    auto new_used_regions_map_tmp = new map < int, set<Region *> * > ();
    for (auto rec: *pre_regions) {
        (*new_used_regions_map_tmp)[rec.first] = new set < Region * > ();
    }

    for (auto reg: new_used_regions_tmp) {
        for (auto rec: *pre_regions) {
            if (rec.second->find((*aliases_region_pointer)[reg]) != rec.second->end()) {
                (*new_used_regions_map_tmp)[rec.first]->insert((*aliases_region_pointer)[reg]);
            }
        }
    }

    bool excitation_closure = is_excitation_closed(new_used_regions_map_tmp, ER);

    if(excitation_closure){
        cout << "EC satisfied" << endl;
    }
    else{
        cerr << "EC not satisfied" << endl;
    }
}
