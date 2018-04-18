//
// Created by ciuchino on 18/04/18.
//

#include "Utilities.h"

namespace Utilities {

//Region = set<int> ->ritorna un insieme di stati
    set<int> regions_union(vector<Region *> *vec) {
        cout << "region union" << endl;
        Region *all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region: *vec) {
            //cout << "region with size: " << region->size() << endl;
            it = region->begin();
            size = region->size();
            for (int i = 0; i < size; ++i) {
                //cout << "Stato: " << *it << endl;
                all_states->insert(*it);
                ++it;
            }
            //cout << "region size: " << region-> size() << endl;
        }
        //controllo per debug
        //cout << "unione: " << endl;
        for (auto state: *all_states) {
            cout << "st: " << state << endl;
        }
        return *all_states;
    }


    map<int, set<int>* > * regions_intersection(map<int, vector<Region*> *> * pre_regions){

        map<int, set<int>* > *pre_regions_intersection=new map<int,set<int>*>;

        std::vector<Region*>::iterator it;

        //per ogni evento
        for(auto item: *pre_regions) {

            for(auto i: *item.second){
                cout<<"Preregion di " << item.first<<endl;
                for(auto pre: *i)
                    cout<< "S: " << pre <<endl;
            }
            //se c'è più di una preregione
            if(item.second->size()>1) {
                //per ogni preregione
                for (it = item.second->begin(); it != item.second->end(); ++it) {
                    //for (auto set_ptr: *item.second) {
                    auto set_ptr = *it; //puntatore al set
                    if (it == item.second->begin()) {
                        auto set_ptr2 = *(++it);
                        //cout << "setprt" << set_ptr << endl;
                        for (auto state: *set_ptr) {
                            if (set_ptr2->find(state) != set_ptr2->end()) {//trovo lo stato (appartiene a entrambe)
                                if (pre_regions_intersection->find(item.first) == pre_regions_intersection->end()) {
                                    (*pre_regions_intersection)[item.first] = new set<int>();
                                }
                                pre_regions_intersection->at(item.first)->insert(state);
                            }
                        }
                    } else {
                        cout << "else" << endl;
                        for (auto state: *set_ptr) {
                            if (pre_regions_intersection->find(state) !=
                                pre_regions_intersection->end()) {//trovo lo stato (appartiene a entrambe)
                                //aggiungo la regione alla mappa
                                if (pre_regions_intersection->find(item.first) == pre_regions_intersection->end()) {
                                    (*pre_regions_intersection)[item.first] = new set<int>();
                                }
                                pre_regions_intersection->at(item.first)->insert(state);
                            }
                        }
                    }

                    for (auto el: *pre_regions_intersection->at(item.first)) {
                        cout << "INTERSEZIONE: " << el << endl;
                    }
                }
            }
                //c'è solo una preregione tutti gli stati appartengono all'intersezione
            else {
                for(auto state: *((*item.second)[0])){
                    if (pre_regions_intersection->find(item.first) == pre_regions_intersection->end()) {
                        (*pre_regions_intersection)[item.first] = new set<int>();
                    }
                    pre_regions_intersection->at(item.first)->insert(state);
                }

                for (auto el: *pre_regions_intersection->at(item.first)) {
                    cout << "INTERSEZIONE: " << el << endl;
                }
            }
        }

        return pre_regions_intersection;


    }

}

