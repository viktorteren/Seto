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
    map<int, set<int>* > * do_regions_intersection(map<int, vector<Region> *> *);
	map<int, set<int>* > * do_regions_intersection2(map<int, set<Region*> *> *);
	set<int>* regions_intersection(set<Region*>* set);
    void print(Region& region);
	void println(Region& region);
	void print_place(Region &region);
    void print_transactions();
	void print_PN(map<int, set<Region*>> *,map<int, set<Region*>> *);
	set<int>* region_difference(set<int>& first, set<int>& second);
	vector<Region>* copy_map_to_vector(map<int, vector<Region> *>* map);
    set<Region*> *copy_map_to_set(map<int, set<Region*> *> *map);
	bool is_bigger_than(Region* region ,set<int>* region2);
    bool is_bigger_than_or_equal_to(Region*,set<int>*);
    bool are_equals(Region *, Region *);
	bool contains(set<Region*>*,Region*);
	//todo: print_ts_dot(); -> forse non serve se leggiamo già fil in .dot
	//todo: print_pn_dot_file() -> utilizzato per l'output con graphviz
	void print_pn_dot_file(map<int,set<Region*>*>* net, string file_name);
	set<Region *>* initial_regions(map<int,set<Region*>*>* reg);
};