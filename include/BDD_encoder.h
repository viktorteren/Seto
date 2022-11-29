/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once

#include <set>
#include <map>
#include "Utilities.h"
#include <cuddObj.hh>


using namespace std;

class BDD_encoder {
public:
    BDD_encoder(map<int, set<set<int> *> *> *pre_regions, map<int, ER> *ER_map);
    ~BDD_encoder();
    /**
     * @brief This method should take valid region combinations for each event and encode these in boolean formulas, for
     * each FCPN
     * @param num_fcpns Number of FCPNs
     */
    void encode(set<Region *> *regions, map<Region *, int> *regions_alias_mapping, map<int, set<set<int> *> *> *pre_regions);
    set<set<int>> * get_set_of_EC_clauses(map<Region *, int> *regions_alias_mapping);
    vector<vector<int32_t> *>* get_clauses();
private:
    map<int, set<set<Region *>>*> *valid_sets;
    map<int, set<set<Region *>>*> *invalid_sets;
    static bool test_if_enough(ER er, set<Region *>* regions);
    static bool test_if_enough(ER er, Region * region);
    map<int, set<set<Region *>*>*>* map_of_EC_clauses;
    vector<vector<int32_t> *> *clauses;
};



