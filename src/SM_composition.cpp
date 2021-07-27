/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/SM_composition.h"

#include <utility>

using namespace Utilities;

void SM_composition::compose(set<set<Region *>*> *sm_set,
                               map < set<Region *> *, map<int, Region*> * > *map_SM_pre_regions,
                               map < set<Region *> *, map<int, Region*> * > *map_SM_post_regions,
                               map<int, int> *aliases, string file_path){
    cout << "============ [ COMPOSITION ] ===========" << endl;
    set<map<set<Region *>*, Region *>> state_space; //set of maps where for each FCPN there is a set of current regions
    auto arcs = new vector<SM_edge>();
    map<set<Region *>*, Region *> current_map;
    for(auto SM: *sm_set){
        Region *initial_region;
        for(auto reg: *SM){
            if(reg->find(initial_state) != reg->end()){
                initial_region = reg;
                break;
            }
        }
        (current_map)[SM] = initial_region;
    }
    auto initial_state_TS = current_map;
    state_space.insert(current_map);

    set<map<set<Region *>*, Region *>> completely_explored_states;

    do {
        //given an event check if it can fire
        for(auto current_state: state_space) {
            if(completely_explored_states.find(current_state) == completely_explored_states.end()) {
                map<int, bool> firing;
                for(auto rec: *map_SM_pre_regions){
                    for(auto rec1: *rec.second){
                        (firing)[rec1.first] = true;
                    }
                }
                for (auto rec: *map_SM_pre_regions) {
                    auto FCPN = rec.first;
                    for (auto rec1: *rec.second) {
                        auto event = rec1.first;
                        if(firing.at(event)) {
                            if (current_state.at(FCPN) != rec1.second) {
                                firing.at(event) = false;
                            }
                        }
                    }
                }
                for (auto rec: firing) {
                    auto event = rec.first;
                    if (rec.second) {
                        map<set<Region *> *, Region *> next_state_map;
                        for (auto rec1: *map_SM_post_regions) {
                            auto FCPN = rec1.first;
                            //event take part of the FCPN
                            if (rec1.second->find(event) != rec1.second->end()) {
                                //insert into the next state the regions which were unchanged
                                // (didn't took part of the event firing)
                                if(map_SM_pre_regions->at(FCPN)->find(event) != map_SM_pre_regions->at(FCPN)->end()) {
                                    auto reg = current_state.at(FCPN);
                                    if (map_SM_pre_regions->at(FCPN)->at(event) != reg) {
                                        next_state_map[FCPN]=reg;
                                    }
                                }
                                else{
                                    cerr << "WARNING: events without pre-regions" << endl;
                                    //exit(1);
                                }

                                //region activated after the event firing
                                next_state_map[FCPN] = rec1.second->at(event);
                            } else {
                                next_state_map[FCPN] = current_state.at(FCPN);
                            }
                        }
                        if(current_state != next_state_map){
                            state_space.insert(next_state_map);
                            SM_edge arc;
                            arc.start = current_state;
                            if(event < num_events)
                                arc.event = (*aliases_map_number_name)[event];
                            else
                                arc.event = (*aliases_map_number_name)[aliases->at(event)];
                            arc.end = next_state_map;
                            arcs->push_back(arc);
                        }
                    }
                }
                completely_explored_states.insert(current_state);
            }
        }
    } while(state_space != completely_explored_states);

    auto state_aliases = new map <map<set<Region *>*, Region *>, int>();
    int cont = 0;
    for(const auto& state: state_space){
        (*state_aliases)[state] = cont;
        cont++;
    }

    if(dot_output){
        print_ts_dot_file(std::move(file_path), state_aliases,arcs, initial_state_TS);
    }
    else{
        print_ts_aut_file(std::move(file_path), state_aliases,arcs, initial_state_TS);
    }

    delete arcs;
    delete state_aliases;
}