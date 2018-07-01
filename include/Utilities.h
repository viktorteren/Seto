//
// Created by ciuchino on 18/04/18.
//

#pragma once
#include "../include/TS_parser.h"
#include "numeric"
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

typedef set<int> Region;
typedef std::pair<int, int> Edge;
typedef set<int> *ER;
typedef set<Edge *> Edges_list;

const int OK = 0;
const int NOCROSS = 1;
const int EXIT_NOCROSS = 2;
const int ENTER_NOCROSS = 3;
const int EXIT_OUT = 4;
const int ENTER_OUT = 5;
const int ENTER_IN = 6;
const int EXIT_IN = 7;
const int ENTER_EXIT=8;

namespace Utilities {

set<int> *regions_union(vector<Region *> *vec);
set<int> *regions_union(set<Region *> *vec);
set<int> *regions_union(Region *first, Region *second);
set<int> *regions_intersection(Region *first, Region *second);
map<int, set<int> *> *do_regions_intersection(map<int, vector<Region> *> *);
set<int> *regions_intersection(set<Region *> *set);
set<int> *region_difference(set<int> &first, set<int> &second);
set<Region *> *region_pointer_difference(set<Region *> *first,
                                         set<Region *> *second);
vector<Region> *copy_map_to_vector(map<int, vector<Region> *> *map);
set<Region *> *copy_map_to_set(map<int, set<Region *> *> *map);
set<Region *> *initial_regions(map<int, set<Region *> *> *reg);
map<Region *, int> *get_regions_map(map<int, set<Region *> *> *net);
Region *get_ptr_into(set<Region *> *set, Region *region);

void print(Region &region);
void println(Region &region);
void println(set<Region *> &regions);
void println(vector<Region *> &regions);
void print(map<int, set<Region *> *> &net);
void print(map<int, vector<Region> *> &net);
void print_place(Region &region);
void print_transactions();
bool is_bigger_than(Region *region, set<int> *region2);
bool is_bigger_than_or_equal_to(Region *, set<int> *);
bool are_equal(Region *r1, Region *r2);
bool contains(set<Region *> *, Region *);
void print_ts_dot_file(string file_path,map<int, int> *aliases); //-> forse non serve se leggiamo già i file in .dot
void print_pn_dot_file(map<int, set<Region *> *> *net,
                       map<int, set<Region *> *> *post_regions,
                       map<int, int>* aliases, string file_name);
void restore_default_labels(map<int, set<Region *> *> *net,
                            map<int, int> &aliases);
char translate_label(int label);
bool contains_state(Region *reg, int state);
};