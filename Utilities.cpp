//
// Created by ciuchino on 18/04/18.
//

#include "Utilities.h"

namespace Utilities {

    //Region = set<int> ->ritorna un insieme di stati
    set<int> regions_union(vector<Region *> *vec) {
        //cout << "region union" << endl;
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
        /*for (auto state: *all_states) {
            cout << "st: " << state << endl;
        }*/
        return *all_states;
    }

    set<int> regions_union(set<Region *> *vec) {
        //cout << "region union" << endl;
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
        /*for (auto state: *all_states) {
            cout << "st: " << state << endl;
        }*/
        return *all_states;
    }

    set<int> regions_union(Region * first, Region* second) {
        Region *all_states = new Region();
        int size;
        Region::iterator it;
        //for (Region *region: *vec) {
        //cout << "region with size: " << region->size() << endl;
        it = first->begin();
        size = first->size();
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }
        it = second->begin();
        size = second->size();
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }
        return *all_states;
    }


    map<int, set<int>* > * do_regions_intersection(map<int, vector<Region> *> * regions){

        map<int, set<int>* > *pre_regions_intersection=new map<int,set<int>*>;

        std::vector<Region>::iterator it;

        //per ogni evento
        for(auto item: *regions) {

            for(auto i: *item.second){
                cout<<"Preregion di " << item.first<<endl;
                for(auto pre: i)
                    cout<< "S: " << pre <<endl;
            }
            //se c'è più di una preregione
            if(item.second->size()>1) {
                //per ogni regione
                for (it = item.second->begin(); it != item.second->end(); ++it) {
                    //for (auto set_ptr: *item.second) {
                    auto set_ptr = &(*it); //puntatore al set
                    if (it == item.second->begin()) {
                        auto set_ptr2 = &(*(++it));
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
                for(auto state: (*item.second)[0]){
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


        cout<<"intersezione****************" <<endl;
        print(*(pre_regions_intersection->at(4)));

        return pre_regions_intersection;


    }

    set<int> regions_intersection(Region* first, Region* second){
        auto intersection = new set<int>();
        for (auto state: *first) {
            if (second->find(state) != second->end()) {//trovo lo stato (appartiene a entrambe)
                intersection->insert(state);
            }
        }
        return *intersection;
    }

    void print(Region& region){
        for(auto state : region){
            cout << state << ",";
        }
    }

    void println(Region& region){
        print(region);
        cout << endl;
    }

    set<int> region_difference(set<int>& first, set<int>& second){
        set<int> *s = new set<int>();
        for(auto state: first){
            if(second.find(state) == second.end()){
                s->insert(state);
            }
        }
        return *s;
    }

}

