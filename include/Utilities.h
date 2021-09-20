/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once
#include "minisat/core/Solver.h"
#include "minisat/mtl/Vec.h"
#include "minisat/core/Dimacs.h"
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
#include <sstream>



using namespace std;

typedef set<int> Region;
typedef set<Region *> SM;
typedef std::pair<int, int> Edge;
typedef set<int> *ER;
typedef set<Edge *> Edges_list;

struct edge{
    map<set<Region *>*, set<Region *>> start;
    string event;
    map<set<Region *>*, set<Region *>> end;
};

struct SM_edge{
    map<set<Region *>*, Region *> start;
    string event;
    map<set<Region *>*, Region *> end;
};

const int OK = 0;
const int NOCROSS = 1;
const int EXIT_NOCROSS = 2;
const int ENTER_NOCROSS = 3;

extern bool print_step_by_step;
extern bool print_step_by_step_debug;
extern bool decomposition;
extern bool decomposition_debug;
extern bool output;
extern bool decomposition_output_sis;
extern bool ts_output;
extern bool ects_output;
extern bool dot_output;
//extern bool log_file;
extern bool python_all;
extern bool info;
extern bool fcptnet;
extern bool no_merge;
extern bool composition;
extern bool aut_output;
extern bool k_fcpn_decomposition;
__attribute__((unused)) extern bool blind_fcpn;
__attribute__((unused)) extern bool fcpn_modified;
__attribute__((unused)) extern bool fcpn_with_levels;
extern bool pn_synthesis;
extern bool no_fcpn_min;
extern map<int, Region*>* aliases_region_pointer;
extern map<Region*, int>* aliases_region_pointer_inverted;
extern map<Region*, int>* sm_region_aliases;
extern int max_alias_decomp;
extern int num_clauses;
extern map<pair<Region*, Region*>, bool> *overlaps_cache;
extern bool benchmark_script;
extern map<set<Region*>, set<int>*> *intersection_cache;
extern int places_after_initial_decomp;
extern int places_after_greedy;

namespace Utilities {
    __attribute__((unused)) set<Region *> *regions_set_union(set<set<Region*>*> *region_set);
    __attribute__((unused)) set<Region *> *regions_set_union(const set<Region*> *region_set1,const set<Region*> *region_set2);
    __attribute__((unused)) set<Region *> *regions_set_union(const set<Region*>& region_set1,const set<Region*>& region_set2);
    Region *regions_union(vector<Region *> *vec);
    Region *regions_union(set<Region *> *vec);
    Region *regions_union(Region *first, Region *second);
    Region *regions_intersection(Region *first, Region *second);
    bool empty_regions_intersection(Region *first, Region *second);
    bool at_least_one_state_from_first_in_second(Region *first, Region *second);
    set<int> *regions_intersection(set<Region *> *set);
    set<int> *regions_intersection(const set<Region *>& regions);
    set<int> *region_difference(set<int> &first, set<int> &second);
    bool empty_region_set_intersection(set<Region *> *first, set<Region *> *second);
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
    void println(set<Region *> *regions);
    void println(set<Region *> &regions);
    void print(map<int, set<Region *> *> &net);
    bool is_bigger_than_or_equal_to(Region *, set<int> *);
    bool are_equal(Region *r1, Region *r2);
    bool are_equal(Region *region1, Region region2);
    bool are_equal(const Region& r1, Region r2);
    __attribute__((unused)) bool contains(const set<Region *>& set, const Region& region);
    /*template<typename T, typename T2>
    bool contains(T bigger_set, T2 smaller_set);*/
    bool contains(set<Region *> *bigger_set, set<Region *> *smaller_set);
    template <typename T>
    bool contains(T, Region *);
    bool contains(set<Region *> bigger_set, const set<Region *>& smaller_set);
    __attribute__((unused)) bool contains(set<int> bigger_set, const set<int>& smaller_set);
    __attribute__((unused)) bool contains(set<int> *bigger_set, set<int> *smaller_set);

    void print_ts_dot_file(string file_path,map<int, int> *aliases);
    //todo: use template to merge the two versions of print_ts_dot_file
    void print_ts_dot_file(string file_path,
                           map <map<set<Region *>*, set<Region *>>, int> *state_aliases,
                           vector<edge> *arcs,
                           const map<set<Region *>*, set<Region *>>& initial_state_TS);
    void print_ts_dot_file(string file_path,
                           map <map<set<Region *>*, Region *>, int> *state_aliases,
                           vector<SM_edge> *arcs,
                           const map<set<Region *>*, Region *>& initial_state_TS);
    void print_ts_aut_file(string file_path, map<int, int> *aliases);
    //todo: use template to merge the two versions of print_ts_aut_file
    void print_ts_aut_file(string file_path, map <map<set<Region *>*, set<Region *>>, int> *state_aliases, vector<edge> *arcs, const map<set<Region *>*, set<Region *>>& initial_state_TS);
    void print_ts_aut_file(string file_path, map <map<set<Region *>*, Region *>, int> *state_aliases, vector<SM_edge> *arcs, const map<set<Region *>*, Region *>& initial_state_TS);
    void print_pn_dot_file(map<int, set<Region *> *> *pre_regions,
                       map<int, set<Region *> *> *post_regions,
                       map<int, int>* aliases,
                       const string& file_name);
    __attribute__((unused)) void print_pn_g_file(map<int, set<Region *> *> *pre_regions,
                         map<int, set<Region *> *> *post_regions,
                         map<int, int>* aliases,
                         const string& file_name);
    void print_sm_dot_file(map<int, Region *> *pre_regions,
                           map<int, Region *> *post_regions,
                           map<int, int>* aliases,
                           string file_name);
    void print_sm_g_file(map<int, Region *> *pre_regions,
                           map<int, Region *> *post_regions,
                           map<int, int>* aliases,
                           string file_name);
    void print_fcpn_dot_file(map<int, set<Region *> *> *pre_regions,
                            map<int, set<Region *> *> *post_regions,
                            map<int, int> *aliases,
                            const string& file_name,
                            int FCPN_number);
    void print_fcpn_dot_file(map<Region *, int> *regions_mapping,
                             map<int, set<Region *> *> *pre_regions,
                             map<int, set<Region *> *> *post_regions,
                             map<int, int> *aliases,
                             const string&  file_name,
                             int FCPN_number);
    vector<vector<int>*>* add_regions_clauses_to_solver(map<int, set<Region *> *> *regions_map); //s vill recieve new clauses and uncovered_states the states to cover
    map<int, set<Region *> *>* merge_2_maps(map<int, set<Region *> *> *first, map<int, set<Region *> *> *second);
    __attribute__((unused)) map<int, set<Region *> *>* merge_2_maps(map<int, set<Region *> *> *first, map<int, Region *> *second);
    map<int, set<Region *> *>* merge_2_maps(map<int, Region *> *first, map<int, Region *> *second);

    __attribute__((unused)) string convert_to_dimacs(string file_path, int num_var, int num_clauses, vector<vector<int>*>* clauses, set<set<int>*>* new_results_to_avoid);
    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses, vector<set<int>>* new_results_to_avoid);
    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses);
    string convert_to_dimacs_simplified(const string& file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses);
    set<vector<int>*>* overlapping_regions_clause(set<Region *> *overlapping_regions);
    void region_mapping(Region* region);
    __attribute__((unused)) void add_region_to_SM(set<Region*>* SM, Region* region);
    __attribute__((unused)) void print_SM(set<Region *>* SM);
    bool check_sat_formula_from_dimacs(Minisat::Solver& solver, const string& file_path);
    __attribute__((unused)) bool check_sat_formula_from_dimacs2(Minisat::Solver& solver, const string& file_path);
    __attribute__((unused)) bool check_ER_intersection(int event, set<Region*> *pre_regions_set, map<int, ER> *ER_set);
    __attribute__((unused)) bool check_ER_intersection_with_mem(int event, set<Region*> *pre_regions_set, map<int, ER> *ER_set);
    bool check_ER_intersection_with_mem(int event, const set<Region*>& pre_regions_set, map<int, ER> *ER_set);
    void clear_ER_intersection_cache();
    __attribute__((unused)) bool check_ER_intersection_cache(set<Region*> *pre_regions_set);
    bool is_excitation_closed(map<int, set<Region *> *> *pre_regions, map<int, ER> *ER_set );
    string remove_extension(string path);
    bool is_initial_region(Region *);
    int getStatesSum(set<SM*>* SMs);
    double getStatesAvg(set<SM*>* SMs);
    double getStatesVar(set<SM*>* SMs);
    int getNumStates(SM* sm);
    int getTransitionsSum(map<SM*, map<int, Region *>*>* pre_regions);
    double getTransitionsAvg(map<SM*, map<int, Region *>*>* pre_regions);
    double getTransitionsVar(map<SM*, map<int, Region *>*>* pre_regions);
    void create_dimacs_graph(int num_regions, vector<vector<int32_t> *> *clauses);
    void read_SMs(const string& file, set<SM*>* SMs, map<int, Region *> &aliases);
    string get_file_name(string path);
    int getMaxPTSum(map<SM*, map<int, Region *>*>* pre_regions);
    int getMaxTransitionsNumber(map<SM*, map<int, Region *>*>* pre_regions);
    int getMaxAlphabet(map<SM*, map<int, Region *>*>* pre_regions, map<int, int> *label_aliases);
    int getMaxAlphabet(map<SM*, map<int, set<Region *>*>*>* pre_regions, map<int, int> *label_aliases);
    double getAvgAlphabet(map<SM*, map<int, set<Region *>*>*>* pre_regions, map<int, int> *label_aliases);
    __attribute__((unused)) bool checkSMUnionForFCPTNet(SM* sm1, SM* sm2, map<int, set<Region*> *> *post_regions);
    __attribute__((unused)) SM* SMUnionForFCPTNetWithCheck(SM* sm1, SM* sm2, map<int, set<Region*> *> *post_regions);
    bool have_common_regions(set<Region *> *first, set<Region *> *second);
    bool checkCommonRegionsBetweenSMs(SM *sm1, SM *sm2);
    void print_clause(vector<int32_t> *clause);
    void print_clause(set<int32_t> *clause);
    map<int, set<Region *>*>* get_map_of_used_regions(set<set<Region *> *> *SMs_or_PNs, map<int, set<Region *> *> *pre_regions);
    __attribute__((unused)) void map_of_pre_regions_union(map<int, set<Region *> *> *map1, map<int, set<Region *> *> *output_map);
    __attribute__((unused)) bool regions_set_intersection_is_empty(const set<Region*> *region_set1,const set<Region*> *region_set2);
    __attribute__((unused)) bool equal_sets(const set<Region*> *region_set1,const set<Region*> *region_set2);
    map<int, set<set<Region *>*>*>* dnf_to_cnf(map<int, set<set<Region *>>*>* er_satisfiable_set);
    set<Region *> regions_set_union_stack(const set<Region*>& region_set1,const set<Region*>& region_set2);
    set<set<Region *>*>* dnf_to_cnf_core(set<set<Region *>>*cl_set, set<set<Region *>>::iterator it);
    vector<set<Region *>> *split_not_connected_regions(set<Region *> *pn, map<int, set<Region *> *> *connections);
    bool are_connected(const set<Region *>& first, const set<Region *>& second, map<int, set<Region *> *> *connections);
    bool is_a_region(set<int> *set_of_states);
};
