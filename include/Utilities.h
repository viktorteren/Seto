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

extern bool print_step_by_step;
extern bool print_step_by_step_debug;

namespace Utilities {

set<int> *regions_union(vector<Region *> *vec);
set<int> *regions_union(set<Region *> *vec);
set<int> *regions_union(Region *first, Region *second);
set<int> *regions_intersection(Region *first, Region *second);
set<int> *regions_intersection(set<Region *> *set);
set<int> *region_difference(set<int> &first, set<int> &second);
set<Region *> *region_pointer_difference(set<Region *> *first,
                                         set<Region *> *second);
vector<Region> *copy_map_to_vector(map<int, vector<Region> *> *map);
    vector<Region*> *copy_map_to_vector3(map<int, vector<Region> *> *map);
    vector<Region*> *copy_map_to_vector2(map<int, set<Region*> *> *map);
set<Region *> *copy_map_to_set(map<int, set<Region *> *> *map);
set<Region *> *initial_regions(map<int, set<Region *> *> *reg);
map<Region *, int> *get_regions_map(map<int, set<Region *> *> *net);

void print(Region &region);
void println(Region &region);
void println(set<Region *> &regions);
void print(map<int, set<Region *> *> &net);
bool is_bigger_than_or_equal_to(Region *, set<int> *);
bool are_equal(Region *r1, Region *r2);
bool contains(set<Region *> *, Region *);
    bool contains(vector<Region*> *, Region *);
void print_ts_dot_file(string file_path,map<int, int> *aliases);
void print_pn_dot_file(map<int, set<Region *> *> *net,
                       map<int, set<Region *> *> *post_regions,
                       map<int, int>* aliases, string file_name);
};