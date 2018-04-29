//
// Created by viktor on 14/04/18.
//


#ifndef PROGETTO_SSE_ESSENTIAL_REGION_SEARCH_H
#define PROGETTO_SSE_ESSENTIAL_REGION_SEARCH_H


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

private:
	map<int, set<Region*> *> * pre_regions;
};



#endif