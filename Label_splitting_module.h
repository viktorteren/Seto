//
// Created by ciuchino on 13/04/18.
//

#ifndef PROGETTO_SSE_LABEL_SPLITTING_MODULE_H
#define PROGETTO_SSE_LABEL_SPLITTING_MODULE_H

#endif //PROGETTO_SSE_LABEL_SPLITTING_MODULE_H

#include "Minimal_pre_region_generator.h"

using namespace pre_region_gen;

class Label_splitting_module{
public:
    Label_splitting_module(map<int, vector<Region*> *>* pre_regions);
    ~Label_splitting_module();
    bool is_exitation_closed();

private:
    map<int, vector<Region*> *> * pre_regions;
    //bool is_exitation_closed();

};