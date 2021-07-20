/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"

class FCPN_composition {
private:
    struct edge{
        map<set<Region *>*, set<Region *>> start;
        int event;
        map<set<Region *>*, set<Region *>> end;
    };
public:
    static void compose(set<set<Region *>*> *fcpn_set,
                        map < set<Region *> *, map<int, set<Region*> *> * > *map_FCPN_pre_regions,
                        map < set<Region *> *, map<int, set<Region*> *> * > *map_FCPN_post_regions);
};