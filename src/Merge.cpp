/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Merge.h"

using namespace PBLib;
using namespace Minisat;
using namespace Utilities;

Merge::Merge(set<SM *> *SMs,
             int number_of_events,
             map<SM *, map<int, Region *> *> *map_of_SM_pre_regions,
             map<SM *, map<int, Region *> *> *map_of_SM_post_regions,
             const string& file,
             map<Region *, int> *regions_alias_mapping){

    //STEPS OF THE ALGORITHM
    // 1. create the map between SMs and integers from 1 to K
    // 2. create the map between regions used in the SMs and integers from 1 to N -> one index for all different instances of the same region
    // 3. create clauses to satisfy at least one instance of each region: (r1i -v -v - r1k) at least one instance of r1 have to be true
    // 4. create the map between event and linked regions for each SM
    // 5. translate the map into clauses
    // 6. create clauses for the events with pbLib
    // 7. solve the SAT problem decreasing the value of the event sum -> starting value is the sum of all events' instances
    // 8. decode the result leaving only the states corresponding to regions of the model
    // ENCODINGS:
    // N regions, K FSMs, M labels
    // ENCODING FOR LABEL i OF FSM j; M*(j-1)+i , 1 <= i <= M, 1 <= j <= K      Values range [1, M*K], i cannot use 1 it's an invalid variable value
    // ENCODING FOR REGION i OF FSM j: (M*K)+N*(j-1)+i, 1 <= i <= N, 1 <= j <= K Values range [M*K+1, M*K+N*(K-1)+N = K*(N+M)]

    //STEP 1:
    map < SM * , int > SMs_map;
    map < int, SM * > SMs_map_inverted;
    int counter = 1;
    for (auto SM: *SMs) {
        SMs_map[SM] = counter;
        SMs_map_inverted[counter] = SM;
        counter++;
    }

    //STEP 2:
    map < Region * , int > regions_map_for_sat;
    counter = 1;
    for (auto SM: *SMs) {
        for (auto reg: *SM) {
            if (regions_map_for_sat.find(reg) == regions_map_for_sat.end()) {
                regions_map_for_sat[reg] = counter;
                counter++;
            }
        }
    }

    // STEP 3 using encoding for regions
    int K = SMs->size();
    int N = counter;
    int M = number_of_events;
    clauses = new vector<vector<int32_t>*>();
    vector<int32_t> *clause;
    for (auto rec: regions_map_for_sat) {
        auto region = rec.first;
        auto region_counter = rec.second;
        clause = new vector<int32_t>();
        for (auto SM : *SMs) {
            int SM_counter = SMs_map[SM];
            if (SM->find(region) != SM->end()) {
                clause->push_back((M * K) + N * (SM_counter - 1) + region_counter);
            }
        }
        clauses->push_back(clause);
    }

    //set<int> encoded_events_set; //will be used in the 6th step
    map<int, pair<int, int>> encoded_events_map; //map <encoded value, pair<sm, decoded_value>
    set<int> encoded_regions_set; //will be used in the 6th step

    //STEPS 4 and 5: conversion into clauses of pre and post regions for each SM
    for (auto sm: *SMs) {
        int SM_counter = SMs_map[sm];
        auto regions_connected_to_labels = merge_2_maps((*map_of_SM_pre_regions)[sm],
                                                        (*map_of_SM_post_regions)[sm]);

        //conversion into clauses
        for (auto rec: *regions_connected_to_labels) {
            auto ev = rec.first + 1; //events range must start from 1 not 0
            auto ev_encoding = M * (SM_counter - 1) + ev;
            (encoded_events_map)[ev_encoding] = make_pair(SM_counter, ev - 1);
            for (auto reg: *rec.second) {
                int region_counter = regions_map_for_sat[reg];
                int region_encoding = (M * K) + N * (SM_counter - 1) + region_counter;
                encoded_regions_set.insert(region_encoding);
                clause = new vector<int32_t>();
                clause->push_back(-region_encoding);
                clause->push_back(ev_encoding);
                clauses->push_back(clause);
            }
        }
        for (auto rec: *regions_connected_to_labels) {
            delete rec.second;
        }
        delete regions_connected_to_labels;
    }

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

    //int maxValueToCheck = encoded_events_map.size();
    int current_value = encoded_events_map.size();
    int min = 0;
    int max = encoded_events_map.size();

    PBConfig config = make_shared<PBConfigClass>();
    VectorClauseDatabase formula(config);
    //VectorClauseDatabase last_sat_formula(config);
    PB2CNF pb2cnf(config);
    AuxVarManager auxvars(K * (N + M) + 1);
    for (auto cl: *clauses) {
        formula.addClause(*cl);
    }

    /*if(decomposition_debug) {
        cout << "clauses before merge:" << endl;
        formula.printFormula(cout);
    }*/

    Minisat::Solver solver;

    bool sat = true;
    vec < lbool > true_model;

    IncPBConstraint constraint(literals_from_events, LEQ,
                               current_value); //the sum have to be less or equal to current_value
    pb2cnf.encodeIncInital(constraint, formula, auxvars);
    //iteration in the search of a correct assignment decreasing the total weight
    do {
        int num_clauses_formula = formula.getClauses().size();
        dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                        formula.getClauses(), nullptr);
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
                }*/
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
            /*
            if (decomposition_debug)
                cout << "add encoding " << encoded_event << " to removal events" << endl;*/
            to_remove.push_back(encoded_event);
        }
    }

    events_to_remove_per_SM = new map<SM *, set<int> *>();

    for (auto encoded_event: to_remove) {
        int SM_counter = encoded_events_map[encoded_event].first;
        int decoded_event = encoded_events_map[encoded_event].second;

        SM * current_SM = SMs_map_inverted[SM_counter];
        if (decomposition_debug) {
            cout << "in SM " << SM_counter << endl;
            cout << "removing event " << decoded_event << endl;
        }
        if (events_to_remove_per_SM->find(current_SM) == events_to_remove_per_SM->end()) {
            (*events_to_remove_per_SM)[current_SM] = new set<int>();
        }
        (*events_to_remove_per_SM)[current_SM]->insert(decoded_event);
    }

    // NEW MERGE
    for (auto rec: *events_to_remove_per_SM) {
        //cout << "removing events SM" << endl;
        SM * current_SM = rec.first;
        set<int> *removed_events = rec.second;
        auto regions_to_merge = new vector<set<Region *> *>();
        for (auto ev: *removed_events) {
            auto regions_connected_to_ev = new set<Region *>();
            //a transition can be without input edges: infinite firing
            if((*(*map_of_SM_pre_regions)[current_SM])[ev] != nullptr)
                regions_connected_to_ev->insert((*(*map_of_SM_pre_regions)[current_SM])[ev]);
            //a transition can be without exiting edges: firing it removes all incoming tokens from PN
            if((*(*map_of_SM_post_regions)[current_SM])[ev] != nullptr)
                regions_connected_to_ev->insert((*(*map_of_SM_post_regions)[current_SM])[ev]);
            regions_to_merge->push_back(regions_connected_to_ev);
        }

        for(auto reg_set: *regions_to_merge) {
            if (reg_set->size() > 1) {
                //union between sets
                //cout << "union between sets" << endl;
                for (unsigned long i = 0; i < regions_to_merge->size(); i++) {
                    for (unsigned long k = i + 1; k < regions_to_merge->size(); k++) {
                        //check if intersection between region sets is empty or not
                        bool merge = false;
                        if (!(*regions_to_merge)[i]->empty()) {
                            merge = !empty_region_set_intersection((*regions_to_merge)[i], (*regions_to_merge)[k]);
                            if (merge) {
                                for (Region *reg: *(*regions_to_merge)[i])
                                    (*regions_to_merge)[k]->insert(reg);
                                (*regions_to_merge)[i]->clear();
                            }
                        }
                    }
                }
                //merge effettivo tra regioni
                set<Region *> merged_regions;
                for (auto regionSet: *regions_to_merge) {
                    merged_regions.insert(regions_union(regionSet));
                }


                //removing regions used for the merge
                //cout << "removing regions" << endl;
                vector<Region *> to_erase;
                for (Region *reg: *current_SM) {
                    for (Region *mergedReg: merged_regions) {
                        if (at_least_one_state_from_first_in_second(reg, mergedReg)) {
                            //cout << "Adding for removal region ";
                            //println(*reg);
                            to_erase.push_back(reg);
                            //current_SM->erase(reg);
                        }
                    }
                }

                for (auto reg: to_erase) {
                    if(decomposition_debug) {
                        cout << "Removing region ";
                        println(*reg);
                    }
                    current_SM->erase(reg);
                }


                //cout << "adding merged regions" << endl;
                for (Region *mergedReg: merged_regions) {
                    if (!mergedReg->empty()) {
                        if(decomposition_debug) {
                            cout << "adding a merged region to an SM" << endl;
                            println(*mergedReg);
                        }
                        current_SM->insert(mergedReg);
                        if(regions_alias_mapping != nullptr) {
                            int m = -1;
                            for (auto r: *regions_alias_mapping) {
                                if (r.second > m)
                                    m = r.second;
                            }
                            (*regions_alias_mapping)[mergedReg] = m + 1;
                        }
                    } else {
                        delete mergedReg;
                    }
                }
                //todo: nel caso dell'input clock.g e alloc-outbound.g ci sono memory leak
                /*for(Region * r: merged_regions){
                    cout << "removing a region" << endl;
                    println(*r);
                    delete r;
                }*/
            } else {
                if(decomposition_debug)
                    cout << "Removed redundant label without merging regions" << endl;
            }
        }

        for (auto reg_set: *regions_to_merge) {
            delete reg_set;
        }
        delete regions_to_merge;
    }

    //UPDATING MAP OF SM-PRE/POST REGIONS

    for (auto sm: *SMs) {
        auto removed_events_SM = (*events_to_remove_per_SM)[sm];
        if (removed_events_SM != nullptr) {
            delete (*map_of_SM_pre_regions)[sm];
            delete (*map_of_SM_post_regions)[sm];
            //cout << "updating pre-regions map SM" << endl;
            (*map_of_SM_pre_regions)[sm] = Pre_and_post_regions_generator::create_pre_regions_for_SM(sm,
                                                                                                     removed_events_SM);
            //cout << "updating post-regions map SM" << endl;
            (*map_of_SM_post_regions)[sm] = Pre_and_post_regions_generator::create_post_regions_for_SM(
                    (*map_of_SM_pre_regions)[sm]);
        }
    }
//END NEW MERGE
}

void Merge::print_after_merge(set<SM *> *SMs,
                          map<SM *, map<int, Region *> *> *map_of_SM_pre_regions,
                          map<SM *, map<int, Region *> *> *map_of_SM_post_regions,
                          map<int, int> *aliases,
                          const string& file) {
//CREATION OF THE TRANSITIONS BETWEEN STATES OF THE SM
//cout << "pre-regions" << endl;
//print(*pprg->get_pre_regions());

if(!decomposition_output_sis){
    //creation of aliases for each region
    int reg_cont = 1;
    sm_region_aliases = new map<Region *, int>();
    for (SM *sm: *SMs) {
        for(Region *reg: *sm){
            if(sm_region_aliases->find(reg) == sm_region_aliases->end()) {
                (*sm_region_aliases)[reg] = reg_cont;
                reg_cont++;
            }
        }
    }
}

int SM_counter = 0;
for (SM *sm: *SMs) {
    //counter = (SMs_map)[sm];
    string SM_name = remove_extension(file);
    SM_name += "_SM_" + to_string(SM_counter) + ".g";
    if (decomposition_output_sis) {
        print_sm_g_file((*map_of_SM_pre_regions)[sm], (*map_of_SM_post_regions)[sm], aliases, SM_name);
    } else {
        print_sm_dot_file((*map_of_SM_pre_regions)[sm], (*map_of_SM_post_regions)[sm], aliases,
                          SM_name);
    }
    SM_counter++;
}

delete sm_region_aliases;
}

Merge::~Merge(){
    for (auto rec: *events_to_remove_per_SM) {
        delete rec.second;
    }
    delete events_to_remove_per_SM;
    for (auto vec: *clauses) {
        delete vec;
    }
    delete clauses;
}