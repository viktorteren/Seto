//
// Created by viktor on 14/04/18.
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
#include "Label_splitting_module.h"

using namespace std;
using namespace Utilities;

class Essential_regions_search{
public:
	explicit Essential_regions_search(map<int, set<Region*> *>* pre_reg);
	~Essential_regions_search();
	set<Region*> * search();
	map<int, set<Region*>*> * get_essential_regions_map();

private:
	map<int, set<Region*> *> * pre_regions;
	map<int, set<Region*>*> * essential_map;
};