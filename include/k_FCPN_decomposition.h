/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../pblib/pb2cnf.h"
#include <algorithm>
#include <iomanip>
#include "../include/Pre_and_post_regions_generator.h"


class k_FCPN_decomposition {
private:
    int encoded_region(Region *reg, int current_FCPN_number) const;
    pair<int, Region *> decoded_region(int encoded_value) const;
    __attribute__((unused)) int encoded_event(int event, int current_FCPN_number) const;
    __attribute__((unused)) pair<int, int> decoded_event(int encoded_value) const;
    map<Region*, int> *reg_map;
    map<int, Region*> *inverse_reg_map;
    int number_of_regions;
    int number_of_FCPNs;
    int number_of_events;

public:
    k_FCPN_decomposition(int number_of_events,
                       set<Region*> *regions,
                       const string& file,
                       Pre_and_post_regions_generator *pprg,
                       map<int, int> *aliases,
                       map<int, ER> *ER);
    ~k_FCPN_decomposition();
};