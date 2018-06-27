//
// Created by viktor on 12/04/18.
//

#pragma once

#include "Utilities.h"

using namespace std;

class Region_generator {

public:
  Region_generator(int n);
  ~Region_generator();

  map<int, vector<Region> *> *generate();

  map<int, ER> *get_ER_set();
  int number_of_events;

  struct Branches_states_to_add {
    set<int> *states_to_add_exit_or_enter = nullptr;
    set<int> *states_to_add_nocross = nullptr;
    /*~Branches_states_to_add(){
            if(states_to_add_exit_or_enter!= nullptr) delete
       states_to_add_exit_or_enter;
            if(states_to_add_nocross != nullptr) delete states_to_add_nocross;
            }*/
  };

  map<int, vector<Region *> *> *get_middle_set_of_states();
  map<int, vector<int> *> *get_number_of_bad_events();
  map<int, int> *get_trees_init();
    void remove_bigger_regions(Region &new_region,vector<Region>* regions);

private:
  map<int, ER> *ER_set;
  map<int, vector<Region> *> *regions;
  vector<Region> *queue_temp_regions;

  map<int, Branches_states_to_add *> *map_states_to_add = nullptr;
  // Branches_states_to_add *struct_states_to_add= nullptr;
  set<int> *states_to_add_enter = nullptr;
  set<int> *states_to_add_exit = nullptr;
  set<int> *states_to_add_nocross = nullptr;

  map<int, vector<Region *> *> *middle_set_of_states;
  // per ogni evento vector di coppie numero eventi che violano la regione e
  // ptr_regione
  map<int, vector<int> *> *number_of_bad_events;

  // indica che l'albero di first inizia con second
  map<int, int> *trees_init;

  // ER createER(int event);
  int branch_selection(Edges_list *list, Region *region, int event);
  bool region_in_queue(Region &new_region, int init_pos);
  void expand(Region *region, int event, bool is_ER, int init_pos);
  void set_middle_set_of_states(map<int, int> *queue_event_index);
  void set_number_of_bad_events(int *event_type, int l, int event);
};