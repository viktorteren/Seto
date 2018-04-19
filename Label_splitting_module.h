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
    Label_splitting_module(map<int, vector<Region*> *>* pre_regions,vector<ER> *er_set);
    ~Label_splitting_module();
    bool is_excitation_closed();
    void do_label_splitting(map<int, vector<Region*> *>* middle_set_of_states);

private:
    map<int, vector<Region*> *> * pre_regions;
    vector<ER> *ER_set;
    //bool is_exitation_closed();
    bool is_equal_to(ER er,set<int>* intersection);
    bool is_bigger_than(Region*,set<int>*);
    map<int, set<int>* > *pre_regions_intersection;

};

#endif
