/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Essential_region_search.h"

Essential_regions_search::Essential_regions_search(map<int, set<Region *> *> *pre_reg) {
    pre_regions = pre_reg;
};

Essential_regions_search::~Essential_regions_search() {
    for (auto record: *pre_regions) {
        delete (*essential_map)[record.first];
    }
    delete essential_map;
}

/**
    * @brief
    * Algorithm:
    * for each even
    *      union of pre-regions of the given event to create a set with all states
    *      for each state
    *          check the pre-regions of the event not containing the given state
    *          if exists at least one region which does not have the given state then the region is essential
    * @return A vector of pointers to essential pre-regions
    */
set<Region *> *Essential_regions_search::search() {

    if(print_step_by_step)
        cout << "--------------------------------------------------- ESSENTIAL REGION SEARCH --------------------------------------------" << endl;


    set<int> *temp_union;

    auto essential_regions = new set<Region *>();
    essential_map = new map<int, set<Region *> *>();

    Region *last_essential_candidate;
    int counter;

    //cout << "num events: " << pre_regions->size() << endl;
    for (auto record: *pre_regions) {
        //cout <<  "event: " << record.first << endl;

        Region::iterator it2;

        //if I have only one region the region is surely essential
        if (record.second->size() == 1) {
            auto it = record.second->begin();
            essential_regions->insert(*it);
            if(print_step_by_step_debug){
                cout << "found essential region for event "<< record.first  <<": ";
                println(**it);
            }
        } else {
            //union of all pre-regions
            temp_union = regions_union(record.second);
            //cout << "union: ";
            //println(temp_union);

            //for each state of the union
            for (auto state: *temp_union) {
                counter = 0;
                for (auto region: *record.second) {
                    if (region->find(state) == region->end()) {
                        if (counter == 0) {
                            last_essential_candidate = region;
                            counter++;
                        } else {
                            counter = -1;
                            break;
                        }
                    }

                }
                //if I had only one candidate state to be essential then it is really essential
                if (counter == 1) {
                    if(print_step_by_step_debug){
                        cout << "found essential region for event "<< record.first  <<": ";
                        println(*last_essential_candidate);
                    }
                    essential_regions->insert(last_essential_candidate);
                }
            }
            delete temp_union;
        }

        //once all essential regions are found, the regions are saved in a map
        for (auto record2: *pre_regions) {
            for (auto region: *essential_regions) {
                auto set_of_event = record2.second;
                if (set_of_event->find(region) != set_of_event->end()) {
                    if (essential_map->find(record2.first) == essential_map->end()) {
                        (*essential_map)[record2.first] = new set<Region *>();
                    }
                    (*essential_map)[record2.first]->insert(region);
                }

            }
        }

    }

    if(print_step_by_step) {
        cout << "Essential regions: " << endl;
        for (auto reg: *essential_regions) {
            println(*reg);
        }
        cout << "" << endl;
    }



    if(print_step_by_step)
        cout << "num. essential regions: " << essential_regions->size() << endl;
    return essential_regions;
}


map<int, set<Region *> *> *Essential_regions_search::get_essential_regions_map() {
    if (essential_map == nullptr) {
        search();
    }
    return essential_map;
};



