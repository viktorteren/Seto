/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../libs/pblib/pb2cnf.h"
#include "../include/Pre_and_post_regions_generator.h"

class GreedyRemoval {
private:
    static bool check_EC(vector<int>* vec,
                         vector<set<Region*>*> *SM_vector,
                         map<int, set<Region *> *> *pre_regions,
                         map<int, ER> *ER);
    static bool exists_next(vector<int>* vec, int max);
    static vector<int>* next_set(vector<int>* vec, int max);

public:
    static void minimize(set<set<Region *>*> *SMs,
                         Pre_and_post_regions_generator *pprg,
                         map<int, ER> *new_ER,
                         map<int, set<Region *> *> *pre_regions);
    static void minimize_sat(set<set<Region *>*> *FCPNs,
                             set<set<Region *>*> *SMs,
                             map<int, ER> *ER,
                             map<int, set<Region *> *> *pre_regions,
                            const string& file);
    static void minimize_sat_SM(set<set<Region *>*> *SMs,
                             map<int, ER> *ER,
                             map<int, set<Region *> *> *pre_regions,
                             const string& file);
    static void minimize_sat_SM_exact(set<set<Region *>*> *SMs,
                                        map<int, ER> *ER,
                                        map<int, set<Region *> *> *pre_regions);


};