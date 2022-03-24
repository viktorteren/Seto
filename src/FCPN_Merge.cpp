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
    //      instances of the same region
    // 3. create clauses to satisfy at least one instance of each region: (r1i -v -v - r1k) at least one instance of r1
    //      has to be true
    // 4. create the map between event and linked regions for each FCPN, avoiding constraints where one label is
    //      connected to more than 2 regions
    // 5. translate the map into clauses
    // 5b: constraint on the empty intersection between regions which are going to be merged in other case if the
    //      intersection is not empty there is a check if the resultant set of states is a region
    // 5c. constraint which deny the loss of AC/FC property: given a couple of regions r1 and r2 connected by e and r1
    //      is a pre-region for e, r2 is a post-region for e, if r1 has more than one outgoing edges and exists an event
    //      having r2 as pre-region which has more than one pre-region then create a clause (e): denying the merge of
    //      these two regions
    // 6. create clauses for the events with pbLib
    // 7. solve the SAT problem decreasing the value of the event sum -> starting value is the sum of all events'
    //      instances
    // 8. decode the result leaving only the states corresponding to regions of the model
    //
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

    map<int, pair<int, int>> encoded_events_map; //map <encoded value, pair<sm, decoded_value>
    set<int> encoded_regions_set; //will be used in the 6th step

    //STEPS 4 and 5 and 5b: conversion into clauses of pre and post regions for each SM
    for (auto FCPN: *FCPNs) {
        int FCPN_counter = FCPNs_map[FCPN];
        auto regions_connected_to_labels = merge_2_maps((*map_of_FCPN_pre_regions)[FCPN],
                                                        (*map_of_FCPN_post_regions)[FCPN]);

        //conversion into clauses
        for (auto rec: *regions_connected_to_labels) {
            auto ev = rec.first;
            auto ev_encoding = M * (FCPN_counter - 1) + ev + 1;
            (encoded_events_map)[ev_encoding] = make_pair(FCPN_counter, ev);
            set<Region *>::iterator it1;
            set<Region *>::iterator it2;
            bool not_empty_intersection = false;
            for(it1=rec.second->begin();it1!=rec.second->end();++it1){
                for(it2=next(it1);it2!=rec.second->end();++it2){
                    if(!empty_regions_intersection(*it1, *it2)){
                        not_empty_intersection = true;
                        break;
                    }
                }
                if(not_empty_intersection)
                    break;
            }
            if(not_empty_intersection){
                if(decomposition_debug)
                    cout << "NOT EMPTY INTERSECTION for event " << ev << endl;
                auto temp_region = regions_union(rec.second);
                if(is_a_region(temp_region)){
                    if(decomposition_debug)
                        cerr << "UNION OF REGIONS WITH NOT EMPTY INTERSECTION CREATES A REGION" << endl;
                    for (auto reg: *rec.second) {
                        if(decomposition_debug){
                            if(rec.second->size() > 2)
                                cerr << "passed a merge with more than 2 regions and not empty intersection" << endl;
                        }
                        int region_counter = regions_map_for_sat[reg];
                        int region_encoding = (M * K) + N * (FCPN_counter - 1) + region_counter;
                        encoded_regions_set.insert(region_encoding);
                        clause = new vector<int32_t>();
                        clause->push_back(-region_encoding);
                        clause->push_back(ev_encoding);
                        clauses->push_back(clause);
                    }
                }
                else{
                    clause = new vector<int32_t>();
                    clause->push_back(ev_encoding);
                    clauses->push_back(clause);
                }
                delete temp_region;
            }
            else{
                for (auto reg: *rec.second) {
                    if(decomposition_debug){
                        if(rec.second->size() > 2)
                            cerr << "passed a merge with more than 2 regions" << endl;
                    }
                    int region_counter = regions_map_for_sat[reg];
                    int region_encoding = (M * K) + N * (FCPN_counter - 1) + region_counter;
                    encoded_regions_set.insert(region_encoding);
                    clause = new vector<int32_t>();
                    clause->push_back(-region_encoding);
                    clause->push_back(ev_encoding);
                    clauses->push_back(clause);
                }
            }
        }
        for (auto rec: *regions_connected_to_labels) {
            delete rec.second;
        }
        delete regions_connected_to_labels;
    }

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

    auto postregion_for = new map <set<Region*>*, map<Region*, set<int>*>*>();
    for(auto FCPN: *FCPNs){
        if(postregion_for->find(FCPN) == postregion_for->end())
            (*postregion_for)[FCPN] = new map<Region *, set<int>*>();
        for(auto rec: *(*map_of_FCPN_post_regions)[FCPN]){
            auto event = rec.first;
            for(auto reg: *rec.second){
                if((*postregion_for)[FCPN]->find(reg) == (*postregion_for)[FCPN]->end())
                    (*(*postregion_for)[FCPN])[reg] = new set<int>();
                (*(*postregion_for)[FCPN])[reg]->insert(event);
            }
        }
    }

    auto saved_encodings = new set<int>();
    auto map_fundamental_events_PN = new map<SM *, set<int>*>();
    //STEP 5c
    for(auto pn: *FCPNs){
        int FCPN_counter = FCPNs_map[pn];
        for(auto rec: *map_of_FCPN_pre_regions->at(pn)) {
            auto ev = rec.first;
            auto ev_encoding = M * (FCPN_counter - 1) + ev + 1;
            for (auto reg1: *rec.second) {
                if (saved_encodings->find(ev_encoding) == saved_encodings->end()) {
                    int number_outgoing_events = preregion_for->at(pn)->at(reg1)->size();
                    if (number_outgoing_events > 1) {
                        for (auto reg2: *map_of_FCPN_post_regions->at(pn)->at(ev)) {
                            if (saved_encodings->find(ev_encoding) == saved_encodings->end()) {
                                for (auto ev_temp: *preregion_for->at(pn)->at(reg2)) {
                                    if (saved_encodings->find(ev_encoding) == saved_encodings->end()) {
                                        int number_pre_regions_of_temp_event = map_of_FCPN_pre_regions->at(pn)->at(
                                                ev_temp)->size();
                                        if (number_pre_regions_of_temp_event > 1) {
                                            clause = new vector<int32_t>();
                                            clause->push_back(ev_encoding);
                                            clauses->push_back(clause);
                                            //cerr << "ev encoding: " << ev_encoding << endl;
                                            if(map_fundamental_events_PN->find(pn) == map_fundamental_events_PN->end()){
                                                (*map_fundamental_events_PN)[pn] = new set<int>();
                                            }
                                            map_fundamental_events_PN->at(pn)->insert(ev);
                                            if (decomposition_debug) {
                                                cerr << "Added constraint for PN structure safeness: "  << ev
                                                << " in ACPN/FCPN " << FCPN_counter << endl;
                                            }
                                            saved_encodings->insert(ev_encoding);
                                        }
                                    } else {
                                        if (decomposition_debug)
                                            cerr << "CACHE HIT 2" << endl;
                                        break;
                                    }
                                }
                            }
                            else{
                                if (decomposition_debug)
                                    cerr << "CACHE HIT 3" << endl;
                                break;
                            }
                        }
                    }
                }
                else{
                    if(decomposition_debug)
                        cerr << "CACHE HIT 4" << endl;
                    break;
                }
            }
        }
    }
    delete saved_encodings;

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

    PBConfig config = make_shared<PBConfigClass>();
    VectorClauseDatabase formula(config);
    PB2CNF pb2cnf(config);
    AuxVarManager auxvars(K * (N + M) + 1);
    for (auto cl: *clauses) {
        formula.addClause(*cl);
    }
    /*if(decomposition_debug)
        formula.printFormula(cout);*/

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
        IncPBConstraint constraint(literals_from_events, LEQ,
                                current_value); //the sum have to be lesser or equal to current_value
        pb2cnf.encodeIncInital(constraint, formula, auxvars);
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
                /*cout << "Model: ";
                for (int i = 0; i < solver.nVars(); ++i) {
                    if (solver.model[i] != l_Undef) {
                        fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                i + 1);
                    }
                }
                cout << endl;*/
            }
            true_model.clear(true);
            for (auto val: solver.model) {
                true_model.push(val);
            }
            current_value--;
        } else {
            if (decomposition_debug) {
                //cout << "----------" << endl;
                cout << "UNSAT with value " << current_value << endl;
            }
            break;
        }
    } while (true);
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
            /*if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;*/
            to_remove.push_back(encoded_event);
        }
    }

    auto events_to_remove_per_FCPN = new map<SM *, set<int> *>();

    for (auto encoded_event: to_remove) {
        int FCPN_counter = encoded_events_map[encoded_event].first;
        int decoded_event = encoded_events_map[encoded_event].second;
        //cerr << "encodings to remove " << encoded_event << " decoded as " << decoded_event << " in ACPN/FCPN " <<FCPN_counter << endl;

        SM *current_FCPN = FCPNs_map_inverted[FCPN_counter];
        if (decomposition_debug) {
            cout << "in FCPN " << FCPN_counter << " removing event " << decoded_event << " ";// << endl;
            if (decoded_event >= num_events) {
                cout << aliases_map_number_name->at(aliases->at(decoded_event)) << endl;
            } else {
                cout << aliases_map_number_name->at(decoded_event) << endl;
            }
            //auto regions_connected_to_labels = merge_2_maps((*map_of_FCPN_pre_regions)[current_FCPN],
             //                                               (*map_of_FCPN_post_regions)[current_FCPN]);
        }

        if (events_to_remove_per_FCPN->find(current_FCPN) == events_to_remove_per_FCPN->end()) {
            (*events_to_remove_per_FCPN)[current_FCPN] = new set<int>();
        }
        (*events_to_remove_per_FCPN)[current_FCPN]->insert(decoded_event);
    }

    // NEW MERGE
    for (auto rec: *events_to_remove_per_FCPN) {
        //cout << "removing events FCPN" << endl;
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
                if(merge) {
                    for (auto reg: *(*regions_to_merge)[k]) {
                        regions_to_merge->at(i)->insert(reg);
                    }
                    regions_to_merge->erase(regions_to_merge->begin() + k);
                    k=i;
                }
            }
        }

        set<int> *cancelled_events = nullptr;
        do {
            delete cancelled_events;
            cancelled_events = new set<int>();
            for (auto reg_set : *regions_to_merge) {
                int occurrences = 0;
                bool multiple_exit = false;
                bool multiple_enter = false;
                auto involved_regions = new set<Region *>();
                for (auto reg: *reg_set) {
                    if (preregion_for->at(current_FCPN)->at(reg)->size() > 1) {
                        multiple_exit = true;
                        /*if(decomposition_debug)
                            cerr << "MULTIPLE EXIT" << endl;*/
                    }
                    if (postregion_for->at(current_FCPN)->at(reg)->size() > 1) {
                        multiple_enter = true;
                    }
                    if (multiple_exit || multiple_enter) {
                        occurrences++;
                        involved_regions->insert(reg);
                    }
                }
                //todo: relax the constraint checking if only pre regions of involved events are choices and
                // only post-regions of involved events are returns from choice, in this way false positives could be
                // avoided improving the minimization
                if (multiple_exit && multiple_enter && occurrences > 1) {
                    if(decomposition_debug)
                        cout << "removing an event from a merge with " << occurrences << " occurrences" << endl;
                    bool entered = false;
                    for (auto r1: *reg_set) {
                        for (auto r2: *reg_set) {
                            if (r1 != r2) {
                                if (!entered) {
                                    if (involved_regions->find(r1) != involved_regions->end() ||
                                        involved_regions->find(r2) != involved_regions->end()) {
                                        for (auto ev: *events_to_remove_per_FCPN->at(current_FCPN)) {
                                            if (map_of_FCPN_post_regions->at(current_FCPN)->at(ev)->find(r1) !=
                                                map_of_FCPN_post_regions->at(current_FCPN)->at(ev)->end()) {
                                                if (map_of_FCPN_pre_regions->at(current_FCPN)->at(ev)->find(r2) !=
                                                    map_of_FCPN_pre_regions->at(current_FCPN)->at(ev)->end()) {
                                                    cancelled_events->insert(ev);
                                                    entered = true;
                                                    if(decomposition_debug)
                                                        cout << "removed event: " << ev << endl;
                                                }
                                            } else if (map_of_FCPN_post_regions->at(current_FCPN)->at(ev)->find(r2) !=
                                                       map_of_FCPN_post_regions->at(current_FCPN)->at(ev)->end()) {
                                                if (map_of_FCPN_pre_regions->at(current_FCPN)->at(ev)->find(r1) !=
                                                    map_of_FCPN_pre_regions->at(current_FCPN)->at(ev)->end()) {
                                                    cancelled_events->insert(ev);
                                                    entered = true;
                                                    if(decomposition_debug)
                                                        cout << "removal cancelled for event: " << ev << endl;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    //regions_to_merge->erase(regions_to_merge->begin()+i);
                    //i--;
                }
                delete involved_regions;
            }

            if (!cancelled_events->empty()) {
                delete regions_to_merge;
                regions_to_merge = new vector<set<Region *> *>();
                for (auto ev: *removed_events) {
                    if (cancelled_events->find(ev) == cancelled_events->end()) {
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
                        if (merge) {
                            for (auto reg: *(*regions_to_merge)[k]) {
                                regions_to_merge->at(i)->insert(reg);
                            }
                            regions_to_merge->erase(regions_to_merge->begin() + k);
                            k = i;
                        }
                    }
                }
            }


            //(CURRENTLY NOT USED) additional check of current merges: merging regions the PN will still be an ACPN?
            auto avoided_sets = new set<set<Region *>*>();
            for(auto reg_set: *regions_to_merge){
                bool counterexample_found = false;
                for(auto reg1: *reg_set){
                    if(counterexample_found)
                        break;
                    for(auto reg2: *reg_set){
                        if(counterexample_found)
                            break;
                        if(reg1 != reg2){
                            auto events_after_reg1 = preregion_for->at(current_FCPN)->at(reg1);
                            auto events_before_reg2 = postregion_for->at(current_FCPN)->at(reg2);
                            auto events_in_middle = new set<int>();
                            for(auto ev: *events_after_reg1){
                                if(events_before_reg2->find(ev) != events_before_reg2->end()){
                                    events_in_middle->insert(ev);
                                }
                            }
                            for(auto ev: *events_in_middle){
                                if(counterexample_found)
                                    break;
                                if(map_fundamental_events_PN->find(current_FCPN) != map_fundamental_events_PN->end()) {
                                    if (map_fundamental_events_PN->at(current_FCPN)->find(ev) !=
                                        map_fundamental_events_PN->at(current_FCPN)->end()) {
                                        avoided_sets->insert(reg_set);
                                        counterexample_found = true;
                                        if (decomposition_debug) {
                                            //todo: this kind of error not always is found, need further investigation
                                            cerr << "FOUND incompatible set of regions, a merge was avoided: " << ev << " in " << (acpn ? "ACPN " : "FCPN ")
                                            << FCPNs_map[current_FCPN] << endl;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if(!avoided_sets->empty()) {
                for (int i = 0; i < regions_to_merge->size(); ++i) {
                    if (avoided_sets->find(regions_to_merge->at(i)) != avoided_sets->end()) {
                        regions_to_merge->erase(regions_to_merge->begin() + i);
                        i--;
                    }
                }
            }

            for(auto ev: *cancelled_events){
                events_to_remove_per_FCPN->at(current_FCPN)->erase(ev);
            }
        } while(!cancelled_events->empty());


        auto to_erase = set<Region *>();
        for(auto working_set: *regions_to_merge){
            auto merge = regions_union(working_set);
            if(decomposition_debug) {
                /*
                for(auto reg: *working_set){
                    if(preregion_for->at(current_FCPN)->at(reg)->size() > 1){
                        cerr << "merging on choice place" << endl;
                    }
                }*/
                cout << "merging regions: " << endl;
                for (auto reg: *working_set) {
                    println(*reg);
                }
                cout << "into" << endl;
                println(*merge);
            }
            set<set<int> *>::iterator it1;
            set<set<int> *>::iterator it2;
            for(it1 = working_set->begin();it1 != working_set->end();++it1){
                for(it2 = next(it1);it2 != working_set->end();++it2){
                    auto inters = regions_intersection(*it1,*it2);
                    if(!inters->empty()){
                        cerr << "NOT EMPTY MERGE INTERSECTION" << endl;
                        /*if(is_a_region(inters))
                            cerr << "THE INTERSECTION IS A REGION" << endl;*/
                    }
                }
            }
            current_FCPN->insert(merge);
            for(auto reg: *working_set){
                to_erase.insert(reg);
            }
        }
        for(auto reg: to_erase){
            current_FCPN->erase(reg);
        }
        delete regions_to_merge;
        delete cancelled_events;
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
        }
    }

    //END MERGE
    if(output){
        print_after_merge(FCPNs, map_of_FCPN_pre_regions, map_of_FCPN_post_regions,aliases,file);
    }

    for(auto cl: *clauses){
        delete cl;
    }
    delete clauses;

    for(auto rec: *preregion_for){
        for(auto rec1: *rec.second){
            delete rec1.second;
        }
        delete rec.second;
    }
    delete preregion_for;

    for(auto rec: *postregion_for){
        for(auto rec1: *rec.second){
            delete rec1.second;
        }
        delete rec.second;
    }
    delete postregion_for;


    for(auto rec: *events_to_remove_per_FCPN){
        delete rec.second;
    }
    delete events_to_remove_per_FCPN;

    for(auto rec: *map_fundamental_events_PN){
        delete rec.second;
    }
    delete map_fundamental_events_PN;
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