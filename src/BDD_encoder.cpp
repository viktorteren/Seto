/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "include/BDD_encoder.h"

using namespace Utilities;

//complexity (worst case): num_events*num_regions!
BDD_encoder::BDD_encoder(map<int, set<set<int> *> *> *pre_regions, map<int, ER> *ER_map) {
    //TODO: implement a mechanism which stops without having explored all possible EC combinations
    // a possible result could be a normal computation for n cycles and at this point a search for only events without
    // any set of regions for their EC
    cout << "==============[BDD ENCODER]===============" << endl;
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
    // cache come vincenti e non contengono un vincente (significa che sono minimali)
    for(auto rec: *pre_regions){
        auto event = rec.first;
        //cout << "event: " << event << endl;
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
                /*cout << "valid sets: \n";
                for(auto rec: *valid_sets){
                    cout << "new set: \n";
                    auto tmp = rec.second;
                    for(auto tmp2: *tmp){
                        println(tmp2);
                    }
                }*/
            }
            else{
                invalid_sets->at(event)->insert(*temp);
                secondary_regions->push_back(pre_region);
            }
        }
        //cout << "Secondary regions size: " << secondary_regions->size() << endl;
        if(valid_sets->at(event)->empty()) {
            //int cycle_counter = 0;
            auto to_add_later = new set<set<Region *>>();
            auto cache = set<set<Region *>*>();
            do {
                //cycle_counter++;
                for (auto reg_set: *to_add_later) {
                    invalid_sets->at(event)->insert(reg_set);
                }
                to_add_later->clear();
                for (const auto &reg_set: *invalid_sets->at(event)) {
                    for (auto reg: *secondary_regions) {
                        if (!contains(reg_set, reg)) {
                            auto temp_set = new set<Region *>(reg_set.begin(), reg_set.end());
                            temp_set->insert(reg);
                            if(cache.find(temp_set) == cache.end()) {
                                cache.insert(temp_set);
                                //if temp_set is already in invalid_set it means that it was already analyzed
                                if (!contains(invalid_sets->at(event), temp_set)) {
                                    bool contains_a_valid_set = false;
                                    //if temp_set contains at least one of the sets of valid_set it means that temp_set is
                                    // redundant
                                    for (const auto &valid_set: *valid_sets->at(event)) {
                                        if (contains(*temp_set, valid_set)) {
                                            contains_a_valid_set = true;
                                            break;
                                        }
                                    }
                                    if (!contains_a_valid_set) {
                                        bool enough = test_if_enough(event_ER, temp_set);
                                        if (enough) {
                                            valid_sets->at(event)->insert(*temp_set);
                                            /*cout << "valid sets: \n";
                                            for(auto rec: *valid_sets){
                                                cout << "new set: \n";
                                                auto tmp = rec.second;
                                                for(auto tmp2: *tmp){
                                                    println(tmp2);
                                                }
                                            }*/
                                        } else {
                                            to_add_later->insert(*temp_set);
                                            //approximation in case of very big benchmarks -> improves performance approximating the result
                                            /*if (cycle_counter > 2 && !valid_sets->at(event)->empty()) {
                                                //don't add anything
                                                delete temp_set;
                                                //cout << "CUT" << endl;
                                            } else {
                                                to_add_later->insert(*temp_set);
                                            }*/
                                            //cout << "to add later: \n";
                                            //println(temp_set);
                                        }
                                    } else {
                                        delete temp_set;
                                    }
                                } else {
                                    delete temp_set;
                                }
                            }
                            else{
                                //cout << "CACHE HIT!!!" << endl;
                            }
                        }
                    }
                }
            } while (!to_add_later->empty());
            delete to_add_later;
        }
        delete secondary_regions;
    }
}

BDD_encoder::~BDD_encoder() {
    for(auto rec: *valid_sets){
        delete rec.second;
    }
    delete valid_sets;
    for(auto rec: *invalid_sets){
        delete rec.second;
    }
    delete invalid_sets;
    delete map_of_EC_clauses;
}

bool BDD_encoder::test_if_enough(ER er, set<Region *> *regions) {
    auto intersection = regions_intersection(regions);
    if(*er == *intersection){
        return true;
    }
    return false;
}

bool BDD_encoder::test_if_enough(ER er, Region *region) {
    if(*region == *er)
        return true;
    return false;
}

void BDD_encoder::encode(set<Region *> *regions){
    Cudd mgr(0,0);
    //regions_bdd_map is a map containing a vector of bdd variables, one BDD for each FCPN
    map<Region *, BDD> regions_bdd_map;
    map<int, BDD> event_bdd_encodings;
    //creation of variables for each region
    for(auto reg: *regions){
        regions_bdd_map[reg]=mgr.bddVar();
    }


    if(decomposition_debug){
        cout << "valid sets" << endl;
        for(auto rec: *valid_sets){
            cout << "ev: " << rec.first << endl;
            for(auto reg_set: *rec.second){
                println(reg_set);
            }
        }
    }



    //BDD creation
    for(int i=0; i < num_events_after_splitting; ++i){
        event_bdd_encodings[i] = mgr.bddZero();
        auto current_set = valid_sets->at(i);
        for(const auto& combination: *current_set){
            BDD current = mgr.bddOne();
            for (auto reg: combination) {
                current *= regions_bdd_map[reg];
            }
            event_bdd_encodings[i] += current;
        }
    }

    char const* inames[regions->size()];
    char const* onames[num_events_after_splitting];
    for (int k=0;k < regions->size();++k) {
        auto tmp = new string();
        *tmp = "r";
        tmp->append(to_string(k));
        inames[k] = tmp->c_str();
    }


    //assign for each value of onames a char array with inside the integer of the position
    for(int i=0; i < num_events_after_splitting; ++i){
        string tmp;
        tmp.append(to_string(i));
        onames[i] = tmp.c_str();
    }

    /*
    for(auto str: inames){
        cout << str << endl;
    }

    for(auto str: onames){
        cout << str << endl;
    }*/

    DdNode *Dds[num_events_after_splitting];
    for(int i=0; i < num_events_after_splitting; ++i){
        Dds[i] = event_bdd_encodings.at(i).getNode();
    }
    FILE* fp = fopen("graph.dot", "w");
    Cudd_DumpDot(mgr.getManager(), num_events_after_splitting, Dds, (char**) inames, (char**) onames, fp);

    fclose(fp);
    //cout << "ok" << endl;

    char const *inames_without_r[regions->size()];

    //removing from inames r, leaving only the index
    for(int i=0;i< regions->size();++i){
        string tmp = inames[i];
        std::size_t pos = 1;
        auto tmp2 = tmp.substr(pos);
        char* tmp3=new char();
        strcpy(tmp3,tmp2.c_str());
        inames_without_r[i] = tmp3;
    }

    //code for clause creation and dump on file
    FILE* fpb = fopen("BDD_clauses.txt", "w");
    for(int i=0;i<event_bdd_encodings.size();++i) {
        event_bdd_encodings[i].PrintTwoLiteralClauses((char **) inames_without_r, fpb);
    }

    fclose(fpb);

    //reading elements from file:
    string inFile = "BDD_clauses.txt";
    ifstream fin(inFile);
    //TODO: scrivere i file temporanei in memoria e non su disco
    string tmp, last;
    set<set<int>> *clause_set;
    clause_set = new set<set<int>>();
    set<int> tmp_set;
    vector<string> data;
    while(true){
        if((last == tmp) && (!last.empty())) {
            //clause_set.insert(tmp_set);
            break;
        }
        last = tmp;
        fin >> tmp;
        data.push_back(tmp);
        //cout << tmp << endl;
    }


    for(int i=0;i < data.size();++i){
        if(i == data.size()-1){
            tmp_set.insert(stoi(data[i]));
            clause_set->insert(tmp_set);
            //cout << "adding entire clause" << endl;
            break;
        }
        else if(i == 0){
            tmp_set.insert(stoi(data[i]));
            //cout << "adding" << endl;
            //cout << "adding " << data[i] << endl;
        }
        else {
            if(data[i] != "|"){
                if(data[i-1] != "|"){
                    clause_set->insert(tmp_set);
                    tmp_set.clear();
                    tmp_set.insert(stoi(data[i]));
                }
                else{
                    tmp_set.insert(stoi(data[i]));
                }
            }
            else{

            }
        }
    }

    /*
    for(auto s: *clause_set){
        println(s);
    }*/

    map_of_EC_clauses = clause_set;

    for(auto val: inames_without_r){
        delete val;
    }
}

set<set<int>> *BDD_encoder::getMapOfECClaues() {
    return map_of_EC_clauses;
}
