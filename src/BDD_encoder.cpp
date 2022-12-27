/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "include/BDD_encoder.h"
#include "libs/cudd-3.0.0/st/st.h"

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
    int approx_counter = 0;
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
                if(invalid_sets->at(event)->size() > 45){
                    cerr << "TOO MANY EVENTS FOR BDD EXECUTION" << endl;
                    exit(1);
                }
            }
            delete temp;
        }

        auto to_add_later = new set<set<Region *>>();
        auto cache = set<set<Region *>*>();
        do {
            for (const auto& reg_set: *to_add_later) {
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
                                        //cout << "to add later: \n";
                                        //println(temp_set);
                                    }
                                    delete temp_set;
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
        delete secondary_regions;
        //map_of_EC_clauses = dnf_to_cnf(valid_sets);
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
    for(auto cl: *clauses){
        delete cl;
    }
    delete clauses;
}

bool BDD_encoder::test_if_enough(ER er, set<Region *> *regions) {
    auto intersection = regions_intersection(regions);
    if(*er == *intersection){
        delete intersection;
        return true;
    }
    delete intersection;
    return false;
}

bool BDD_encoder::test_if_enough(ER er, Region *region) {
    if(*region == *er)
        return true;
    return false;
}

void BDD_encoder::encode(set<Region *> *regions, map<Region *, int> *regions_alias_mapping, map<int, set<set<int> *> *> *pre_regions){
    clauses = new vector<vector<int32_t> *>();
    Cudd mgr(0,0);
    //regions_bdd_map is a map containing a vector of bdd variables, one BDD for each FCPN
    map<Region *, BDD> regions_bdd_map;
    map<int, BDD> event_bdd_encodings;
    //creation of variables for each region
    for(auto reg: *regions){
        regions_bdd_map[reg]=mgr.bddVar();
    }

    if(decomposition_debug){
        cout << "number of regions: " << regions->size()<<endl;
        cout << "valid sets" << endl;
        for(auto rec: *valid_sets){
            cout << "ev: " << rec.first << endl;
            for(auto reg_set: *rec.second){
                println(reg_set);
                cout << "--" << endl;
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

    DdManager* manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    //the number assigned to the node is not the same of the region
    vector<DdNode *> inodes;

    map<Region *, DdNode *> region_inode_mapping;
    for(auto reg: *regions){
        auto tmpNode = Cudd_bddNewVar(manager);
        inodes.push_back(tmpNode);
        region_inode_mapping[reg] = tmpNode;
    }

    /*
      CODE EXAMPLE:
      x1 x2 | out
      0  0  |  1
      0  1  |  0
      1  0  |  1
      1  1  |  0

      out = (!x1 A !x2) V (x1 A !x2)
      x1 and x2 are inodes[0] e inodes[1]


    DdNode* first = Cudd_bddAnd(manager, Cudd_Not(inodes[0]), Cudd_Not(inodes[1]));
    DdNode* second = Cudd_bddAnd(manager, inodes[0], Cudd_Not(inodes[1]));
    DdNode* out = Cudd_bddOr(manager, first, second);

    DdNode* restrictBy = Cudd_bddAnd(manager, Cudd_Not(inodes[0]), Cudd_Not(inodes[1]));
    auto testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 0, x2 = 0, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

    restrictBy = Cudd_bddAnd(manager, Cudd_Not(inodes[0]), inodes[1]);
    testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 0, x2 = 1, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

    restrictBy = Cudd_bddAnd(manager, inodes[0], Cudd_Not(inodes[1]));
    testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 1, x2 = 0, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

    restrictBy = Cudd_bddAnd(manager, inodes[0], inodes[1]);
    testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 1, x2 = 1, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

     */

     /*
      * regions_bdd_map mappa le regioni e BDD da qui posso passare al DdNode e creare le restrizioni
      * quando creo le combinazioni di regioni in input il bello è che il restrictBy può essere fatto per ogni evento
      * o forse mi sto sbagliando, basterebbe limitarsi alle pre-regioni di quel evento quindi gli altri sono DC
      * sia nel restrict by che nella creazione delle clausole
      */

    for(int i=0; i < num_events_after_splitting; ++i){
        /*
        if(decomposition_debug){
            cout << "*event*: " << i << endl;
        }
         */
        auto regs = pre_regions->at(i); //regions' set
        auto regs_vector = new vector<Region *>(regs->begin(), regs->end());
        //here I have to create all binary combinations between these regions
        for(int counter = 0; counter < pow(2,regs->size());++counter){ //all possible combinations between [0, 2^(num event pre-regions) - 1]
            //encoding of counter into a vector
            auto tmp_vec = new vector<int>();
            int counter2 = counter;
            while(counter2 != 0){
                if(counter2 % 2 == 1){
                    tmp_vec->push_back(1);
                    counter2 = counter2 - 1;
                }
                else{
                    tmp_vec->push_back(0);
                }
                counter2 = counter2 / 2;
            }
            while(tmp_vec->size() < regs->size()){
                tmp_vec->push_back(0);
            }
            reverse(tmp_vec->begin(), tmp_vec->end());
            /*
            for(int s=0;s < tmp_vec->size();++s){
                cout << tmp_vec->at(s) << ", ";
            }
            cout << endl;*/

            //for each region of the vector encode the restriction
            vector<DdNode*> restrictByVector(tmp_vec->size());
            for(int pos=0;pos < tmp_vec->size();++pos){
                auto tmp_node = region_inode_mapping.at(regs_vector->at(pos));
                /*if(decomposition_debug){
                    cout << "pos: " << pos << endl;
                }*/
                if(pos == 0){
                    if (tmp_vec->at(pos) == 1) {
                        restrictByVector.at(pos) = tmp_node;
                        /*if(decomposition_debug)
                            cout << "value: true" << endl;*/
                    } else {
                        restrictByVector.at(pos) = Cudd_Not(tmp_node);
                        /*if(decomposition_debug)
                            cout << "value: false" << endl;*/
                    }
                }
                else {
                    if (tmp_vec->at(pos) == 1) {
                        restrictByVector.at(pos) = Cudd_bddAnd(manager, restrictByVector.at(pos - 1), tmp_node);
                        /*if(decomposition_debug)
                            cout << "value: true" << endl;*/
                    } else {
                        restrictByVector.at(pos) = Cudd_bddAnd(manager, restrictByVector.at((pos - 1)),
                                                               Cudd_Not(tmp_node));
                        /*if(decomposition_debug)
                            cout << "value: false" << endl;*/
                    }
                }
                Cudd_Ref(restrictByVector.at(pos));
            }
            DdNode *testEvent = Cudd_bddRestrict(manager, event_bdd_encodings[i].getNode(), restrictByVector.at(tmp_vec->size()-1));
            int result = (1 - Cudd_IsComplement(testEvent));

            //if the result is 0 then create a clause with all values inverted
            if(result == 0){
                /*if(decomposition_debug)
                    cout << "result: 0" << endl;*/
                auto clause = new vector<int32_t>();
                for(int pos=0;pos < tmp_vec->size();++pos){
                    int current_encoded_region = regions_alias_mapping->at(regs_vector->at(pos));
                    //creation of clauses from truth table where there result is 0
                    //in order to create CNFF clauses each variable sign is inverted
                    //example: vec[pos] == true -> encoded value will be negative (false)
                    if(tmp_vec->at(pos) == 1){
                        //1 is removed in this way region 0 can hae a negative value
                        clause->push_back(-current_encoded_region-1);
                    }
                    else{
                        //1 is added because ... see if branch
                        clause->push_back(current_encoded_region+1);
                    }
                }
                clauses->push_back(clause);
                if(decomposition_debug)
                    print_clause(clause);
            }
            delete tmp_vec;
        }
        delete regs_vector;
    }
    for(auto node: inodes){
        Cudd_RecursiveDeref(manager, node);
    }
}

vector<vector<int32_t> *> *BDD_encoder::get_clauses() {
    return clauses;
}
