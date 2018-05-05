//
// Created by ciuchino on 13/04/18.
//

#ifndef PROGETTO_SSE_LABEL_SPLITTING_MODULE_H
#define PROGETTO_SSE_LABEL_SPLITTING_MODULE_H




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
    Label_splitting_module(map<int, vector<Region> *>* regions,vector<ER> *er_set);
    ~Label_splitting_module();
    vector<int>* is_excitation_closed();
    vector<Region>* do_label_splitting(map<int, vector<Region*> *>* middle_set_of_states,
                                       map<int,vector< int >*>* number_of_bad_events,vector<int> * events_not_satisfy_EC);

private:
    map<int, vector<Region> *> * regions;
    vector<ER> *ER_set;
    //bool is_exitation_closed();
    bool is_equal_to(ER er,const set<int>* intersection);
    bool is_bigger_than(Region*,set<int>*);
    map<int, set<int>* > *regions_intersection;
    //coppia numero eventi che violano la regione e ptr_regione
    //map<int,vector< pair<int,Region*> >*>* number_of_bad_events;
    int branch_selection(List_edges *list, Region *region);
    void set_number_of_bad_events(vector<int>* event_type,int event,vector< int >* number_of_bad_events);

};

#endif
