//
// Created by ciuchino on 09/05/18.
//
#pragma once

#include "Utilities.h"
#include <algorithm>
using namespace std;

class Merging_Minimal_Preregions_module {

private:
  map<int, set<Region *> *> *total_pre_regions_map = nullptr;
  map<int, set<Region *> *> *merged_pre_regions_map = nullptr;
  map<int, ER> *er = nullptr;
  Region *union_ptr = nullptr;

  map<int, set<Region *> *> *merging_preregions(map<int, ER> *);
  void merging_2_maps(map<int, set<Region *> *> *, map<int, set<Region *> *> *);

public:
  map<int, set<Region *> *> *get_merged_preregions_map();
  map<int, set<Region *> *> *get_total_preregions_map();
  Merging_Minimal_Preregions_module(map<int, set<Region *> *> *,
                                    map<int, set<Region *> *> *,
                                    map<int, ER> *);
  ~Merging_Minimal_Preregions_module();
};
