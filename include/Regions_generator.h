/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once

#include "Utilities.h"

using namespace std;

class Region_generator {

public:
    explicit Region_generator(int n);
    ~Region_generator();
    void delete_ER_set();
    __attribute__((unused)) void delete_ES_set_not_in(map<int, ES> *es);
    void delete_regions_map();
    void basic_delete();
    map<int, vector<Region> *> *generate();
    map<int, ES> *get_ES_set();
    int number_of_events;
    struct Branches_states_to_add {
        set<int> *states_to_add_exit_or_enter = nullptr;
        set<int> *states_to_add_nocross = nullptr;
    };

    map<int, vector<Region *> *> *get_middle_set_of_states();
    map<int, vector<int> *> *get_number_of_bad_events();
    static bool remove_bigger_regions(Region &new_region,vector<Region>* regions);
    map<int, map<int, int >*>* get_violations_event();
    map<int, map< int , vector<Edge*> *>* >* get_violations_trans();

private:
    map<int, ES> *ES_set=nullptr;
    map<int, vector<Region> *> *regions;
    vector<Region> *queue_temp_regions;

    map<int, Branches_states_to_add *> *map_states_to_add = nullptr;
    set<int> *states_to_add_enter = nullptr;
    set<int> *states_to_add_exit = nullptr;
    set<int> *states_to_add_nocross = nullptr;

    map<int, vector<Region *> *> *middle_set_of_states;
    map<int, vector<int> *> *number_of_bad_events;

    map<int, map<int, int >*>* event_violations;
    map<int, map<int, vector<Edge*> *>* >* trans_violations;

    int branch_selection(Edges_list *list, Region *region, int event,int region_id_position);
    bool region_in_queue(Region &new_region, int init_pos);
    void expand(Region *region, int event, bool is_ER, int init_pos,int region_id_position);
    void set_middle_set_of_states(map<int, int> *queue_event_index);
    void set_number_of_bad_events(const int *event_type, int l, int event);

};