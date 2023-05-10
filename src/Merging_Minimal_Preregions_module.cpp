/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Merging_Minimal_Preregions_module.h"

Merging_Minimal_Preregions_module::Merging_Minimal_Preregions_module(
        map<int, set<Region *> *> *essential_preregions,
        map<int, set<Region *> *> *irredundant_preregions, map<int, ES> *ES) {
    es = ES;

    total_pre_regions_map = merge_2_maps(essential_preregions, irredundant_preregions);
    merged_pre_regions_map = merging_preregions(ES);
}

__attribute__((unused)) Merging_Minimal_Preregions_module::Merging_Minimal_Preregions_module(
                                                                    map<int, set<Region *> *> *preregions,
                                                                    map<int, ES> *ES,
                                                                    bool gen) {
    es = ES;
    generic = gen;
    total_pre_regions_map = preregions;
    merged_pre_regions_map = merging_preregions(ES);
}

Merging_Minimal_Preregions_module::~Merging_Minimal_Preregions_module() {
    for (auto el : *total_pre_regions_map)
        delete el.second;
    delete total_pre_regions_map;

    if (merged_pre_regions_map != nullptr) {
        for (auto el : *merged_pre_regions_map)
            delete el.second;
        delete merged_pre_regions_map;
        delete union_ptr;
    }

    for (auto el : *es)
        delete el.second;
    delete es;
}


map<int, set<Region *> *> *Merging_Minimal_Preregions_module::get_merged_preregions_map() {
    return merged_pre_regions_map;
}

map<int, set<Region *> *> *Merging_Minimal_Preregions_module::get_total_preregions_map() {
    return total_pre_regions_map;
}

//execute te first merge and then return the result, could be modified to do it on all possible combinations
map<int, set<Region *> *> *Merging_Minimal_Preregions_module::merging_preregions(map<int, set<int> *> *ER_map) {

    //cout << "MERGING PREREGIONS___________" << endl;

    set<Region *> *preregions_set = copy_map_to_set(total_pre_regions_map);
    Region *reg_union = nullptr;
    //int cont_preregions = 0;

    /* for (auto el : *preregions_set) {
       println(*el);
     }*/

    // for each pair of pre-regions
    // for each event
    // check EC with the new region
    // if it is valid

    auto tmp_map = new map<int, set<Region *> *>();
    for (auto record : *total_pre_regions_map) {
        auto set = record.second;
        (*tmp_map)[record.first] = new std::set<Region *>(set->begin(), set->end());
    }

    set<Region *>::iterator it;
    set<Region *>::iterator it2;

    for (it = preregions_set->begin(); it != preregions_set->end(); ++it) {
        for (it2 = next(it); it2 != preregions_set->end(); ++it2) {
            auto reg1 = *it;
            auto reg2 = *it2;

            /*cout << "r1: ";
            println(*reg1);
            cout << "r2: ";
            println(*reg2);*/
            // the regions are disjointed!!!
            auto inter = regions_intersection(reg1, reg2);
            if (inter->empty()) {
                delete inter;
                if (!are_equal(reg1, reg2)) {
                    reg_union = regions_union(reg1, reg2);

                    //generic bool gives the possibility to merge two regions which merge create the entire set of states
                    if (reg_union->size() != num_states || generic) {
                        bool ec_and_pre_region = false;


                        for (auto record : *total_pre_regions_map) {
                            auto event = record.first;

                            /*if(Pre_and_post_regions_generator::is_pre_region(&ts_map->at(event), reg_union)){
                                //cout << "---" << endl;
                                cout << "preregione per " << event << endl;
                                println(*reg_union);
                                cout << "---" << endl;
                                cont_preregions++;
                            }*/

                            // cout << "event: " << event;
                            // auto tmp_set = new set<Region
                            // *>(total_pre_regions_map->at(event)->begin(),
                            //  total_pre_regions_map->at(event)->end());

                            auto tmp_set = new set<Region *>();

                            bool event_contains_reg = false;
                            for (auto reg : *total_pre_regions_map->at(event)) {
                                if (!are_equal(reg, reg1) && !are_equal(reg, reg2))
                                    tmp_set->insert(reg); //inserisco in temp_set tutte le regioni tranne quelle del merge
                                else
                                    event_contains_reg = true;
                            }
                            // insert the union if the event contains at least one of the two regions to merge
                            if (event_contains_reg) {
                                if (Pre_and_post_regions_generator::is_pre_region(&ts_map->at(event), reg_union)) {
                                    tmp_set->insert(reg_union);
                                    auto intersection = regions_intersection(tmp_set);
                                    auto new_er = ER_map->at(event);

                                    /* cout << "intersection:" << endl;
                                     println(*intersection);*/

                                    // cout << "er:" << endl;
                                    //println(*er);

                                    // check ec ER(ev)==intersec(prereg(ev))
                                    ec_and_pre_region = are_equal(intersection, new_er);
                                    //cout << "ec and pre region = " << ec_and_pre_region << endl;

                                    delete intersection;

                                    // try other two regions because for an event EC is not satisfied
                                    if (!ec_and_pre_region) {
                                        // cout << "BREAK the event does not satisfy EC, try other regions"
                                        //<< endl;
                                        delete tmp_set;
                                        delete reg_union;
                                        break;
                                    } else {
                                        delete (*tmp_map)[event];
                                        (*tmp_map)[event] = tmp_set;
                                        union_ptr = reg_union;
                                    }
                                } else {
                                    ec_and_pre_region = false;
                                    delete tmp_set;
                                    delete reg_union;
                                    break;
                                }
                            } else {
                                delete tmp_set;
                                // cout << "event:" << event << endl;
                                // cout <<  the event is ok because its pre-regions were not modified with this union"
                                //  << endl;
                            }
                        }

                        // if for all events the pair is ok, the merge is actually done
                        if (ec_and_pre_region) {
                            delete preregions_set;
                            if (print_step_by_step || decomposition_debug)
                                cout << "merging ok" << endl;
                            //println(*reg1);
                            //println(*reg2);

                            if(print_step_by_step || decomposition_debug){
                                cout << "Pre-regions after the fusion: "<< endl;
                                for (auto el : *tmp_map) {
                                    cout << "event: " << el.first << endl;
                                    for (auto r : *el.second) {
                                        println(*r);
                                    }
                                }
                                cout << "" << endl;
                            }
                            return tmp_map;
                        }
                    }
                    else{
                        delete reg_union;
                    }
                }
            } else {
                delete inter;
                // cout << "the regions are disjointed" << endl;
            }
        }
    }

    //the merge was not performed
    if(print_step_by_step || decomposition_debug){
        cout << "Any fusion done" << endl;
        cout << "" << endl;
    }

    for (auto el : *tmp_map) {
        delete el.second;
    }
    delete tmp_map;
    delete preregions_set;

    return nullptr;
}
