//
// Created by ciuchino on 18/04/18.
//

#pragma once
#include "minisat/core/Solver.h"
#include "minisat/mtl/Vec.h"
#include "minisat/core/Dimacs.h"
#include "minisat/core/SolverTypes.h"
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
typedef set<Region *> SM;
typedef std::pair<int, int> Edge;
typedef set<int> *ER;
typedef set<Edge *> Edges_list;

const int OK = 0;
const int NOCROSS = 1;
const int EXIT_NOCROSS = 2;
const int ENTER_NOCROSS = 3;

extern bool print_step_by_step;
extern bool print_step_by_step_debug;
extern bool decomposition;
extern bool decomposition_debug;
extern bool decomposition_output;
extern bool log_file;
extern bool irredundant_search;
extern map<int, Region*>* aliases_region_pointer;
extern map<Region*, int>* aliases_region_pointer_inverted;
extern int max_alias_decomp;
extern int num_clauses;
extern map<pair<Region*, Region*>, bool> *overlaps_cache;

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
    set<Region *> *copy_map_to_set(map<int, Region *> *map);
    set<Region *> *initial_regions(map<int, set<Region *> *> *reg); //regions that contains initial states
    set<Region *> *initial_regions(map<int, Region *> *reg);
    map<Region *, int> *get_regions_map(map<int, set<Region *> *> *net);
    map<Region *, int> *get_regions_map(map<int, Region *> *net);

    void print(Region &region);
    void println(Region &region);
    void println(set<Region *> &regions);
    void print(map<int, set<Region *> *> &net);
    bool is_bigger_than_or_equal_to(Region *, set<int> *);
    bool are_equal(Region *r1, Region *r2);
    bool are_equal(const Region& r1, Region r2);
    bool contains(set<Region *> *, Region *);
    bool contains(vector<Region*> *, Region *);
    void print_ts_dot_file(string file_path,map<int, int> *aliases);
    void print_pn_dot_file(map<int, set<Region *> *> *pre_regions,
                       map<int, set<Region *> *> *post_regions,
                       map<int, int>* aliases, string file_name);
    void print_sm_dot_file(map<int, Region *> *pre_regions,
                           map<int, Region *> *post_regions,
                           map<int, int>* aliases, string file_name);
    //Minisat::vec<Minisat::Lit>* region_to_clause(map<int, set<Region *> *> *irredundant_regions);
    vector<vector<int>*>* add_regions_clauses_to_solver(map<int, set<Region *> *> *regions_map); //s vill recieve new clauses and uncovered_states the states to cover
    map<int, set<Region *> *>* merge_2_maps(map<int, set<Region *> *> *first, map<int, set<Region *> *> *second);
    map<int, set<Region *> *>* merge_2_maps(map<int, set<Region *> *> *first, map<int, Region *> *second);
    map<int, set<Region *> *>* merge_2_maps(map<int, Region *> *first, map<int, Region *> *second);
    string convert_to_dimacs(string file_path, int num_var, int num_clauses, vector<vector<int>*>* clauses, set<set<int>*>* new_results_to_avoid);
    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses, set<set<int>*>* new_results_to_avoid);
    set<vector<int>*>* overlapping_regions_clause(set<Region *> *overlapping_regions);
    void region_mapping(Region* region);
    void add_region_to_SM(set<Region*>* SM, Region* region);
    void print_SM(set<Region *>* SM);
    bool check_sat_formula_from_dimacs(Minisat::Solver& solver, const string& file_path);
    bool is_excitation_closed(map<int, set<Region *> *> *pre_regions, map<int, ER> *ER_set );
    string remove_extension(string path);
    bool is_initial_region(Region *);
    int getStatesSum(set<SM*>* SMs);
    int getNumStates(SM* sm);
};
