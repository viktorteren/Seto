//
// Created by ciuchino on 18/04/18.
//

#include "Utilities.h"

namespace Utilities {
    //Region = set<int> ->ritorna un insieme di stati
    set<int> *regions_union(vector<Region *> *vec) {
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region: *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int> *regions_union(set<Region *> *vec) {
        //cout << "region union" << endl;
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region: *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                //cout << "Stato: " << *it << endl;
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int>* regions_union(Region * first, Region* second) {
        auto all_states = new Region();
        int size;
        Region::iterator it;
        it = first->begin();
        size = static_cast<int>(first->size());
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }
        it = second->begin();
        size = static_cast<int>(second->size());
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }
        return all_states;
    }


    map<int, set<int>* > * do_regions_intersection(map<int, vector<Region> *> * regions){

        auto pre_regions_intersection=new map<int,set<int>*>;

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


        return pre_regions_intersection;
    }

    set<int> *regions_intersection(Region* first, Region* second){
        auto intersection = new set<int>();
        for (auto state: *first) {
            if (second->find(state) != second->end()) {//trovo lo stato (appartiene a entrambe)
                intersection->insert(state);
            }
        }
        return intersection;
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

    set<int>* region_difference(set<int>& first, set<int>& second){
        auto s = new set<int>();
        for(auto state: first){
            if(second.find(state) == second.end()){
                s->insert(state);
            }
        }
        return s;
    }

    vector<Region>* copy_map_to_vector(map<int, vector<Region> *>* map){
        auto input = new set<Region>();
        for(auto record: *map){
            for(const auto &region: *record.second){
                input->insert(region);
            }
        }
        vector<Region>* vec=new vector<Region>(input->size());
		std::copy(input->begin(), input->end(), vec->begin());
		delete input;
        return vec;
    }


    bool is_bigger_than(Region* region ,set<int>* region2){

        if(region->size() > region2->size()) {
            for (auto elem: *region2) {
                //nella regione non trovo un elem
                if (region->find(elem) == region->end()) {
                    cout << "FALSE**************" << endl;
                    return false;
                }
            }
        }
        else if(region->size() <= region2->size())
            return false;

        //nella regione trovo tutti gli stati della reg2
        cout<<"TRUE**************"<<endl;
        return true;
    }

}

