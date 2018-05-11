//
// Created by ciuchino on 13/04/18.
//

#pragma once

#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <tuple>
#include <map>
#include <string>
#include <set>
#include "Utilities.h"

using namespace std;
using namespace Utilities;

class Label_splitting_module{
public:
    Label_splitting_module(map<int, vector<Region> *>* regions,map<int,ER> *er_set);
    ~Label_splitting_module();
    set<int>* is_excitation_closed();
    vector<Region>* do_label_splitting(map<int, vector<Region*> *>* middle_set_of_states,
                                       map<int,vector< int >*>* number_of_bad_events,set<int> * events_not_satisfy_EC);

private:
    map<int, vector<Region> *> * regions;
    map<int,ER> *ER_set;
    map<int, set<int>* > *regions_intersection;
    //coppia numero eventi che violano la regione e ptr_regione
    //map<int,vector< pair<int,Region*> >*>* number_of_bad_events;
    int branch_selection(List_edges *list, Region *region);
    void set_number_of_bad_events(vector<int>* event_type,int event,vector< int >* number_of_bad_events);

};
