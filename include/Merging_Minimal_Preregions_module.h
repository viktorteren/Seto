//
// Created by ciuchino on 09/05/18.
//
#pragma once

#include "Utilities.h"
#include <algorithm>
using namespace std;

class Merging_Minimal_Preregions_module {

    private: map<int,set<Region*>*>* total_pre_regions_map= nullptr;
    public:

        map<int,set<Region*>*> *get_total_preregions_map();
        Merging_Minimal_Preregions_module(map<int,set<Region*>*> *,map<int,set<Region*>*> *);
        ~Merging_Minimal_Preregions_module();

    private:

        map<int,set<Region*>*>* merging_preregions();
        void merging_2_maps(map<int,set<Region*>*>*,map<int,set<Region*>*>*);
        ER create_ER_after_splitting(int event);
};

