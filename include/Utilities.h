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
typedef set<int> * ER;
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
	//map<int, set<int>* > * do_regions_intersection2(map<int, set<Region*> *> *);
	set<int>* regions_intersection(set<Region*>* set);
	set<int>* region_difference(set<int>& first, set<int>& second);
	set<Region *>* region_pointer_difference(set<Region *>* first, set<Region*>* second);
	vector<Region>* copy_map_to_vector(map<int, vector<Region> *>* map);
    set<Region*> *copy_map_to_set(map<int, set<Region*> *> *map);
	set<Region *>* initial_regions(map<int,set<Region*>*>* reg);
	map<Region *, int>* get_regions_map(map<int,set<Region*>*>* net);
    Region* get_ptr_into(set<Region *> *set, Region *region);

    void print(Region& region);
    void println(Region& region);
	void println(set<Region *>& regions);
	void print(map<int, set<Region*>*>& net);
    void print_place(Region &region);
    void print_transactions();
    void print_PN(map<int, set<Region*>> *,map<int, set<Region*>> *);
    bool is_bigger_than(Region* region ,set<int>* region2);
    bool is_bigger_than_or_equal_to(Region*,set<int>*);
    bool are_equals(Region *, Region *);
    bool contains(set<Region*>*,Region*);
    //todo: print_ts_dot(); -> forse non serve se leggiamo già i file in .dot
    void print_pn_dot_file(map<int,set<Region*>*>* net, map<int, set<Region *>*> *post_regions, string file_name);
	void restore_default_labels(map<int, set<Region*>*>* net, map<int, int>& aliases);
    bool contains_state(Region* reg,int state);
};