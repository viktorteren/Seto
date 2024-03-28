/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Label_splitting_module.h"

Label_splitting_module::Label_splitting_module(
        map<int, set<Region*> *> *pre_regions, map<int, ES> *es_set, map<int, vector<Region *> *> *middle_set_of_states) {
    this->pre_regions = pre_regions;
    this->ES_set = es_set;
    this->middle_set_of_states=middle_set_of_states;
};

Label_splitting_module::~Label_splitting_module() {
    for (auto el : *regions_intersection_map) {
        delete el.second;
    }
    delete regions_intersection_map;

};

/**
 *
 * @return Events not satisfying EC, in this way label splitting can be performed only on these events. If EC is satisfied
 * nullptr is returned.
 */
set<int> *Label_splitting_module::is_excitation_closed() {

    regions_intersection_map = new map<int, set<int> *>() ;

    auto events_not_satisfy_EC = new set<int>;

    // for each event
    // if for an event it is true that the intersection is equal to ES, the TS is not excitation-closed
    // bool res=true;

    //for (auto item : *pre_regions) {
    //cout<<"num evens after splitting "<<num_events_after_splitting<<endl;
    for(int event=0;event<num_events_after_splitting;++event){
        //cout << "event: " << item.first;
        //auto event = item.first;
        auto es = ES_set->at((event));
        //the event does not have pre-regions therefore EC is not satisfied
        if(pre_regions->find(event)==pre_regions->end()){
            events_not_satisfy_EC->insert(event);
            (*regions_intersection_map)[event] = new set<int>();
        } else {
            auto intersec = regions_intersection(pre_regions->at(event));
            (*regions_intersection_map)[event] = intersec;
            //cout << "Intersec at" << event << " :" << endl;
            //println(*intersec);
            if (!(are_equal(es, intersec))) {
                // cout << "region of event:" << event;
                events_not_satisfy_EC->insert(event);
                // res=false;
            }
        }
    }

    //the map contains candidate regions only for the events which have these candidate regions!!!
     /*for (auto ev : *events_not_satisfy_EC) {
       cout << "event not sat EC----------" << ev << endl;
     }*/

     if(print_step_by_step){
        if(!events_not_satisfy_EC->empty())
            cout<<"Ts not excitation closed"<<endl << endl;
        else
            cout<<"Ts excitation closed"<<endl << endl;
     }

    return events_not_satisfy_EC;
}

map<int, pair<int,Region*>* > * Label_splitting_module::candidate_search(
        map<int, vector<int> *> *number_of_bad_events,
        set<int> *events_not_satisfy_EC){

    // for each event
    //      for each intermediate state
    //          if the state takes part of the set of intersections
    //              take the state for label splitting

    auto events_type = new vector<int>(middle_set_of_states->size());
    Region *candidate_region = nullptr;
    pair<int,Region*>* pair_reg= nullptr;

    int num_bad_event_min;
    auto candidate_regions = new map<int , pair<int,Region*>*>();

    //cout << "middle TOT: " << endl;
    vector<Region *>::iterator it;
    for (auto event : *events_not_satisfy_EC) {
        if(pair_reg != nullptr){
            delete pair_reg;
            pair_reg = nullptr;
        }


        //cout << "EVENT: " << event << "*************" << endl;

        int pos = 0;
        candidate_region = nullptr;
        num_bad_event_min = -1;

        //special case with empty intersection because there are no pre-regions, chose ES as candidate region
        if(regions_intersection_map->at(event)->empty()){
            candidate_region=*middle_set_of_states->at(event)->begin();
            pair_reg=new pair<int,Region*>(pos,candidate_region);
        }else {

            // auto set_forced_to_be_a_region;
            for (it = middle_set_of_states->at(event)->begin();
                 it < middle_set_of_states->at(event)->end(); ++it) {
                //cout << "middle:" << endl;
                //println(**it);

                if (is_bigger_than_or_equal_to(*it, regions_intersection_map->at(event))) {
                    // cout << "erase" << endl;
                    *middle_set_of_states->at(event)->erase(it, it);
                } else { // the state is ok
                    // cache and recalculate only if it is not available (break  was hit  because of no cross)
                    // the number of events in violation is taken for each set of states!!!
                    //cout << "event: " << event << endl;
                    auto vec_ptr = number_of_bad_events->at(event);
                    if ((*vec_ptr)[pos] == -1) {

                        // println(**it);
                        // recomputation
                        for (auto e : *middle_set_of_states)
                            (*events_type)[e.first] =
                                    branch_selection(&(ts_map->at(e.first)), *it);

                        //set_number_of_bad_events(events_type, event, vec_ptr, pos);
                        set_number_of_bad_events(events_type, vec_ptr, pos);
                    }

                    if ((*vec_ptr)[pos] != 0) {
                        if (candidate_region == nullptr ||
                            num_bad_event_min > (*vec_ptr)[pos]) {
                            candidate_region = *it;
                            delete pair_reg;
                            pair_reg = new pair<int, Region *>(pos, candidate_region);
                            num_bad_event_min = (*vec_ptr)[pos];

                        } else if (num_bad_event_min == (*vec_ptr)[pos]) {
                            // check based on the size of the two regions
                            if (candidate_region->size() >= (*it)->size()) {
                                candidate_region = *it;
                                delete pair_reg;
                                pair_reg = new pair<int, Region *>(pos, candidate_region);
                                num_bad_event_min = (*vec_ptr)[pos];
                            }
                        }
                    }
                    //cout << "NUMBER candidate: " << num_bad_event_min << endl;
                }
                pos++;
            }
        }

        //add candidate_region to the regions of the current event
        /*cout << "___________________________CANDIDATE REGION__________________"
             << endl;*/
        if (candidate_region != nullptr) {
            (*candidate_regions)[event]=new pair<int, Region *>(pair_reg->first, pair_reg->second);
        }
    }

    delete pair_reg;

    delete events_type;

    return candidate_regions;
}

int Label_splitting_module::branch_selection(Edges_list *list, Region *region) {
    // which branch has to be taken betwee nok, nocross or two branches? (for an event)
    vector<int> *trans = new vector<int>(4, 0);
    // cout << "INSIDE" << endl;

    // num in-out-exit-enter
    const int in = 0;
    const int out = 1;
    const int exit = 2;
    const int enter = 3;

    for (auto t : *list) {
        if (region->find(t->first) !=
            region->end()) { // the first state belongs to the region
            if (region->find(t->second) !=
                region->end()) { // also the second state belongs to the region
                (*trans)[in]++;
                //  cout << t->first << "->" << t->second << " IN " << endl;
                // for "nocross" is ok, the other branches cannot be chosen
            } else {
                (*trans)[exit]++;
                //  cout << t->first << "->" << t->second << " EXIT" << endl;
            }
        } else { // the first does not belong
            if (region->find(t->second) !=
                region->end()) { // the second state belongs to the region
                (*trans)[enter]++;
                // cout << t->first << "->" << t->second << " ENTER" << endl;
            } else {
                (*trans)[out]++;
                // cout << t->first << "->" << t->second << " OUT" << endl;
            }
        }
    }

    /*int it = 0;
     cout << ">> IN = 0/OUT = 1/EXIT = 2/ENTER = 3" << endl;
    for (auto i : *trans) {
      cout << "num trans " << it << ": " << i << endl;
      it++;
    }*/

    // enter + in should become nocross
    if (((*trans)[in] > 0 && (*trans)[enter] > 0) ||
        ((*trans)[in] > 0 && (*trans)[exit] > 0) ||
        ((*trans)[enter] > 0 && (*trans)[exit] > 0)) {
        //cout << "return no cross" << endl;
        delete trans;
        return NOCROSS;
    } else if ((*trans)[exit] > 0 && (*trans)[out] > 0) { //(exit-out)
        // cout << "return exit_no cross" << endl;
        delete trans;
        return EXIT_NOCROSS;
    } else if ((*trans)[enter] > 0 && (*trans)[out] > 0) { //(enter-out)
        // cout << "return enter_no cross" << endl;
        delete trans;
        return ENTER_NOCROSS;
    } else {
        //cout << "return ok" << endl;
        delete trans;
        return OK;
    }
}

void Label_splitting_module::set_number_of_bad_events(
                                                vector<int> *event_type, vector<int> *number_of_bad_events, int pos) {
    // count for each set of states the "bad" events

    int counter = 0;
    for (auto n : *event_type) {
        if (n != OK) {
            counter++;
        }
    }

    (*number_of_bad_events)[pos] = counter;
}

void Label_splitting_module::split_ts_map(map<int, pair<int, Region *> *> *candidate_regions,
                                          map<int, int> *event_alias,
                                          map<int, map<int, int> *> *event_violations,
                                          map<int, map<int, vector<Edge *> *> *> *trans_violations,
                                          map<int,vector<Region>*> *regions_old) {
    Region *best_region = nullptr;
    map<int, pair<int, Region *> *>::iterator it;

    int best_region_root_event = -1;
    int best_region_id;
    for (it = candidate_regions->begin(); it != candidate_regions->end(); ++it) {
        //cout<<"cand region"<<endl;
        //println(*(*it).second->second);
        if (best_region == nullptr) {
            best_region = (*it).second->second;
            best_region_root_event = (*it).first;
            best_region_id = (*it).second->first;
        } else {
            if (best_region->size() > (*it).second->second->size()) {
                best_region = (*it).second->second;
                best_region_root_event = (*it).first;
                best_region_id = (*it).second->first;
            }
        }
    }

    vector<Region*>* regions_vec;
    if(pre_regions->empty()){
        regions_vec = Utilities::copy_map_to_vector3(regions_old);
    }
    else{
        regions_vec = Utilities::copy_map_to_vector2(pre_regions);
    }
    vector<Region*>::iterator it2;
    for (it2 = regions_vec->begin(); it2 < regions_vec->end(); ++it2) {

        //for the event in violation
        // delete transitions in violation if the transitions involve the aforementioned event and create new ones
        // with the alias, otherwise nothing

        auto total_events = static_cast<int>(ts_map->size());
        //cout << "total events !!!: " << total_events << endl;
        auto event = (*event_violations)[best_region_root_event]->at(best_region_id);

        if (ts_map->find(total_events) == ts_map->end()) {
            (*ts_map)[total_events] = Edges_list();
        }

        // create alias and new transitions and after erase
        //ts size is the new event
        if (event_alias->find(event) != event_alias->end()) {
            (*event_alias)[total_events] = event_alias->at(event);
        } else {
            (*event_alias)[total_events] = event;
        }
        if(print_step_by_step){
            cout << "event " << event << " split into " << total_events << " and " << event << endl;
            cout<<""<< endl;
        }

        auto to_erase = new set<Edge *>();

        auto transitions = trans_violations->at(event)->at(best_region_id);

        for (auto tr: *transitions) {
            auto pair = new Edge();
            pair->first = tr->first;
            pair->second = tr->second;
            (*ts_map)[total_events].insert(pair);
            to_erase->insert(tr);
        }

        for (auto el : *to_erase) {
            ts_map->at(event).erase(el);
            delete el;
        }
        delete to_erase;
        break;
    }

    delete regions_vec;
}

