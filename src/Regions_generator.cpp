/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Regions_generator.h"

using namespace Utilities;

Region_generator::Region_generator(int n) : number_of_events(n) {
    ES_set = new map<int, ES>;
    regions = new map<int, vector<Region> *>;
    queue_temp_regions = new vector<Region>;
    map_states_to_add = new map<int, Branches_states_to_add *>();
    middle_set_of_states = new map<int, vector<Region *> *>;
    number_of_bad_events = new map<int, vector<int> *>();
    event_violations = new map<int, map<int , int >*>();
    trans_violations = new map<int, map<int, vector<Edge*>* >*>();

    max_state_id_ = 0;
    for (const auto& kv : *ts_map) {
        for (const auto* e : kv.second) {
            if (e->first > max_state_id_) max_state_id_ = e->first;
            if (e->second > max_state_id_) max_state_id_ = e->second;
        }
    }
    if (max_state_id_ >= 0 && max_state_id_ <= 1000000) {
        region_membership_.assign(static_cast<size_t>(max_state_id_ + 1), 0);
    }
    event_types_buf_.assign(static_cast<size_t>(number_of_events), -1);
}

void Region_generator::delete_ER_set(){
    for(auto rec: *ES_set){
        delete rec.second;
    }
    delete ES_set;
}

__attribute__((unused)) void Region_generator::delete_ES_set_not_in(map<int, ES> *es){
    set<ES> vec;
    for(auto rec: *es){
        vec.insert(rec.second);
    }
    for(auto rec: *ES_set){
        if(vec.find(rec.second) == vec.end())
            delete rec.second;
    }
    delete ES_set;
}

void Region_generator::delete_regions_map(){
    for(auto rec: *regions){
        delete rec.second;
    }
    delete regions;
}

Region_generator::~Region_generator() {
    delete_regions_map();
}

void Region_generator::basic_delete(){
    for (auto record : *map_states_to_add) {
        delete record.second->states_to_add_nocross;
        delete record.second->states_to_add_exit_or_enter;
        delete record.second;
    }
    delete map_states_to_add;
    delete queue_temp_regions;

    for (auto el : *middle_set_of_states) {
        delete el.second;
    }
    delete middle_set_of_states;

    for (auto elem : *number_of_bad_events) {
        delete elem.second;
    }
    delete number_of_bad_events;
    for (auto elem : *trans_violations) {
        for(auto elem2: *elem.second){
            delete elem2.second;
        }
        delete elem.second;
    }
    delete trans_violations;

    for (auto elem : *event_violations) {
        delete elem.second;
    }
    delete event_violations;
}

/**
 * @param new_region Region to compare
 * @param regions_vector Vector containing the regions from which regions bigger than new_region can be removed
 * @return true if at least one region was removed from regions_vector, otherwise false
 */
void Region_generator::prune_non_minimal_regions(vector<Region> *regions_vector) {
    if (!regions_vector || regions_vector->size() <= 1) return;

    // Sort by size ascending, then lexicographically, and deduplicate
    std::sort(regions_vector->begin(), regions_vector->end(),
              [](const Region &a, const Region &b) {
                  if (a.size() != b.size()) return a.size() < b.size();
                  return a < b;
              });
    regions_vector->erase(std::unique(regions_vector->begin(), regions_vector->end()),
                          regions_vector->end());

    const size_t n = regions_vector->size();
    std::vector<uint8_t> removed(n, 0);

    for (size_t i = 0; i < n; ++i) {
        if (removed[i] || (*regions_vector)[i].empty()) continue;
        const Region &small_reg = (*regions_vector)[i];
        const size_t small_sz = small_reg.size();
        for (size_t j = i + 1; j < n; ++j) {
            if (removed[j]) continue;
            const Region &big_reg = (*regions_vector)[j];
            if (big_reg.size() <= small_sz) continue;
            if (std::includes(big_reg.begin(), big_reg.end(), small_reg.begin(), small_reg.end())) {
                removed[j] = 1;
            }
        }
    }

    size_t write_idx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (!removed[i]) {
            if (write_idx != i) {
                (*regions_vector)[write_idx] = std::move((*regions_vector)[i]);
            }
            ++write_idx;
        }
    }
    regions_vector->resize(write_idx);
}

bool Region_generator::remove_bigger_regions(Region &new_region, vector<Region> *regions_vector) {
    bool removed_something = false;
    unsigned int cont;
    Region region;

    for (unsigned int i = 0; i < regions_vector->size(); i++) {
        region = regions_vector->at(i);
        cont = 0;
        if (region.size() > new_region.size()) {
            for (auto state : new_region) {
                if (region.find(state) == region.end()) {
                    break;
                } else {
                    cont++;
                }
            }
            if (cont == new_region.size()  && !new_region.empty()) {
                regions_vector->erase(regions_vector->begin() + i);
                i--;
                removed_something = true;
            }
        }
    }
    return removed_something;
}

/**
 *
 * @return Map with ESs of each event
 */
map<int, ES> *Region_generator::get_ES_set() { return ES_set; }

int Region_generator::branch_selection(Edges_list *list, Region *region,
                                       int event,int region_id_position) {
    int trans[4] = {0, 0, 0, 0};
    constexpr int in = 0;
    constexpr int out = 1;
    constexpr int exit = 2;
    constexpr int enter = 3;

    const bool use_bitmap = !region_membership_.empty();
    auto in_region = [&](int s) -> bool {
        if (use_bitmap && s >= 0 && static_cast<size_t>(s) < region_membership_.size()) {
            return region_membership_[static_cast<size_t>(s)] != 0;
        }
        return region->find(s) != region->end();
    };

    for (auto t : *list) {
        const bool s1 = in_region(t->first);
        const bool s2 = in_region(t->second);
        if (s1) {
            trans[s2 ? in : exit]++;
        } else {
            trans[s2 ? enter : out]++;
        }
    }

    const bool is_nocross = (trans[in] > 0 && trans[enter] > 0) ||
                            (trans[in] > 0 && trans[exit] > 0) ||
                            (trans[enter] > 0 && trans[exit] > 0);
    const bool is_exit_nocross = !is_nocross && (trans[exit] > 0 && trans[out] > 0);
    const bool is_enter_nocross = !is_nocross && !is_exit_nocross && (trans[enter] > 0 && trans[out] > 0);

    if (!is_nocross && !is_exit_nocross && !is_enter_nocross) {
        return OK;
    }

    states_to_add_enter = new set<int>;
    states_to_add_exit = new set<int>;
    states_to_add_nocross = new set<int>;

    auto enter_tr = new vector<Edge*>();
    auto exit_tr = new vector<Edge*>();
    auto out_tr = new vector<Edge*>();

    bool out_add = false;
    bool exit_add = false;
    bool enter_add = false;

    for (auto t : *list) {
        if (in_region(t->first)) {
            if (!in_region(t->second)) {
                exit_tr->push_back(t);
                states_to_add_nocross->insert(t->second);
            }
        } else {
            if (in_region(t->second)) {
                enter_tr->push_back(t);
                states_to_add_nocross->insert(t->first);
            } else {
                out_tr->push_back(t);
                states_to_add_enter->insert(t->second);
                states_to_add_exit->insert(t->first);
            }
        }
    }

    bool map_states_to_add_contains_event = (*map_states_to_add).find(event) != map_states_to_add->end();

    if (is_nocross) {
        if(trans_violations->find(event) == trans_violations->end())
            (*trans_violations)[event] = new map< int, vector<Edge*>* >();

        if(trans_violations->at(event)->find(region_id_position)!=trans_violations->at(event)->end()){
            delete (*(*trans_violations)[event])[region_id_position];
        }

        if ((trans[in] > 0 && trans[enter] > 0)) {
            (*(*trans_violations)[event])[region_id_position] = enter_tr;
            enter_add=true;
        }
        else if(trans[in] > 0 && trans[exit] > 0) {
            (*(*trans_violations)[event])[region_id_position] = exit_tr;
            exit_add=true;
        }
        else if (trans[enter] > 0 && trans[exit] > 0){
            enter_add=true;
            (*(*trans_violations)[event])[region_id_position] = enter_tr;
        }

        if(!out_add) delete out_tr;
        if(!exit_add) delete exit_tr;
        if(!enter_add) delete enter_tr;

        if (map_states_to_add_contains_event &&
            (*map_states_to_add)[event]->states_to_add_nocross != nullptr)
            delete (*map_states_to_add)[event]->states_to_add_nocross;
        if (map_states_to_add_contains_event &&
            (*map_states_to_add)[event]->states_to_add_exit_or_enter)
            delete (*map_states_to_add)[event]->states_to_add_exit_or_enter;
        if (map_states_to_add_contains_event)
            delete (*map_states_to_add)[event];
        (*map_states_to_add)[event] = new Branches_states_to_add();
        map_states_to_add->at(event)->states_to_add_nocross = states_to_add_nocross;

        delete states_to_add_enter;
        delete states_to_add_exit;
        return NOCROSS;
    } else if (is_exit_nocross) {
        if(trans_violations->find(event) == trans_violations->end())
            (*trans_violations)[event] = new map< int, vector<Edge*>* >();

        if(trans_violations->at(event)->find(region_id_position)!=trans_violations->at(event)->end()){
            delete (*(*trans_violations)[event])[region_id_position];
        }

        (*(*trans_violations)[event])[region_id_position] = out_tr;

        delete exit_tr;
        delete enter_tr;

        if (map_states_to_add_contains_event && (*map_states_to_add)[event]->states_to_add_nocross != nullptr)
            delete (*map_states_to_add)[event]->states_to_add_nocross;

        if (map_states_to_add_contains_event && (*map_states_to_add)[event]->states_to_add_exit_or_enter)
            delete (*map_states_to_add)[event]->states_to_add_exit_or_enter;

        if (map_states_to_add_contains_event)
            delete (*map_states_to_add)[event];
        (*map_states_to_add)[event] = new Branches_states_to_add();
        map_states_to_add->at(event)->states_to_add_exit_or_enter = states_to_add_exit;
        map_states_to_add->at(event)->states_to_add_nocross = states_to_add_nocross;

        delete states_to_add_enter;

        return EXIT_NOCROSS;
    } else {
        if(trans_violations->find(event) == trans_violations->end())
            (*trans_violations)[event] = new map< int, vector<Edge*>* >();

       if(trans_violations->at(event)->find(region_id_position)!=trans_violations->at(event)->end()){
            delete (*(*trans_violations)[event])[region_id_position];
        }
        (*(*trans_violations)[event])[region_id_position] = out_tr;
        delete exit_tr;
        delete enter_tr;

        if (map_states_to_add_contains_event &&
            (*map_states_to_add)[event]->states_to_add_nocross != nullptr)
            delete (*map_states_to_add)[event]->states_to_add_nocross;

        if (map_states_to_add_contains_event &&
            (*map_states_to_add)[event]->states_to_add_exit_or_enter)
            delete (*map_states_to_add)[event]->states_to_add_exit_or_enter;

        if (map_states_to_add_contains_event)
            delete (*map_states_to_add)[event];
        (*map_states_to_add)[event] = new Branches_states_to_add();
        map_states_to_add->at(event)->states_to_add_exit_or_enter =
                states_to_add_enter;
        map_states_to_add->at(event)->states_to_add_nocross = states_to_add_nocross;

        delete states_to_add_exit;

        return ENTER_NOCROSS;
    }
}

bool Region_generator::region_in_queue(Region &new_region, int init_pos) {
    for (auto it = queue_temp_regions->begin() + init_pos; it != queue_temp_regions->end(); ++it) {
        if (*it == new_region) {
            return true;
        }
    }
    return false;
}

void Region_generator::expand(Region *region, int event, bool is_ER, int init_pos, int region_id_position) {
    if (static_cast<int>(event_types_buf_.size()) < number_of_events) {
        event_types_buf_.resize(static_cast<size_t>(number_of_events));
    }
    std::fill(event_types_buf_.begin(), event_types_buf_.begin() + number_of_events, -1);
    int* event_types = event_types_buf_.data();

    int last_event_2braches = -1;
    int last_event_nocross = -1;
    Region expanded_regions[2];

    if (!region_membership_.empty()) {
        for (int s : *region) {
            if (s >= 0 && static_cast<size_t>(s) < region_membership_.size()) {
                region_membership_[static_cast<size_t>(s)] = 1;
            }
        }
    }
    if(print_step_by_step_debug){
        cout<<"eve:"<<event<<endl;
        cout << "|||REGION: " << region << " --- ";
        for (auto i : (*region)) {
            cout << i << " ";
        }
        cout << endl;
    }
   // cout<<"id position reg: " << region_id_position<<endl;

    for (int i = 0; i < number_of_events; i++) {
        event_types[i] = -1;
    }

    for (auto e : *ts_map) {
        //cout << "EVENT: " << e.first << endl;
        // check all, it is not an ES
        if (e.first != event || !is_ER) {
            //cout << "It is not an ES" << endl;
            event_types[e.first] = branch_selection(&e.second, region, e.first,region_id_position);

            /*for (int i = 0; i < number_of_events; i++) {
              cout << "event_type: " << event_types[i] << endl;
            }*/

            // if it is no cross other events are not checked
            if (event_types[e.first] == NOCROSS)
                break;
        }
        //it is an ES, the event related to ES is not checked
        else if (e.first == event) {
            //cout << "is an ES of " << event << endl;
            event_types[event] = OK;
        }
    }
    if (!region_membership_.empty()) {
        for (int s : *region) {
            if (s >= 0 && static_cast<size_t>(s) < region_membership_.size()) {
                region_membership_[static_cast<size_t>(s)] = 0;
            }
        }
    }
    int branch = OK;
    int type;

    // herw there is an array which gives for each event the type of branch for the events after no cross
    // where -1 means that break was done
    //cout << "region pass:" << region;
    set_number_of_bad_events(event_types, number_of_events, event);


    for (int i = 0; i < number_of_events; i++) {
        type = event_types[i];

        if (type == NOCROSS) {
            if(print_step_by_step_debug)
             cout << "Break for no_cross ev "<< i << endl;
            branch = NOCROSS;
            last_event_nocross = i;
            break;
        }
        if (type == EXIT_NOCROSS) {
            if (branch == OK) {
                branch = EXIT_NOCROSS;
                last_event_2braches = i;
            }
            if(print_step_by_step_debug)
            cout << "2 exit branches" << endl;
        } else if (type == ENTER_NOCROSS) {
            if (branch == OK) {
                branch = ENTER_NOCROSS;
                last_event_2braches = i;
            }
            if(print_step_by_step_debug)
             cout << "2 enter branches" << endl;
            //cout << "branch: " << branch << endl;
        }
    }

    if (branch == OK) {
        //cout << "OK" << endl;
        regions->at(event)->push_back(*region);
        // cout<<"push ok"<<endl;

    } else if (branch == NOCROSS) {
        //    cout << "only one nocross branch" << endl;
        for (auto state : *region) {
            (*expanded_regions).insert(state);
            if(print_step_by_step_debug)
            cout << "adding to extended Reg: " << state << endl;
        }
        /*for (auto i : *region) {
            cout << "Stati region " << i << endl;
        }*/

        //cout << "map states to add size: " << (*map_states_to_add).size() << endl;

        Branches_states_to_add *branches = (*map_states_to_add)[last_event_nocross];

        //cout << "dim primo set vettore: " << branches->states_to_add_nocross->size()
        //   << endl;

        for (auto state : *branches->states_to_add_nocross) {
            expanded_regions[0].insert(state);
            if(print_step_by_step_debug)
            cout << "adding to extended Reg: " << state << endl;
        }

        //(*trees_init)[event] = last_event_nocross;
        if(event_violations->find(event) == event_violations->end())
            (*event_violations)[event] = new map<int, int >();
        //auto pairs = new pair<int,int>(event, last_branch);
        (*(*event_violations)[event])[region_id_position] = last_event_nocross;

        // if the temp region is already available it is not inserted
        if (!region_in_queue(*expanded_regions, init_pos)) {
            queue_temp_regions->push_back(*expanded_regions);
            //cout << "Region added to the queue" << endl;
        } /*else {
            //cout << "Region not added to the queue (already available)" << endl;
        }*/

    } else {
        // for no cross the source of all entering arcs has to be added (enter becomes in)
        // for enter the ddestination of out arcs has to be added (out -> enter)

        //cout << "BRANCH EXIT/ENTER_NOCROSS " << endl;
        // (*region).insert(region->begin(), 1);
        //cout << "region size " << (*region).size() << endl;


        if(event_violations->find(event) == event_violations->end())
            (*event_violations)[event] = new map<int, int >();
        (*(*event_violations)[event])[region_id_position] = last_event_2braches;

        for (auto state : *region) {
            (*expanded_regions).insert(state);
            (*(expanded_regions + 1)).insert(state);
            if(print_step_by_step_debug)
            cout << "insert into extended Reg: " << state << endl;
        }

        /*for (auto i : *region) {
          cout << "Stati region " << i << endl;
        }*/

        // BRANCH 1 (NO_CROSS)
        //cout << "map states to add size: " << (*map_states_to_add).size() << endl;

        //cout << "last event 2 branch " << last_event_2braches << endl;

        Branches_states_to_add *branches =
                (*map_states_to_add)[last_event_2braches];

        for (auto state : *branches->states_to_add_nocross) {
            expanded_regions[0].insert(state);
            if(print_step_by_step_debug)
            cout << "insert into extended Reg: " << state << endl;
        }

        if(print_step_by_step_debug) {
            for (auto i: expanded_regions[0]) {
                cout << "State of the expanded region NOCROSS " << i << endl;
            }
        }

        bool reg_not_in_queue = !region_in_queue(*expanded_regions, init_pos);

        if (reg_not_in_queue) {
            queue_temp_regions->push_back(*expanded_regions);
            // cout << "Branch1: region added to the queue" << endl;
        } else {
            //cout << "Branch1: region not added to the queue (already available)" << endl;
        }

        // BRANCH 2 (EXIT/ENTER)

        for (auto state : *branches->states_to_add_exit_or_enter) {
            expanded_regions[1].insert(state);
            if(print_step_by_step_debug)
            cout << "adding to extended Reg: " << state << endl;
        }

        /*if(violations->find(event) == violations->end())
            (*violations)[event] = new map<Region*, pair<int, int>*>();
        pairs = new pair<int,int>(event, last_exit_enter);
        (*(*violations)[event])[region] = pairs;*/
        if(print_step_by_step_debug) {
            for (auto i: expanded_regions[1]) {
                cout << "state of the expanded region ENTER " << i << endl;
            }
        }

        if (reg_not_in_queue) {
            queue_temp_regions->push_back(std::move(expanded_regions[1]));
            //cout << "Branch2: region added to the queue" << endl;
        } else {
            // cout << "Branch 2: region not added to the queue (already available)" << endl;
        }
    }
}

ES createER(int event) {
    auto er = new set<int>;
    for (auto edge : (*ts_map)[event]) {
        (*er).insert(edge->first);
        //  cout << "CREATE ER of " << event << " Insert state: " << edge->first
        //     << endl;
    }

    /* for (auto i : *er) {
       cout << "S: " << i << endl;
     }*/
    return er;
}

map<int, vector<Region> *> *Region_generator::generate() {
    //cout << "----------------------- REGIONS GENERATOR --------------------------" << endl;
    //cout << "GENERATE REGIONS " << endl;
    unsigned int pos = 0;
    int init_pos;
    auto queue_event_index = new map<int, int>; // event and finish index
    ES er_temp;

    for (const auto& e : *ts_map) {
        //cout<<"event "<<e.first<<endl;
        er_temp = createER(e.first);
        (*ES_set)[e.first] = er_temp;
        //cout<<"er "<<endl;
        //println(*er_temp);

        (*regions)[e.first] = new vector<Region>();

        init_pos = pos;
        //cout<<"pos "<<pos<<endl;
        queue_temp_regions->push_back(*er_temp);
        // expand the first time - the region is ES
        expand(&((*queue_temp_regions)[pos]), e.first, true, init_pos, pos-init_pos);
        pos++;

        // add to ES queue and move the position in order to have the complete tree because I want ES as middle_state


        //cout << "*********************************: pos: " << pos
        //   << " reg queue size " << queue_temp_regions->size() << endl;
        while (pos < queue_temp_regions->size() /*&& queue_temp_regions->size()<2*/) {
            if ((*queue_temp_regions)[pos].size() != num_transactions)
                expand(&((*queue_temp_regions)[pos]), e.first, false, init_pos,pos-init_pos);
            else
                regions->at(e.first)->push_back((*queue_temp_regions)[pos]);
            pos++;
        }
        (*queue_event_index)[e.first] = pos - 1;
    }

    set_middle_set_of_states(queue_event_index);

    delete queue_event_index;
    return regions;
}

map<int, vector<Region *> *> *Region_generator::get_middle_set_of_states() {
    return middle_set_of_states;
}

void Region_generator::set_middle_set_of_states(
        map<int, int> *queue_event_index) {

    int init;
    int end = -1;

    for (auto record : *queue_event_index) {
        //cout << "event: " << record.first << endl;
        if ((*middle_set_of_states)[record.first] == nullptr)
            (*middle_set_of_states)[record.first] = new vector<Region *>();

        init = end + 1;
        end = (*queue_event_index)[record.first];
        //cout << "init: " << init << " end: " << end;
        for (int i = init; i <= end; i++) {
            (*middle_set_of_states)
                    .at(record.first)
                    ->push_back(&(*queue_temp_regions)[i]);
        }
    }
}

void Region_generator::set_number_of_bad_events(const int *event_type, int l,
                                                int event) {
    // count for each set of states the bad events

    if (number_of_bad_events->find(event) == number_of_bad_events->end()) {
        (*number_of_bad_events)[event] = new vector<int>;
    }

    // cout << "\nSET BAD NUMBER per " << event << "********" << endl;

    int counter = 0;
    for (int i = 0; i < l; i++) {
        if (event_type[i] != OK && event_type[i] != -1) {
            counter++;
        } else if (event_type[i] == -1) { // break was done for no cross
            counter = -1;
            break;
        }
    }

    number_of_bad_events->at(event)->push_back(counter);
}

map<int, vector<int> *> *Region_generator::get_number_of_bad_events() {
    return number_of_bad_events;
}

map<int, map<int, int >*>* Region_generator::get_violations_event(){
   return event_violations;
}
map<int, map<int, vector<Edge*> *>* >* Region_generator::get_violations_trans(){
    return trans_violations;
}