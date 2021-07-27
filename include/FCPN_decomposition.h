/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../pblib/pb2cnf.h"
#include "../include/Pre_and_post_regions_generator.h"
#include "../include/FCPN_composition.h"


class FCPN_decomposition {
private:

public:
    static set<set<Region *> *> *search(int number_of_events,
                                 const set<Region *>& regions,
                                 const string& file,
                                 Pre_and_post_regions_generator *pprg,
                                 map<int, ER> *ER,
                                 map<int, int> *aliases);
};