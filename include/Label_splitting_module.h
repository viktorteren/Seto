//
// Created by ciuchino on 13/04/18.
//

#pragma once

#include "Utilities.h"

using namespace std;
using namespace Utilities;

class Label_splitting_module {
public:
    Label_splitting_module(map<int, set<Region*> *> *pre_regions,
                         map<int, ER> *er_set, map<int, vector<Region *> *> *middle_set_of_states);

    ~Label_splitting_module();

    set<int> *is_excitation_closed();

    map<int, pair<int, Region *> *> *candidate_search(
            map<int, vector<int> *> *number_of_bad_events,
            set<int> *events_not_satisfy_EC);

    void split_ts_map(map<int, pair<int, Region *> *> *candidate_regions, map<int, int> *event_alias,
                      map<int, map<int, int> *> *event_violations,
                      map<int, map<int, vector<Edge *> *> *> *trans_violations,
                      map<int,vector<Region>*> *regions_old);

private:
  map<int, set<Region*> *> *pre_regions;
  map<int, ER> *ER_set;
  map<int, set<int> *> *regions_intersection_map{};
  map<int, vector<Region *> *> *middle_set_of_states;
  My_Map* ts{};
  int branch_selection(Edges_list *list, Region *region);
  void set_number_of_bad_events(vector<int> *event_type, int event,
                                vector<int> *number_of_bad_events, int pos);

};
