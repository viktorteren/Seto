/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once

#include "Essential_region_search.h"
using namespace std;

//irredundant because given all computational branches there is no branch with redundancies
//minimum because given all irredundant paths will be chosen the one with the minimum cost
//minimum implies irredundant but not vice versa
class Place_irredundant_pn_creation_module {
public:
    explicit Place_irredundant_pn_creation_module(map<int, set<Region *> *> *pre_reg, map<int, ES> *er_map);
    ~Place_irredundant_pn_creation_module();

    map<int, set<Region *> *> *get_irredundant_regions();
    map<int, set<Region *> *> *get_essential_regions();

private:
    Essential_regions_search *ers;
    map<int, set<Region *> *> *pre_regions = nullptr;
    set<Region *> *essential_regions = nullptr;
    set<Region *> *not_essential_regions = nullptr;
    set<int> *uncovered_states = nullptr;
    map<int, ES> *es = nullptr;
    set<int>* all_events = nullptr;
    map<int, set<Region *> *> *not_essential_regions_map; // maps: event -> sety of not essential regions for that event
    set<int> *search_not_covered_states_per_event();
    map<Region *, int> *cost_map; //cost for each region (given by: number of events for which it is a pre-region +
                                  // number of events for which it is a post-region + 1)
    set<Region *> *irredundant_regions = nullptr;
    set<set<Region *>> *computed_paths_cache = nullptr; //if I have already calculated the cost of a path [set of regions]
                                                        // then the value is already in cache
    map<int, set<Region *> *> *irredundant_regions_map = nullptr;
    set<int>* events_without_essential_regions = nullptr;

    void search_not_essential_regions();
    void cost_map_filling();

    /**
     * @return returns the best result of the child branches
     */
    int minimum_cost_search_with_label_costraints(set<int> states_to_cover, set<Region *> *used_regions,
                            int last_best_cost, int father_cost, int level);
    unsigned long region_cost(Region *reg);
    void calculate_irredundant_regions_map();
    set<int>* calculate_events_without_essential_regions();
    bool all_events_have_ec_satisfied(set<Region*> &irredundant_regions);
    bool ec_satisfied(int event, set<Region*> *events_regions);
    __attribute__((unused)) bool irredundant_set_of_regions(set<Region*>* irredundant_regions_set);
    bool irredundant_set_for_event(int event, set<Region*> *events_regions);
};