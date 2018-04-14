//
// Created by viktor on 14/04/18.
//

#include "Minimal_pre_region_generator.h"

#ifndef PROGETTO_SSE_ESSENTIAL_REGION_SEARCH_H
#define PROGETTO_SSE_ESSENTIAL_REGION_SEARCH_H

#endif //PROGETTO_SSE_ESSENTIAL_REGION_CALCULATOR_H

using namespace pre_region_gen;

namespace essential_regions{

    map<int, vector<Region*> *> * pre_regions;
    class Essential_regions_search{
        public:
            explicit Essential_regions_search(map<int, vector<Region*> *>* pre_regions);
            ~Essential_regions_search();
            set<Region *> search();

        private:
            set<int> regions_union(vector<Region*> vec);
    };

}