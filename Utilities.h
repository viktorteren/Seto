//
// Created by ciuchino on 18/04/18.
//

#ifndef PROGETTO_SSE_UTILITIES_H
#define PROGETTO_SSE_UTILITIES_H


#include <iostream>
#include <vector>
#include <tuple>
#include <map>
#include <set>
#include <fstream>
#include <assert.h>
#include "TS_parser.h"

using namespace std;

typedef set<int> Region;
typedef std::pair<int, int> Edge;
typedef set<int> *ER;
typedef vector<Edge> List_edges;

const int OK = 0;
const int NOCROSS = 1;
const int EXIT_NOCROSS = 2;
const int ENTER_NOCROSS = 3;

namespace Utilities {
    set<int> regions_union(vector<Region*>* vec);
	set<int> regions_union(set<Region*>* vec);
    map<int, set<int>* > * regions_intersection(map<int, vector<Region*> *> * pre_regions);
    void print(Region& region);
	set<int> region_difference(set<int>& first, set<int>& second);
};


#endif //PROGETTO_SSE_UTILITIES_H
