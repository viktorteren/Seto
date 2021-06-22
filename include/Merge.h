/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../pblib/pb2cnf.h"
#include <algorithm>
#include <iomanip>
#include "../include/Pre_and_post_regions_generator.h"

class Merge {

private:
    map < SM * , int > SMs_map;
    map < int, SM * > SMs_map_inverted;
    int counter = 0;
    map < SM * , set<int> * > *events_to_remove_per_SM;

public:
    Merge(set<SM *> *SMs,
          vector<vector<int32_t> *> *clauses,
          int number_of_events,
          map<SM *, map<int, Region *> *> *map_of_SM_pre_regions,
          map<SM *, map<int, Region *> *> *map_of_SM_post_regions,
          const string& file,
          Pre_and_post_regions_generator *pprg);

    static void print_after_merge(set<SM *> *SMs,
                        map<SM *, map<int, Region *> *> *map_of_SM_pre_regions,
                        map<SM *, map<int, Region *> *> *map_of_SM_post_regions,
                        map<int, int> *aliases,
                        const string& file);

    ~Merge();

    string dimacs_file;

};
