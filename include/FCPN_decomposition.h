/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../pblib/pb2cnf.h"
#include <algorithm>
#include <iomanip>
#include "../include/Pre_and_post_regions_generator.h"


class FCPN_decomposition {
private:
    set<set<Region *>*> *computation_with_missing_FCPN(map<int, set<Region *>*>*pre_regions_map,
                                       map<int, set<Region *> *>*post_regions_map,
                                       set<Region *> *regions,
                                       Pre_and_post_regions_generator *pprg,
                                       set<set<Region *>*> *fcpn_set,
                                       int number_of_events,
                                       const string& file);
    bool new_non_minimal_regions_used = false;

public:
    FCPN_decomposition(int number_of_events,
                       set<Region*> *regions,
                       const string& file,
                       Pre_and_post_regions_generator *pprg,
                       map<int, int> *aliases,
                       map<int, ER> *ER);
    ~FCPN_decomposition();
};