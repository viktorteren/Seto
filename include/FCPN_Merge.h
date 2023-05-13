/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "Utilities.h"
#include <pblib/pb2cnf.h>
#include "../include/Pre_and_post_regions_generator.h"

class FCPN_Merge {

private:

public:
    FCPN_Merge(set<SM *> *FCPNs,
          int number_of_events,
          map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_pre_regions,
          map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_post_regions,
          const string& file,
          map<int, int> *aliases);

    static void print_after_merge(set<set<Region *> *> *FCPNs,
                                  map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_pre_regions,
                                  map<SM *, map<int, set<Region*> *> *> *map_of_FCPN_post_regions,
                                  map<int, int> *aliases,
                                  const string& file);
    ~FCPN_Merge();

    string dimacs_file;

};
