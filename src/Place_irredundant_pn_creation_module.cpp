/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Place_irredundant_pn_creation_module.h"
#include "../include/Essential_region_search.h"
#include <climits>

Place_irredundant_pn_creation_module::Place_irredundant_pn_creation_module(
        map<int, set<Region *> *> *pre_reg, map<int, ES> *er_map) {

    pre_regions = pre_reg;

    cost_map = new map<Region *, int>();
    not_essential_regions_map = new map<int, set<Region *> *>();
    not_essential_regions = new set<Region *>();
    ers = new Essential_regions_search(pre_regions);
    essential_regions = ers->search();
    all_events = new set<int>();
    for (auto record: *pre_regions) {
        all_events->insert(record.first);
    }
    events_without_essential_regions = calculate_events_without_essential_regions();
    search_not_essential_regions();
    irredundant_regions = new set<Region *>();
    es = er_map;
    if (!not_essential_regions->empty()) {
        cost_map_filling();
        if (print_step_by_step) {
            cout << "Not essential regions:" << endl;
            println(*not_essential_regions);
            cout << endl;
        }
        //cout << "Number of not essential regions: " << not_essential_regions->size() << endl;
        //println(*not_essential_regions);
        uncovered_states = search_not_covered_states_per_event();
        if (!uncovered_states->empty()) {
            set<int> states_to_cover = *uncovered_states;
            delete uncovered_states;
            auto used_regions = new set<Region *>();
            computed_paths_cache = new set<set<Region *>>();
            //cout << "--------------------------------------------------- MINIMUM "
            //     "COST SEARCH --------------------------------------------"
            //<< endl;
            //minimum_cost_search(states_to_cover, used_regions, INT_MAX, 0);
            //cout << "-----------IRREDUNDANT REGIONS SEARCH-------------" << endl;
            minimum_cost_search_with_label_costraints(states_to_cover, used_regions, INT_MAX, 0, 0);
            //  cout << "min cost: " << min << endl;
            if (print_step_by_step) {
                cout << "Irredundant set of regions: " << endl;
                for (auto region : *irredundant_regions) {
                    // cout << "[" << &(*region)  << "] ";
                    println(*region);
                }
                cout << "" << endl;
            }
            delete used_regions;
        } else {
            if (print_step_by_step) {
                cout << "Each region is essential" << endl;
                cout << "" << endl;
            }
        }

    } else {
        if (print_step_by_step) {
            cout << "Each region is essential" << endl;
            cout << "" << endl;
        }
    }
}

Place_irredundant_pn_creation_module::~Place_irredundant_pn_creation_module() {
    delete ers;
    delete essential_regions;
    delete not_essential_regions;
    delete cost_map;
    for (auto rec : *not_essential_regions_map) {
        delete rec.second;
    }
    delete not_essential_regions_map;

    if (irredundant_regions_map != nullptr) {
        for (auto rec : *irredundant_regions_map) {
            delete rec.second;
        }
        delete irredundant_regions_map;
    }
    delete irredundant_regions;
    delete computed_paths_cache;
    delete events_without_essential_regions;
    delete all_events;
}

set<int> *Place_irredundant_pn_creation_module::calculate_events_without_essential_regions() {
    int cont = 0;
    auto events = new set<int>();
    for (auto record: *pre_regions) {
        for (auto region: *record.second) {
            cont = 0;
            if (essential_regions->find(region) != essential_regions->end()) {
                cont++;
                break;
            }
        }
        if (cont == 0) {
            events->insert(record.first);
        }
    }
    /*cout << "events without essential regions: ";
    for(auto ev: *events){
      cout << " " << ev;
    }*/
    cout << endl;
    return events;
}

__attribute__((unused)) bool Place_irredundant_pn_creation_module::irredundant_set_of_regions(set<Region *> *irredundant_regions_set) {
    auto candidate_set_of_regions = new set<Region *>(*irredundant_regions_set);
    //union between essential and irredundant regions
    candidate_set_of_regions->insert(essential_regions->begin(), essential_regions->end());

    for (auto ev: *all_events) {
        //check which regions of event wv are left
        auto regions_of_ev = new set<Region *>();
        for (auto reg: *pre_regions->at(ev)) {
            if (candidate_set_of_regions->find(reg) != candidate_set_of_regions->end()) {
                regions_of_ev->insert(reg);
            }
        }
        // check if all its regions have EC satisfied
        if (!irredundant_set_for_event(ev, regions_of_ev)) {
            delete regions_of_ev;
            delete candidate_set_of_regions;
            return false;
        }
        delete regions_of_ev;
    }
    delete candidate_set_of_regions;

    return true;
}

bool Place_irredundant_pn_creation_module::irredundant_set_for_event(int event, set<Region *> *events_regions) {
    set<Region *> *regions_of_event = pre_regions->at(event);
    int total_states = regions_union(regions_of_event)->size();
    int temp_states;
    for (auto reg: *events_regions) {
        auto regions_copy = new set<Region *>(*events_regions);
        regions_copy->erase(reg);
        temp_states = regions_union(regions_copy)->size();
        if (temp_states == total_states) {
            delete regions_copy;
            return false;
        }
        delete regions_copy;
    }
    return true;
}

bool Place_irredundant_pn_creation_module::all_events_have_ec_satisfied(set<Region *> &irredundant_regions_set) {
    //for each event without essential regions
    auto candidate_set_of_regions = new set<Region *>(irredundant_regions_set);

    //union between essential regions and irredundant ones
    candidate_set_of_regions->insert(essential_regions->begin(), essential_regions->end());

    for (auto ev: *all_events) {
        //check which regions of event ev are left
        auto regions_of_ev = new set<Region *>();
        for (auto reg: *pre_regions->at(ev)) {
            if (candidate_set_of_regions->find(reg) != candidate_set_of_regions->end()) {
                regions_of_ev->insert(reg);
            }
        }
        //check if all its regions have EC satisfied
        if (!ec_satisfied(ev, regions_of_ev)) {
            delete regions_of_ev;
            delete candidate_set_of_regions;
            return false;
        }
        delete regions_of_ev;
    }
    delete candidate_set_of_regions;
    //cout << "all ec satiisfied" << endl;
    return true;
}

bool Place_irredundant_pn_creation_module::ec_satisfied(int event, set<Region *> *events_regions) {
    auto intersection = regions_intersection(events_regions);
    if (intersection->size() != es->at(event)->size()) {
        delete intersection;
        //cout << "return false ec satisfied" << endl;
        return false;
    } else {
        for (auto reg: *es->at(event)) {
            if (intersection->find(reg) == intersection->end()) {
                delete intersection;
                //cout << "return false ec satisfied" << endl;
                return false;
            }
        }
    }
    delete intersection;
    //cout << "return true ec satisfied" << endl;
    return true;
}


map<int, set<Region *> *> *Place_irredundant_pn_creation_module::get_irredundant_regions() {
    if (!irredundant_regions->empty()) {
        calculate_irredundant_regions_map();
        return irredundant_regions_map;
    } else {
        return nullptr;
    }
}

map<int, set<Region *> *> * Place_irredundant_pn_creation_module::get_essential_regions() {
    return ers->get_essential_regions_map();
}

void Place_irredundant_pn_creation_module::search_not_essential_regions() {
    for (auto record : *pre_regions) {

        // cout << "neewe event: " << record.first << endl;
        for (auto region : *record.second) {
            // cout << &(*region) << endl;
            // not essential region
            if (essential_regions->find(&(*region)) == essential_regions->end()) {
                if (not_essential_regions_map->find(record.first) ==
                    not_essential_regions_map->end()) {
                    (*not_essential_regions_map)[record.first] = new set<Region *>();
                }
                (*not_essential_regions_map)[record.first]->insert(region);
                not_essential_regions->insert(region);
            }
        }
    }
}

set<int> *Place_irredundant_pn_creation_module::search_not_covered_states_per_event() {
    //cout << "--------------------------------------------------- SEARCHING FOR "
    //     "UNCOVERED STATES --------------------------------------------"
    //<< endl;
    int event;
    set<int> *event_states = nullptr;
    set<int> *essential_states = nullptr;
    // set<int> *uncovered_states = nullptr;
    set<int> *total_uncovered_states = nullptr;
    set<Region *> *regions;
    auto essential_regions_of_event = new vector<Region *>();
    bool first_iteration = true;

    // for each event with not essential regions:
    for (auto record : *not_essential_regions_map) {
        if (!first_iteration)
            delete uncovered_states;
        first_iteration = false;
        set<int> *uncovered_states = nullptr;
        event = record.first;
        // cout << "uncovered states for event " << event << ":" << endl;
        // calculate the union of states covered by pre-regions of the current event
        regions = (*(pre_regions)->find(event)).second;
        event_states = regions_union(regions);
        // calculate the states covered by essential pre-regions
        // check all pre-regions of the event taking only the essential ones
        // cout << "added essential region of the event " << event << endl;
        for (auto reg : *(pre_regions->find(event)->second)) {
            if (essential_regions->find(reg) != essential_regions->end()) {
                /*cout << "essential region: ";
                print(*reg);*/
                essential_regions_of_event->push_back(reg);
            }
        }
        essential_states = regions_union(essential_regions_of_event);

        // calculate the uncovered states
        uncovered_states = region_difference(*event_states, *essential_states);
        // println(*uncovered_states);
        // cout << "---------------" << endl;
        // cout << "event: " << record.first << endl;
        /*cout << "all the states of the events: ";
        print(event_states);
        cout << "states covered by essential events: ";
        print(essential_states);*/
        /*cout << "states not covered by essential events: ";
        println(uncovered_states);
        cout << "---------------" << endl;*/

        if (total_uncovered_states == nullptr) {
            total_uncovered_states = uncovered_states;
        } else {
            if (total_uncovered_states->empty()) {
                total_uncovered_states = uncovered_states;
            } else {
                //				total_uncovered_states =
                //regions_union(total_uncovered_states, uncovered_states);
                auto tmp = regions_union(total_uncovered_states, uncovered_states);
                delete total_uncovered_states;
                total_uncovered_states = tmp;
                delete uncovered_states;
            }
        }

        // clear the avariables for each iteration
        essential_regions_of_event->erase(essential_regions_of_event->begin(),
                                          essential_regions_of_event->end());
        delete event_states;
        delete essential_states;
    }
    delete essential_regions_of_event;
    delete uncovered_states;

    return total_uncovered_states;
}

int Place_irredundant_pn_creation_module::minimum_cost_search_with_label_costraints(
        set<int> states_to_cover, set<Region *> *used_regions, int last_best_cost,
        int father_cost, int level) {
    // for the set of uncovered states the possible irredundant coverings have to be found

    // aggregate not covered states without splitting these for different events and add not essential regions
    // which cover most possible uncovered states
    // the computation stops if the current result is greater than the old bes one -> at the end the set is surely
    // irredundant


    //cout << "Regions of my father: ";
    //println(*used_regions);

    auto candidate = reinterpret_cast<Region *>(-1);
    int cover_of_candidate;
    int temp_cover;
    // copy my father's content in order to not overwrite the data with child's data
    int cost_of_candidate;
    set<int> *new_states_to_cover;
    set<Region *> *new_states_used = nullptr;
    auto chosen_candidates = new set<Region *>(*used_regions);
    set<Region *> *temp_aggregation;
    int new_best_cost = last_best_cost; //one of che child branches could have improved the result, therefore there is
                                        // the need of a new variable and the parameter sent to the function is not used
                                        // until there are candidates which increase the covering
    while (true) {
        cover_of_candidate = -1;
        delete new_states_used;
        new_states_used = new set<Region *>(*used_regions);
        // chosing the next candidate
        for (auto region : *not_essential_regions) {
            // the new region cannot be an old candidate
            if (chosen_candidates->find(region) == chosen_candidates->end()) {
                // check if the set with the candidate was already computed or not
                // [this check is done because it should be lighter than the intersection in the next condition]
                temp_aggregation = new set<Region *>(*used_regions);
                temp_aggregation->insert(region);
                if (computed_paths_cache->find(*temp_aggregation) ==
                    computed_paths_cache->end()) {
                    // the size of the intersection of states to cover and the region has to be seen
                    auto cover = regions_intersection(&states_to_cover, region);
                    temp_cover = static_cast<int>(cover->size());
                    delete cover;
                    if (temp_cover > cover_of_candidate) {
                        // cout << "new candidate" << endl;
                        cover_of_candidate = temp_cover;
                        candidate = region;
                    } else {
                        //cout << "temp cover has less coverage than cover of candidate" << endl;
                    }
                } else {
                    /*if(print_step_by_step_debug) {
                        cout << "the set is already in cache: " << endl;
                        println(*temp_aggregation);
                    }*/
                }
                // each time entering in the if condition a new spacce is allocated therefore it has to be deallocated
                delete temp_aggregation;
            }
        }
        //the candidate was not found
        if (cover_of_candidate == -1) {
            //cout << "candidate not found" << endl;
            break;
        }
        // cout << "cover of candidate: " << cover_of_candidate << endl;

        // the coverage cannot be improved and EC is satisfied
        if (cover_of_candidate == 0) {
            if (all_events_have_ec_satisfied(*new_states_used)) {
                //cout << "break exit. cover = 0 && all..." << endl;
                break;
            }
        }

        if (candidate == reinterpret_cast<Region *>(-1)) {
            //cout << "candidate not initialized" << endl;
            break;
        }

        // save the new candidate
        chosen_candidates->insert(candidate);

        cost_of_candidate = (*cost_map)[&(*candidate)];
        // cout << "cost of candidate: " << cost_of_candidate << endl;
        // cout << "candidate cover: " << cover_of_candidate << endl;
        // the recursive call has not to be done if the cost is too big or the coverage is already complete

        // a better solution cannot be found with the following candidate
        int current_cost = cost_of_candidate + father_cost;
        new_states_used->insert(candidate);

        if (print_step_by_step_debug) {
            //cout << "I am at level " << level << endl;
            //cout << "the set of candidates is: " << endl;
            //println(*new_states_used);
        }

        // save the path in the cash in order to not repeat it
        computed_paths_cache->insert(*new_states_used);
        if (current_cost >= new_best_cost) {
            // do not continue on this path
            //if (print_step_by_step_debug)
               // cout << "taglio" << endl;
        }
        // the coverage is complete and it is better than the previous one
        else if (states_to_cover.size() - cover_of_candidate == 0 && all_events_have_ec_satisfied(*new_states_used)) {
            new_best_cost = current_cost;
            // deallocate the old space in order to allocate a new one
            delete irredundant_regions;
            irredundant_regions = new set<Region *>(*new_states_used);
            //cout << "result found: ";
            //println(*irredundant_regions);
            //break;
        } else {
            // being the candidate already chosen and knowing that the recursive call has to be done, the new set to
            // cover has to be calculated
            new_states_to_cover = region_difference(states_to_cover, *candidate);

            // recursive call to expand the coverage with the chosen candidate
            int cost = minimum_cost_search_with_label_costraints(*new_states_to_cover, new_states_used,
                                                                 new_best_cost, current_cost, level + 1);
            if (cost < new_best_cost) {
                new_best_cost = cost;
                // the result has not to be saved because it was previously saved in the recursive call with the better
                // cost
            }

            delete new_states_to_cover;
        }
    }

    //save the path in cach in order to not repeat it: all child paths were deleted for this reason the code is arrived
    // to the return
    //cout << "regioni irridondanti correnti: ";
    //println(*used_regions);
    delete chosen_candidates;
    delete new_states_used;
    //cout << "new best cost: " << new_best_cost << endl;
    return new_best_cost;
}

// the map has to contain only the  costs of not essential regions
void Place_irredundant_pn_creation_module::cost_map_filling() {
    // cout << "--------------------------------------------------- COST MAP "
    //       "FILLING --------------------------------------------"
    // << endl;

    for (auto record : *not_essential_regions_map) {
        for (Region *reg : *record.second) {
            // the cost for the region reg was not calculated yet
            if (cost_map->find(&(*reg)) == cost_map->end()) {
                (*cost_map)[&(*reg)] = region_cost(&(*reg));
                //   cout << "Found not essential region :";
                // print(*reg);
                //cout << "[" << &(*reg) << "] with cost: " << (*cost_map)[&(*reg)]
                //   << endl;
            }
        }
    }
}

/**
 *
 * @param reg
 * @return 1
 * @brief Function which minimize the number of places
 */
unsigned long Place_irredundant_pn_creation_module::region_cost(Region *reg) {
    // Commented code  was done in order to minimize the number of places and arcs
    /*int cost = 1;
    for(auto record: *pre_regions){
            if(record.second->find(reg) != record.second->end()){
                    cost++;
            }
    }
    for(auto record: *post_regions){
            if(record.second->find(reg) != record.second->end()){
                    cost++;
            }
    }
    return cost;*/

    return 1;
}

void Place_irredundant_pn_creation_module::calculate_irredundant_regions_map() {

    //cout << "CALCULATE IRREDUNDANT REGION MAP_______________" << endl;
    irredundant_regions_map = new map<int, set<Region *> *>();
    for (auto record : *not_essential_regions_map) {
        for (auto reg : *record.second) {
            if (irredundant_regions->find(reg) != irredundant_regions->end()) {
                if (irredundant_regions_map->find(record.first) ==
                    irredundant_regions_map->end()) {
                    (*irredundant_regions_map)[record.first] = new set<Region *>();
                }

                //  cout << "REGION: " << reg << " AT " << record.first << endl;
                (*irredundant_regions_map)[record.first]->insert(reg);
            }
        }
    }

}
