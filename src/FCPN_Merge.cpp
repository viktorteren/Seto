/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/FCPN_Merge.h"

using namespace PBLib;
using namespace Minisat;
using namespace Utilities;

FCPN_Merge::FCPN_Merge(set<SM *> *FCPNs,
             int number_of_events,
             map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_pre_regions,
             map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_post_regions,
             const string& file,
             map<int, int> *aliases) {
    cout << "MERGE MODULE" << endl;

    //STEPS OF THE ALGORITHM
    // 1. create the map between SMs and integers from 1 to K
    // 2. create the map between regions used in the FCPNs and integers from 1 to N -> one index for all different
    // instances of the same region
    // 3. create clauses to satisfy at least one instance of each region: (r1i -v -v - r1k) at least one instance of r1
    // have to be true
    // (OPTIONAL: don't know if needed) 3b. create clauses to satisfy at least one instance of each event currently part of the FCPN set
    // 4. create the map between event and linked regions for each FCPN, avoiding constraints where one label is
    // connected to more than 2 regions
    // (OPTIONAL: don't know if needed) 4b. create clauses avoiding the removal of regions having more than one outgoing edges
    // 5. translate the map into clauses
    // 6. create clauses for the events with pbLib
    // 7. solve the SAT problem decreasing the value of the event sum -> starting value is the sum of all events'
    // instances
    // 8. decode the result leaving only the states corresponding to regions of the model
    // ENCODINGS:
    // N regions, K FCPNs, M labels
    // ENCODING FOR LABEL i OF FCPN j; M*(j-1)+i , 1 <= i <= M, 1 <= j <= K      Values range [1, M*K], i cannot use 1
    // it's an invalid variable value
    // ENCODING FOR REGION i OF FCPN j: (M*K)+N*(j-1)+i, 1 <= i <= N, 1 <= j <= K Values range [M*K+1, M*K+N*(K-1)+N =
    // K*(N+M)]
    auto clauses = new vector<vector<int32_t> *>();

    //STEP 1:
    map<SM *, int> FCPNs_map;
    map<int, SM *> FCPNs_map_inverted;
    int counter = 1;
    for (auto FCPN: *FCPNs) {
        FCPNs_map[FCPN] = counter;
        FCPNs_map_inverted[counter] = FCPN;
        counter++;
    }

    //STEP 2:
    map<Region *, int> regions_map_for_sat;
    counter = 1;
    for (auto FCPN: *FCPNs) {
        for (auto reg: *FCPN) {
            if (regions_map_for_sat.find(reg) == regions_map_for_sat.end()) {
                regions_map_for_sat[reg] = counter;
                counter++;
            }
        }
    }

    // STEP 3 using encoding for regions
    int K = FCPNs->size();
    int N = counter;
    int M = number_of_events;
    for (auto vec: *clauses) {
        delete vec;
    }
    clauses->clear();
    vector<int32_t> *clause;
    for (auto rec: regions_map_for_sat) {
        auto region = rec.first;
        auto region_counter = rec.second;
        clause = new vector<int32_t>();
        for (auto FCPN: *FCPNs) {
            int SM_counter = FCPNs_map[FCPN];
            if (FCPN->find(region) != FCPN->end()) {
                clause->push_back((M * K) + N * (SM_counter - 1) + region_counter);
            }
        }
        clauses->push_back(clause);
    }


    //STEP 3b
    /*
    auto events_inFCPNs = new map<int, set<set<Region *>*>*>();
    for(auto rec: *map_of_FCPN_pre_regions){
        auto FCPN = rec.first;
        for(auto rec1: *rec.second){
            auto event = rec1.first;
            if(events_inFCPNs->find(event) == events_inFCPNs->end())
                (*events_inFCPNs)[event] = new set<set<Region *>*>();
            (*events_inFCPNs)[event]->insert(FCPN);
        }
    }

    for(auto rec: *events_inFCPNs){
        auto event = rec.first;
        clause = new vector<int32_t>();
        for(auto FCPN: *rec.second){
            if(events_inFCPNs->at(event)->find(FCPN) != events_inFCPNs->at(event)->end()) {
                int FCPN_counter = FCPNs_map[FCPN];
                auto ev_encoding = M * (FCPN_counter - 1) + event + 1;
                clause->push_back(ev_encoding);
            }
        }
        clauses->push_back(clause);
    }*/


    map<int, pair<int, int>> encoded_events_map; //map <encoded value, pair<sm, decoded_value>
    set<int> encoded_regions_set; //will be used in the 6th step

    //STEPS 4 and 5: conversion into clauses of pre and post regions for each SM
    for (auto FCPN: *FCPNs) {
        int FCPN_counter = FCPNs_map[FCPN];
        auto regions_connected_to_labels = merge_2_maps((*map_of_FCPN_pre_regions)[FCPN],
                                                        (*map_of_FCPN_post_regions)[FCPN]);

        //conversion into clauses
        for (auto rec: *regions_connected_to_labels) {
            auto ev = rec.first + 1; //events range must start from 1 not 0
            auto ev_encoding = M * (FCPN_counter - 1) + ev;
            (encoded_events_map)[ev_encoding] = make_pair(FCPN_counter, ev - 1);
            if (rec.second->size() <= 2) {
                for (auto reg: *rec.second) {
                    int region_counter = regions_map_for_sat[reg];
                    int region_encoding = (M * K) + N * (FCPN_counter - 1) + region_counter;
                    encoded_regions_set.insert(region_encoding);
                    clause = new vector<int32_t>();
                    clause->push_back(-region_encoding);
                    clause->push_back(ev_encoding);
                    clauses->push_back(clause);
                }
            }
                //event connected to more regions therefore we avoid merges which removes this label
            else {
                clause = new vector<int32_t>();
                clause->push_back(ev_encoding);
                clauses->push_back(clause);
            }
        }
        for (auto rec: *regions_connected_to_labels) {
            delete rec.second;
        }
        delete regions_connected_to_labels;
    }

    // STEP 4b
    /*
    auto preregion_for = new map <set<Region*>*, map<Region*, set<int>*>*>();
    for(auto FCPN: *FCPNs){
        if(preregion_for->find(FCPN) == preregion_for->end())
            (*preregion_for)[FCPN] = new map<Region *, set<int>*>();
        for(auto rec: *(*map_of_FCPN_pre_regions)[FCPN]){
            auto event = rec.first;
            for(auto reg: *rec.second){
                if((*preregion_for)[FCPN]->find(reg) == (*preregion_for)[FCPN]->end())
                    (*(*preregion_for)[FCPN])[reg] = new set<int>();
                (*(*preregion_for)[FCPN])[reg]->insert(event);
            }
        }
    }

    for(auto rec: *preregion_for){
        auto FCPN = rec.first;
        int FCPN_counter = FCPNs_map[FCPN];
        for(auto rec1: *rec.second){
            if(rec1.second->size() > 1){
                clause = new vector<int32_t>();
                int region_counter = regions_map_for_sat[rec1.first];
                int region_encoding = (M * K) + N * (FCPN_counter - 1) + region_counter;
                clause->push_back(region_encoding);
                clauses->push_back(clause);
            }
        }
    }

    for(auto rec: *preregion_for){
        for(auto rec1: *rec.second){
            delete rec1.second;
        }
        delete rec.second;
    }
    delete preregion_for;*/

    //STEP 6:
    vector<WeightedLit> literals_from_events = {};

    literals_from_events.reserve(encoded_events_map.size()); //improves the speed
    for (auto rec: encoded_events_map) {
        auto ev_encoded = rec.first;
        literals_from_events.emplace_back(ev_encoded, 1);
    }
    for (auto reg_encoded: encoded_regions_set) {
        literals_from_events.emplace_back(reg_encoded, 0);
    }

    //STEP 7:
    int current_value = encoded_events_map.size();
    int min = 0;
    int max = encoded_events_map.size();

    PBConfig config = make_shared<PBConfigClass>();
    VectorClauseDatabase formula(config);
    PB2CNF pb2cnf(config);
    AuxVarManager auxvars(K * (N + M) + 1);
    for (auto cl: *clauses) {
        formula.addClause(*cl);
    }

    Minisat::Solver solver;

    bool sat = true;
    vec < lbool > true_model;

    /*
    int number_of_missing_event_instances = 0;
    int number_of_missing_region_instances = 0;
    for(auto FCPN: *FCPNs){
        int max_reg = map_of_FCPN_pre_regions->at(FCPN)->size();
        int tmp = map_of_FCPN_post_regions->at(FCPN)->size();
        if(tmp > max_reg)
            max_reg = tmp;
        if(max_reg < number_of_events){
            number_of_missing_event_instances += number_of_events - max_reg;
        }
        number_of_missing_region_instances += N - FCPN->size();
    }*/

    //iteration in the search of a correct assignment decreasing the total weight
    do {
        PBConstraint constraint(literals_from_events, LEQ,
                                current_value); //the sum have to be lesser or equal to current_value
        pb2cnf.encode(constraint, formula, auxvars);
        int num_clauses_formula = formula.getClauses().size();
        dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar() //-
                //number_of_missing_event_instances -
                //number_of_missing_region_instances
                ,
                                        num_clauses_formula,
                                        formula.getClauses());
        sat = check_sat_formula_from_dimacs(solver, dimacs_file);
        if (sat) {
            if (decomposition_debug) {
                //cout << "----------" << endl;
                cout << "SAT with value " << current_value << endl;
                //cout << "formula: " << endl;
                //formula.printFormula(cout);
                cout << "Model: ";
                for (int i = 0; i < solver.nVars(); ++i) {
                    if (solver.model[i] != l_Undef) {
                        fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                i + 1);
                    }
                }
                cout << endl;
            }
            true_model.clear(true);
            for (auto val: solver.model) {
                true_model.push(val);
            }
            //maxValueToCheck--;
            max = current_value;
        } else {
            if (decomposition_debug) {
                //cout << "----------" << endl;
                cout << "UNSAT with value " << current_value << endl;
            }
            min = current_value;
        }
        current_value = (min + max) / 2;
    } while ((max - min) > 1);
    //if(decomposition_debug)
    //   cout << "UNSAT with value " << maxValueToCheck << endl;

    //STEP 8:
    //new decode algorithm:
    //take all negated events
    //given a negated event check the two connected regions, at least one of them will be negated
    //take it and remove the region and event connecting to other parts of the SM

    vector<int> to_remove;
    for (auto rec: encoded_events_map) {
        auto encoded_event = rec.first;
        if (solver.model[encoded_event - 1] == l_False) {
            if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;
            to_remove.push_back(encoded_event);
        }
    }

    auto events_to_remove_per_FCPN = new map<SM *, set<int> *>();

    for (auto encoded_event: to_remove) {
        int FCPN_counter = encoded_events_map[encoded_event].first;
        int decoded_event = encoded_events_map[encoded_event].second;

        SM *current_FCPN = FCPNs_map_inverted[FCPN_counter];
        if (decomposition_debug) {
            cout << "in FCPN " << FCPN_counter << " removing event " << decoded_event << " ";// << endl;
            if (decoded_event >= num_events) {
                cout << aliases_map_number_name->at(aliases->at(decoded_event)) << endl;
            } else {
                cout << aliases_map_number_name->at(decoded_event) << endl;
            }
        }

        if (events_to_remove_per_FCPN->find(current_FCPN) == events_to_remove_per_FCPN->end()) {
            (*events_to_remove_per_FCPN)[current_FCPN] = new set<int>();
        }
        (*events_to_remove_per_FCPN)[current_FCPN]->insert(decoded_event);
    }

    // NEW MERGE
    for (auto rec: *events_to_remove_per_FCPN) {
        //cout << "removing events SM" << endl;
        SM *current_FCPN = rec.first;
        set<int> *removed_events = rec.second;
        auto regions_to_merge = new vector<set<Region *> *>();
        for (auto ev: *removed_events) {
            auto regions_connected_to_ev = new set<Region *>();
            //a transition can be without input edges: infinite firing
            if ((*(*map_of_FCPN_pre_regions)[current_FCPN])[ev] != nullptr) {
                for (auto reg: *(*(*map_of_FCPN_pre_regions)[current_FCPN])[ev]) {
                    regions_connected_to_ev->insert(reg);
                }
            }
            //a transition can be without exiting edges: firing it removes all incoming tokens from PN
            if ((*(*map_of_FCPN_post_regions)[current_FCPN])[ev] != nullptr) {
                for (auto reg: *(*(*map_of_FCPN_post_regions)[current_FCPN])[ev]) {
                    regions_connected_to_ev->insert(reg);
                }
            }
            regions_to_merge->push_back(regions_connected_to_ev);
        }

        for (int i = 0; i < regions_to_merge->size(); ++i) {
            for (int k = i + 1; k < regions_to_merge->size(); ++k) {
                bool merge = false;
                for (auto reg: *(*regions_to_merge)[i]) {
                    if ((*regions_to_merge)[k]->find(reg) != (*regions_to_merge)[k]->end()) {
                        merge = true;
                        break;
                    }
                }
                for (auto reg: *(*regions_to_merge)[k]) {
                    regions_to_merge->at(i)->insert(reg);
                }
                regions_to_merge->erase(regions_to_merge->begin() + k);
                k--;
            }
        }

        auto to_erase = set<Region *>();
        for(auto working_set: *regions_to_merge){
            auto merge = regions_union(working_set);
            current_FCPN->insert(merge);
            for(auto reg: *working_set){
                to_erase.insert(reg);
            }
        }
        for(auto reg: to_erase){
            current_FCPN->erase(reg);
        }
    }

    //UPDATING MAP OF FCPN-PRE/POST REGIONS

    for (auto FCPN: *FCPNs) {
        auto removed_events_FCPN = (*events_to_remove_per_FCPN)[FCPN];
        if (removed_events_FCPN != nullptr) {
            delete (*map_of_FCPN_pre_regions)[FCPN];
            delete (*map_of_FCPN_post_regions)[FCPN];
            //cout << "updating pre-regions map SM" << endl;
            (*map_of_FCPN_pre_regions)[FCPN] = Pre_and_post_regions_generator::create_pre_regions_for_FCPN(FCPN,
                                                                                                           removed_events_FCPN);
            //cout << "updating post-regions map SM" << endl;
            (*map_of_FCPN_post_regions)[FCPN] = Pre_and_post_regions_generator::create_post_regions_for_FCPN(
                    (*map_of_FCPN_pre_regions)[FCPN]);
            if((*map_of_FCPN_pre_regions)[FCPN]->size() != (*map_of_FCPN_post_regions)[FCPN]->size()){
                cerr << "WARNING: different map sizes" << endl;
            }
        }
    }

}

void FCPN_Merge::print_after_merge(set<set<Region *> *> *FCPNs,
                              map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_pre_regions,
                              map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_post_regions,
                              map<int, int> *aliases,
                              const string& file){

    int FCPN_counter = 0;
    for (auto FCPN: *FCPNs) {
        print_fcpn_dot_file((*map_of_FCPN_pre_regions)[FCPN], (*map_of_FCPN_post_regions)[FCPN], aliases, file, FCPN_counter);
        FCPN_counter++;
    }
}

FCPN_Merge::~FCPN_Merge()= default;