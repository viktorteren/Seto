/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include <include/Pre_and_post_regions_generator.h>
#include "../include/GreedyRemoval.h"

using namespace Utilities;

void GreedyRemoval::minimize(set<set<Region *>*> *SMs, Pre_and_post_regions_generator *pprg, map<int, ER> *new_ER, map<int, set<Region *> *> *pre_regions){
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
            cout << "check if can rermove the SM: " << endl;
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
