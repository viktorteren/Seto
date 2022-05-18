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



    //----------------- counting different regions-------------------------
    if(decomposition_debug) {
        set<int> new_used_regions_tmp;
        for (auto tmp_SM: *SMs) {
            for (auto region: *tmp_SM) {
                new_used_regions_tmp.insert((*aliases_region_pointer_inverted)[region]);
            }
        }

        cout << "used regions: " << new_used_regions_tmp.size() << endl;
    }
}

void GreedyRemoval::minimize_sat(set<set<Region *>*> *FCPNs,
                                 set<set<Region *>*> *SMs,
                                 map<int, ER> *ER,
                                 map<int, set<Region *> *> *pre_regions,
                             const string& file){
    cout << "[EXACT SEARCH]=====================" << endl;
    /* Encodings:
     * FCPNs: [1, number of FCPNs and SMs = K]
     * places: [number of FCPNs and SMs + 1 = K + 1, number of places*number of FCPNs and SMs + number of FCPNs and SMs = N*K+K = K*(N+1)]
     */

    /*
     * Constraints:
     * 1) minimization of the number of FCPNs and SMs
     * 2) at least one occurrence of each region among different FCPNs and SMs but considering only the regions contained in FCPNs
     * 3) given an SM/FCPN all it's region have to take part of the result if the FCPN/SM takes part of the result and vice versa
     *      the clauses for A => (B and C) are converted into (!A v B) and (!A v C)
     *      the clauses for (B and C) => A are converted into (!B v A) and (!C v A)
     */

    auto clauses = new vector<vector<int32_t> *>();

    // create the map between FCPNs and integers from 1 to K
    map<SM *, int> FCPNs_map;
    map<int, SM *> FCPNs_map_inverted;
    int counter = 1;
    for (auto FCPN: *FCPNs) {
        FCPNs_map[FCPN] = counter;
        FCPNs_map_inverted[counter] = FCPN;
        counter++;
    }
    if(SMs != nullptr) {
        for (auto FCPN: *SMs) {
            FCPNs_map[FCPN] = counter;
            FCPNs_map_inverted[counter] = FCPN;
            counter++;
        }
    }

    // create the map between regions used in the FCPNs and integers from 1 to N -> one index for all different
    //      instances of the same region
    map<Region *, int> regions_map_for_sat;
    counter = 1;
    for (auto FCPN: *FCPNs) {
        for (auto reg: *FCPN) {
            if (regions_map_for_sat.find(reg) == regions_map_for_sat.end()) {
                regions_map_for_sat[reg] = counter;
                counter++;
                //println(*reg);
            }
        }
    }

    // STEP 2 using encoding for regions
    int K = FCPNs->size()+SMs->size();
    int N = counter-1;
    if(decomposition_debug) {
        cout << "N = " << N << endl;
        cout << "K = " << K << endl;
    }

    vector<int32_t> *clause;
    for (auto rec: regions_map_for_sat) {
        auto region = rec.first;
        auto region_counter = rec.second;
        clause = new vector<int32_t>();
        for (auto FCPN: *FCPNs) {
            int FCPN_counter = FCPNs_map[FCPN];
            if (FCPN->find(region) != FCPN->end()) {
                clause->push_back(K+N*(FCPN_counter - 1) + region_counter); //encoding of the place,
                                                                    // region counter is always greater of 0,
                                                                    // the same for SM_counter
                     //cout << "adding the encoding of region " << region_counter << " in SM " << SM_counter << endl;
            }
        }
        if(SMs != nullptr) {
            for (auto FCPN: *SMs) {
                int SM_counter = FCPNs_map[FCPN];
                if (FCPN->find(region) != FCPN->end()) {
                    clause->push_back(K + N * (SM_counter - 1) + region_counter); //encoding of the place,
                    // region counter is always greater of 0,
                    // the same for SM_counter
                    //cout << "adding the encoding of region " << region_counter << " in SM " << SM_counter << endl;
                }
            }
            clauses->push_back(clause); //all same places from different PNs
            //cout << "complete clause" << endl;
        }

    }

    //STEP 3
    for (auto FCPN: *FCPNs) {
        int SM_counter = FCPNs_map[FCPN];
        for (auto region: *FCPN) {
            auto region_counter = regions_map_for_sat.at(region);
            if (FCPN->find(region) != FCPN->end()) {
                clause = new vector<int32_t>();
                clause->push_back(K+N*(SM_counter - 1) + region_counter); //place
                clause->push_back(-SM_counter);
                clauses->push_back(clause); //(!A v B)
                //cout << "adding clause !SM v region: SM "  << SM_counter << " region " << region_counter << endl;
                clause = new vector<int32_t>();
                clause->push_back(-(K+N*(SM_counter - 1) + region_counter)); //place
                clause->push_back(SM_counter);
                clauses->push_back(clause); //(A v !B)
                //cout << "adding clause SM v !region: SM "  << SM_counter << " region " << region_counter << endl;
            }
        }
    }
    if(SMs != nullptr) {
        for (auto FCPN: *SMs) {
            int SM_counter = FCPNs_map[FCPN];
            for (auto region: *FCPN) {
                if (regions_map_for_sat.find(region) != regions_map_for_sat.end()) {
                    auto region_counter = regions_map_for_sat.at(region);
                    if (FCPN->find(region) != FCPN->end()) {
                        clause = new vector<int32_t>();
                        clause->push_back(K + N * (SM_counter - 1) + region_counter); //place
                        clause->push_back(-SM_counter);
                        clauses->push_back(clause); //(!A v B)
                        //cout << "adding clause !SM v region: SM "  << SM_counter << " region " << region_counter << endl;
                        clause = new vector<int32_t>();
                        clause->push_back(-(K + N * (SM_counter - 1) + region_counter)); //place
                        clause->push_back(SM_counter);
                        clauses->push_back(clause); //(A v !B)
                        //cout << "adding clause SM v !region: SM "  << SM_counter << " region " << region_counter << endl;
                    }
                }
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

    for(auto PN: *FCPNs){
        int encoded_value = FCPNs_map[PN];
        if (solver.model[encoded_value - 1] == l_False) {
            /*if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;*/
            to_remove.push_back(PN);
        }
    }

    for(auto PN: to_remove){
        FCPNs->erase(PN);
    }

    for(auto PN: *SMs){
        int encoded_value = FCPNs_map[PN];
        if (solver.model[encoded_value - 1] == l_True) {
            /*if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;*/
            FCPNs->insert(PN);
        }
    }


    //-----------------------------EC CHECK----------------------------------------------------------------

    set<int> new_used_regions_tmp;
    for (auto tmp_FCPN: *FCPNs) {
        for (auto region: *tmp_FCPN) {
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

    if(decomposition_debug) {
        //----------------- counting different regions-------------------------
        cout << "used regions: " << new_used_regions_tmp.size() << endl;
    }
}

void GreedyRemoval::minimize_sat_SM_exact(set<set<Region *>*> *SMs,
                                    map<int, ER> *ER,
                                    map<int, set<Region *> *> *pre_regions){
    auto SM_vector = new vector<set<Region*>*>();
    for(auto SM: *SMs){
        SM_vector->push_back(SM);
    }
    bool found = false;
    auto result = new vector<int>();

    int max = SMs->size()-1;

    for(int size=1;size<=SMs->size();++size){
        cout <<  "size:" << size << endl;
        auto temp_vec = new vector<int>();
        for(int i=0;i<size;++i){
            temp_vec->push_back(i);
        }
        cout << "temp vec: ";
        for(int j : *temp_vec){
            cout << j << " ";
        }
        cout << endl;
        do{
            found = check_EC(temp_vec,SM_vector,pre_regions,ER);
            if(!found){
                if(exists_next(temp_vec,max)){
                    temp_vec = next_set(temp_vec,max);
                    cout << "temp vec: ";
                    for(int j : *temp_vec){
                        cout << j << " ";
                    }
                    cout << endl;
                }
                else break;
            }
            else{
                cout << "FOUND"<< endl;
                result = temp_vec;
            }
        }while(!found);
        if(found) break;
    }

    cout << "result vec: ";
    for(int j : *result){
        cout << j << " ";
    }
    cout << endl;
}

bool GreedyRemoval::check_EC(vector<int>* vec,
                             vector<set<Region*>*> *SM_vector,
                             map<int, set<Region *> *> *pre_regions,
                             map<int, ER> *ER ){
    auto SMs = new set<set<Region*>*>();
    for(auto index: *vec){
        SMs->insert(SM_vector->at(index));
    }
    set<int> new_used_regions_tmp;
    for (auto tmp_FCPN: *SMs) {
        for (auto region: *tmp_FCPN) {
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

    return is_excitation_closed(new_used_regions_map_tmp, ER);
}

bool GreedyRemoval::exists_next(vector<int> *vec,int max){
    for(int i=vec->size()-1;i>=0;i--){
        if(vec->at(i) < max && i == vec->size()-1)
            return true;
        if(vec->at(i) < max && i != vec->size()-1)
            if(vec->at(i+1) > vec->at(i)+1)
                return true;
    }
    return false;
}

vector<int>* GreedyRemoval::next_set(vector<int>* vec, int max){
    for(int i=vec->size()-1;i>=0;i--){
        if(vec->at(i) < max && i == vec->size()-1){
            auto temp = new vector<int>();
            for(int k=0;k<vec->size();k++){
                if(k != i){
                    temp->push_back(vec->at(k));
                }
                else{
                    temp->push_back(vec->at(k)+1);
                }
            }
            if(temp->size() != vec->size())
                cout << "different size"<< endl;
            return temp;
        }
        if(vec->at(i) < max && i != vec->size()-1)
            if(vec->at(i+1) > vec->at(i)+1){
                auto temp = new vector<int>();
                for(int k=0;k<vec->size();k++){
                    if(k != i){
                        temp->push_back(vec->at(k));
                    }
                    else{
                        temp->push_back(vec->at(k)+1);
                    }
                }
                if(temp->size() != vec->size())
                    cout << "different size"<< endl;
                return temp;
            }
    }
}

void GreedyRemoval::minimize_sat_SM(set<set<Region *>*> *SMs,
                                 map<int, ER> *ER,
                                 map<int, set<Region *> *> *pre_regions,
                                 const string& file){
    cout << "[EXACT SEARCH]=====================" << endl;
    /* Encodings:
     * FCPNs: [1, number of FCPNs and SMs = K]
     * places: [number of FCPNs and SMs + 1 = K + 1, number of places*number of FCPNs and SMs + number of FCPNs and SMs = N*K+K = K*(N+1)]
     */

    /*
     * Constraints:
     * 1) minimization of the number of FCPNs and SMs
     * 2) at least one occurrence of each region among different FCPNs and SMs but considering only the regions contained in FCPNs
     * 3) given an SM/FCPN all it's region have to take part of the result if the FCPN/SM takes part of the result and vice versa
     *      the clauses for A => (B and C) are converted into (!A v B) and (!A v C)
     *      the clauses for (B and C) => A are converted into (!B v A) and (!C v A)
     */



    auto clauses = new vector<vector<int32_t> *>();

    // create the map between FCPNs and integers from 1 to K
    map<SM *, int> SMs_map;
    map<int, SM *> SMs_map_inverted;
    int counter = 1;
    if(SMs != nullptr) {
        for (auto SM: *SMs) {
            SMs_map[SM] = counter;
            SMs_map_inverted[counter] = SM;
            counter++;
        }
    }

    // create the map between regions used in the FCPNs and integers from 1 to N -> one index for all different
    //      instances of the same region
    map<Region *, int> regions_map_for_sat;
    counter = 1;
    for (auto SM: *SMs) {
        for (auto reg: *SM) {
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
    if(decomposition_debug) {
        cout << "N = " << N << endl;
        cout << "K = " << K << endl;
    }

    vector<int32_t> *clause;
    for (auto rec: regions_map_for_sat) {
        auto region = rec.first;
        auto region_counter = rec.second;
        clause = new vector<int32_t>();
        for (auto SM: *SMs) {
            int SM_counter = SMs_map[SM];
            if (SM->find(region) != SM->end()) {
                clause->push_back(K + N * (SM_counter - 1) + region_counter); //encoding of the place,
                // region counter is always greater of 0,
                // the same for SM_counter
                //cout << "adding the encoding of region " << region_counter << " in SM " << SM_counter << endl;
            }
        }
        clauses->push_back(clause); //all same places from different PNs
        //cout << "complete clause" << endl;
    }

    //STEP 3
    for (auto SM: *SMs) {
        int SM_counter = SMs_map[SM];
        for (auto region: *SM) {
            if (regions_map_for_sat.find(region) != regions_map_for_sat.end()) {
                auto region_counter = regions_map_for_sat.at(region);
                if (SM->find(region) != SM->end()) {
                    clause = new vector<int32_t>();
                    clause->push_back(K + N * (SM_counter - 1) + region_counter); //place
                    clause->push_back(-SM_counter);
                    clauses->push_back(clause); //(!A v B)
                    //cout << "adding clause !SM v region: SM "  << SM_counter << " region " << region_counter << endl;
                    clause = new vector<int32_t>();
                    clause->push_back(-(K + N * (SM_counter - 1) + region_counter)); //place
                    clause->push_back(SM_counter);
                    clauses->push_back(clause); //(A v !B)
                    //cout << "adding clause SM v !region: SM "  << SM_counter << " region " << region_counter << endl;
                }
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
                cout << "SAT with value " << current_value << ": representing the number of SMs"
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

                cout << "UNSAT with value " << current_value << ": representing the number of SMs" << endl;/*
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
        int encoded_value = SMs_map[PN];
        if (solver.model[encoded_value - 1] == l_False) {
            /*if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;*/
            to_remove.push_back(PN);
        }
    }

    for(auto PN: to_remove){
        SMs->erase(PN);
    }

    for(auto PN: *SMs){
        int encoded_value = SMs_map[PN];
        if (solver.model[encoded_value - 1] == l_True) {
            /*if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;*/
            SMs->insert(PN);
        }
    }


    //-----------------------------EC CHECK----------------------------------------------------------------

    set<int> new_used_regions_tmp;
    for (auto tmp_FCPN: *SMs) {
        for (auto region: *tmp_FCPN) {
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

    if(decomposition_debug) {
        //----------------- counting different regions-------------------------
        cout << "used regions: " << new_used_regions_tmp.size() << endl;
    }
}
