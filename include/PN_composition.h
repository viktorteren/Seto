/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once
#include "Utilities.h"

class PN_composition {
private:

public:
    static void compose(set<set<Region *>*> *fcpn_set,
                        map < set<Region *> *, map<int, set<Region*> *> * > *map_FCPN_pre_regions,
                        map < set<Region *> *, map<int, set<Region*> *> * > *map_FCPN_post_regions,
                        map <int, int> *aliases,
                        string file_path);
    static void compose(set<set<Region *>*> *fcpn_set,
                        map < set<Region *> *, map<int, Region*> * > *map_FCPN_pre_regions,
                        map < set<Region *> *, map<int, Region*> * > *map_FCPN_post_regions,
                        map <int, int> *aliases,
                        string file_path);
};