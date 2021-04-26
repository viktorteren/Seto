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

public:
    FCPN_decomposition(int number_of_events,
                       set<Region*> *regions,
                       const string& file,
                       Pre_and_post_regions_generator *pprg,
                       map<int, int> *aliases,
                       map<int, ER> *ER);
    ~FCPN_decomposition();
};