/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../pblib/pb2cnf.h"
#include <algorithm>
#include <iomanip>

class GreedyRemoval {
private:

public:
    static void minimize(set<set<Region *>*> *SMs, Pre_and_post_regions_generator *pprg, map<int, ER> *new_ER, map<int, set<Region *> *> *pre_regions);
};