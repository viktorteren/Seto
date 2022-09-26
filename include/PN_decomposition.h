/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../pblib/pb2cnf.h"
#include "../include/Pre_and_post_regions_generator.h"
#include "../include/PN_composition.h"


class PN_decomposition {
private:

public:
    static set<set<Region *> *> *search(int number_of_events,
                                 const set<Region *>& regions,
                                 const string& file,
                                 Pre_and_post_regions_generator *pprg,
                                 map<int, ER> *ER,
                                 map<int, int> *aliases,
                                 set<set<Region *>*>* SMs);
    static set<set<Region *> *> *search_k(int number_of_events,
                    const set<Region *>& regions,
                    const string& file,
                    Pre_and_post_regions_generator *pprg,
                    map<int, ER> *ER, map<int, int> *aliases,
                    set<set<Region *>*>* SMs, set<set<Region *>> *EC_clauses);
};