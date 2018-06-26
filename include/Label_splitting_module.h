//
// Created by ciuchino on 13/04/18.
//

#pragma once

#include "Utilities.h"

using namespace std;
using namespace Utilities;

class Label_splitting_module {
public:
  Label_splitting_module(map<int, vector<Region> *> *regions,
                         map<int, ER> *er_set);
  ~Label_splitting_module();
  set<int> *is_excitation_closed();
  vector<Region> *
  do_label_splitting(map<int, vector<Region *> *> *middle_set_of_states,
                     map<int, vector<int> *> *number_of_bad_events,
                     set<int> *events_not_satisfy_EC);
  void split_ts_map(map<int, int> *events_alias,
                    map<int, set<Region *> *> *pre_regions);

    void split_ts_map_2(vector<Region> * candidate_regions);

private:
  map<int, vector<Region> *> *regions;
  map<int, ER> *ER_set;
  map<int, set<int> *> *regions_intersection;
  map<int,int>* event_alias= nullptr;
  // coppia numero eventi che violano la regione e ptr_regione
  // map<int,vector< pair<int,Region*> >*>* number_of_bad_events;
  int branch_selection(Edges_list *list, Region *region);
  void set_number_of_bad_events(vector<int> *event_type, int event,
                                vector<int> *number_of_bad_events, int pos);


    map<int, vector<Edge *> *> *calculate_exit_trans(Region *pSet);

    vector<int> *calculate_exit_trans_events(Region *pSet);
};
