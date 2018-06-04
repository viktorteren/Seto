//
// Created by viktor on 14/04/18.
//

#pragma once

#include "Label_splitting_module.h"
#include "Utilities.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;
using namespace Utilities;

class Essential_regions_search {
public:
  explicit Essential_regions_search(map<int, set<Region *> *> *pre_reg);
  ~Essential_regions_search();
  set<Region *> *search();
  map<int, set<Region *> *> *get_essential_regions_map();

private:
  map<int, set<Region *> *> *pre_regions;
  map<int, set<Region *> *> *essential_map;
};