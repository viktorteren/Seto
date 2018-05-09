//
// Created by ciuchino on 09/05/18.
//

#ifndef PROGETTO_SSE_MERGING_MINIMAL_PREREGIONS_MODULE_H
#define PROGETTO_SSE_MERGING_MINIMAL_PREREGIONS_MODULE_H

#include "Utilities.h"
using namespace std;

class Merging_Minimal_Preregions_module {

    //private: map<int,set<Region*>*> total_pre_regions_map= nullptr;
    public:
        map<int,set<Region*>*> merging_preregions();
        Merging_Minimal_Preregions_module(map<int,set<Region*>*>,map<int,set<Region*>*>);
        ~Merging_Minimal_Preregions_module();

    private: map<int,set<Region*>*> merging_2_maps();
};


#endif //PROGETTO_SSE_MERGING_MINIMAL_PREREGIONS_MODULE_H
