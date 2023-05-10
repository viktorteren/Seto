/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once

#include <algorithm>
#include "Utilities.h"
#include "../include/Pre_and_post_regions_generator.h"

using namespace std;
using namespace Utilities;

class Merging_Minimal_Preregions_module {

private:
    map<int, set<Region *> *> *total_pre_regions_map = nullptr;
    map<int, set<Region *> *> *merged_pre_regions_map = nullptr;
    map<int, ES> *es = nullptr;
    Region *union_ptr = nullptr;
    bool generic=false;

    map<int, set<Region *> *> *merging_preregions(map<int, ES> *);

public:
    map<int, set<Region *> *> *get_merged_preregions_map();
    map<int, set<Region *> *> *get_total_preregions_map();
    Merging_Minimal_Preregions_module(map<int, set<Region *> *> * essential_regions,
                                    map<int, set<Region *> *> * irredundant_regions,
                                    map<int, ES> *ES);

    __attribute__((unused)) Merging_Minimal_Preregions_module(map<int, set<Region *> *> * regions,
                                    map<int, ES> *ES,
                                    bool generic);
    ~Merging_Minimal_Preregions_module();
};
