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
    map<int, set<Region> *> *non_minimal_regions_per_level;

public:
    FCPN_decomposition();
    ~FCPN_decomposition();
    set<set<Region *> *> *search(int number_of_events,
                                 const set<Region *>& regions,
                                 const string& file,
                                 Pre_and_post_regions_generator *pprg,
                                 map<int, ER> *ER, int level);
};