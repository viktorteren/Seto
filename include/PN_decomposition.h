/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include "../pblib/pb2cnf.h"
#include "../include/Pre_and_post_regions_generator.h"
#include "../include/PN_composition.h"
#include "BDD_encoder.h"


class PN_decomposition {
private:
    static int k_search_event_offset(int num_events, int num_regions, int num_FCPNs);
    static int k_search_region_offset(int num_events, int num_regions, int num_FCPNs);
    static void check_EC_and_structure(map<int, ER> *ER,
                                       map<set<Region *> *, map<int, set<Region *> *> *>* map_of_FCPN_pre_regions,
                                       map<set<Region *> *, map<int, set<Region *> *> *>* map_of_FCPN_post_regions,
                                       map<int, set<Region *>*> *pre_regions_map,
                                       map<Region *, set<int> *>* region_ex_event_map,
                                       set<set<Region *> *>* pn_set);

public:
    static set<set<Region *> *> *search(int number_of_events,
                                        const set<Region *>& regions,
                                        const string& file,
                                        Pre_and_post_regions_generator *pprg,
                                        map<int, ER> *ER,
                                        map<int, int> *aliases,
                                        set<set<Region *>*>* SMs);
    static set<set<Region *> *> *search_k(int number_of_events,
                                          set<Region *> *regions,
                                          const string& file,
                                          Pre_and_post_regions_generator *pprg,
                                          map<int, ER> *ER,
                                          map<int, int> *aliases,
                                          set<set<Region *>*>* SMs,
                                          BDD_encoder *be);
};