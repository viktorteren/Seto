/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "include/BDD_encoding.h"

using namespace Utilities;

BDD_encoding::BDD_encoding(map<int, set<set<int> *> *> *pre_regions, map<int, ER> *ER_map, set<Region *> *regions) {
    valid_sets = new map<int, set<set<Region *>>*>();
    invalid_sets = new map<int, set<set<Region *>>*>();
    // [IN ITALIAN]
    // qui dovrei scrivere un algoritmo che:
    // 1) prende gli eventi uno ad uno
    // 2) per questo evento prende tutte le pre-regioni e l'ER
    // 3) cerca tutte le combinazioni (minimali) di pre-regioni per soddisfare l'EC per tale evento
    // 4) potrei usare un codice ricorsivo: partendo da ogni regione, l'ordine ottimale sarebbe però
    // quello che segue una BFS e non DFS: prima controllo le singole regioni, poi coppie ...
    // Quindi scorro per ogni regione e faccio il controllo, salvando i vincitori nella cache
    // una volto scorso per tutti aggiungo solo quelli che sono maggiori dei precedenti e non sono nella
    // cache come vincenti
    for(auto rec: *pre_regions){
        auto event = rec.first;
        cout << "event: " << event << endl;
        (*valid_sets)[event] = new set<set<Region *>>();
        (*invalid_sets)[event] = new set<set<Region *>>();
        auto pre_region_set_for_event = rec.second;
        auto event_ER = ER_map->at(event);
        auto secondary_regions = new vector<Region *>(); //regions that cannot satisfy EC alone
        for(auto pre_region: *pre_region_set_for_event){
            bool enough = test_if_enough(event_ER,pre_region);
            auto temp = new set<Region *>();
            temp->insert(pre_region);
            if(enough){
                valid_sets->at(event)->insert(*temp);
            }
            else{
                invalid_sets->at(event)->insert(*temp);
                secondary_regions->push_back(pre_region);
            }
        }
        auto to_add_later = new set<set<Region *>*>();
        do {
            for(auto reg_set: *to_add_later){
                invalid_sets->at(event)->insert(*reg_set);
            }
            to_add_later->clear();
            for (const auto& reg_set: *invalid_sets->at(event)) {
                for (auto reg: *secondary_regions) {
                    if (!contains(reg_set, reg)) {
                        auto temp_set = new set<Region *>(reg_set.begin(), reg_set.end());
                        temp_set->insert(reg);
                        auto tep = invalid_sets->at(event);
                        //if temp_set is already in invalide_set it means that it was already analyzed
                        if(!contains(invalid_sets->at(event), temp_set)){
                            bool contains_a_valid_set = false;
                            //if temp_set contains at least one of the sets of valid set it means that temp_set is
                            // redundant
                            for (const auto& valid_set: *valid_sets->at(event)) {
                                if (contains(*temp_set, valid_set)) {
                                    contains_a_valid_set = true;
                                    break;
                                }
                            }
                            if (!contains_a_valid_set) {
                                bool enough = test_if_enough(event_ER, temp_set);
                                if (enough) {
                                    valid_sets->at(event)->insert(*temp_set);
                                } else {
                                    to_add_later->insert(temp_set);
                                    cout << "to add later: ";
                                    println(temp_set);
                                }
                            }
                        }
                    }
                }
            }
        }while(!to_add_later->empty());
    }
    cout << "here " << endl;

    encode(regions);
}

BDD_encoding::~BDD_encoding() {
    for(auto rec: *valid_sets){
        delete rec.second;
    }
    delete valid_sets;
    for(auto rec: *invalid_sets){
        delete rec.second;
    }
    delete invalid_sets;
}

bool BDD_encoding::test_if_enough(ER er, set<Region *> *regions) {
    auto intersection = regions_intersection(regions);
    if(*er == *intersection){
        return true;
    }
    return false;
}

bool BDD_encoding::test_if_enough(ER er, Region *region) {
    if(*region == *er)
        return true;
    return false;
}

void BDD_encoding::encode(set<Region *> *regions){
    Cudd mgr(0,0);
    auto event_bdds = new vector<BDD>(num_events);
    map<Region *, BDD> regions_bdd_map;
    map<int, BDD> event_bdd_encodings;
    for(auto reg: *regions){
        regions_bdd_map[reg] = mgr.bddVar();
    }

    for(int i=0;i<num_events;++i){
        event_bdd_encodings[i] = mgr.bddZero();
        auto current_set = valid_sets->at(i);
        for(const auto& combination: *current_set){
            BDD current = mgr.bddOne();
            for(auto reg: combination){
                current *= regions_bdd_map[reg];
            }
            event_bdd_encodings[i] += current;
        }
    }

    auto inames = new string();
    for(int i=0;i<regions->size();++i){
        string tmp = "r";
        tmp.append(to_string(i));
        //inames->push_back(tmp);
    }


    //codice di esempio
    /*
    Cudd mgr(0,0);
    BDD x = mgr.bddVar();
    BDD y = mgr.bddVar();
    BDD f = x * y;
    BDD g = y + !x;
    char const* inames[] = {"x", "y"};
    char const* onames[] = {"f", "g"};
    DdNode *Dds[] = {f.getNode(), g.getNode()};
    FILE* fp = fopen("graph.dot", "w");
    Cudd_DumpDot(mgr.getManager(), 2, Dds, (char**) inames, (char**) onames, fp);
     */
}
