/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/FCPN_composition.h"

using namespace Utilities;

void FCPN_composition::compose(set<set<Region *>*> *fcpn_set,
                               map < set<Region *> *, map<int, set<Region*> *> * > *map_FCPN_pre_regions,
                               map < set<Region *> *, map<int, set<Region*> *> * > *map_FCPN_post_regions){
    set<map<set<Region *>*, set<Region *>>> state_space; //set of maps where for each FCPN there is a set of current regions
    vector<edge> arcs;
    map<set<Region *>*, set<Region *>> current_map;
    for(auto FCPN: *fcpn_set){
        set<Region *> initial_regions;
        for(auto reg: *FCPN){
            if(reg->find(initial_state) != reg->end()){
                initial_regions.insert(reg);
            }
        }
        (current_map)[FCPN] = initial_regions;
    }
    state_space.insert(current_map);

    set<map<set<Region *>*, set<Region *>>> completely_explored_states;

    do {
        //1)dato una regione la regione è pre-regione per 'e'
        //2)se 'e' contiene più pre-regioni tutte devono far parte dello stato attuale
        //3)se 'e' fa parte di un'altra FCPN anche tutte le pre-regioni di 'e' in quella FCPN devono far parte dello
        //stato attuale
        //4)crea arco
        //5)aggiungi la destinazione nell'insieme degli stati
        //6)avendo controllato tutti i possibili scatti aggiungi lo stato attuale a quelli esplorati

        //different prospettive: given an event check if it can fire
        for(auto current_state: state_space) {
            if(completely_explored_states.find(current_state) == completely_explored_states.end()) {
                map<int, bool> firing;
                for(auto rec: *map_FCPN_pre_regions){
                    for(auto rec1: *rec.second){
                        (firing)[rec1.first] = true;
                    }
                }
                for (auto rec: *map_FCPN_pre_regions) {
                    auto FCPN = rec.first;
                    for (auto rec1: *rec.second) {
                        auto event = rec1.first;
                        if(firing.at(event)) {
                            //todo qui bug da sistemare
                            if (!contains(current_state.at(FCPN), *rec1.second)) {
                                firing.at(event) = false;
                            }
                        }
                    }
                }
                for (auto rec: firing) {
                    auto event = rec.first;
                    if (rec.second) {
                        map<set<Region *> *, set<Region *>> next_state_map;
                        for (auto rec1: *map_FCPN_post_regions) {
                            auto FCPN = rec1.first;
                            //event take part of the FCPN
                            if (rec1.second->find(event) != rec1.second->end()) {
                                for (auto reg: *rec1.second->at(event)) {
                                    next_state_map[FCPN].insert(reg);
                                }
                            } else {
                                next_state_map.at(FCPN) = current_state.at(FCPN);
                            }
                        }
                        if(current_state != next_state_map){
                            state_space.insert(next_state_map);
                            edge arc;
                            arc.start = current_state;
                            arc.event = event;
                            arc.end = next_state_map;
                            arcs.push_back(arc);
                        }
                    }
                }
                completely_explored_states.insert(current_state);
            }
        }
    } while(state_space != completely_explored_states);

    //todo: creare una mappa tra stati ed interi univoci che verranno usati per l'output

}