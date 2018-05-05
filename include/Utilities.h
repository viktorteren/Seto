//
// Created by ciuchino on 18/04/18.
//

#pragma once
#include <iostream>
#include <vector>
#include <tuple>
#include <map>
#include <set>
#include <fstream>
#include <cassert>
#include "TS_parser.h"
#include "numeric"

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
    set<int> *regions_union(vector<Region*>* vec);
	set<int> *regions_union(set<Region*>* vec);
	set<int> *regions_union(Region* first, Region* second);
	set<int> *regions_intersection(Region* first, Region* second);
    set<int> *regions_intersection2(Region& first, Region& second);
    map<int, set<int>* > * do_regions_intersection(map<int, vector<Region> *> * regions);
    void print(Region& region);
	void println(Region& region);
	void print_place(Region &region);
    void print_transactions();
	void print_PN(map<int, set<Region*>> *,map<int, set<Region*>> *);
	set<int>* region_difference(set<int>& first, set<int>& second);
	vector<Region>* copy_map_to_vector(map<int, vector<Region> *>* map);
	bool is_bigger_than(Region* region ,set<int>* region2);
	bool are_equals(Region*,Region*);
	bool contains(set<Region*>*,Region*);
	void printRegion(const Region& region);
};