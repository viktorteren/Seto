//
// Created by viktor on 14/04/18.
//


#ifndef PROGETTO_SSE_ESSENTIAL_REGION_SEARCH_H
#define PROGETTO_SSE_ESSENTIAL_REGION_SEARCH_H

#endif //PROGETTO_SSE_ESSENTIAL_REGION_CALCULATOR_H

#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <tuple>
#include <map>
#include <string>
#include <set>

using namespace std;

typedef std::pair<int, int> Edge;
typedef set<int> Region;
typedef set<int> *ER;
//typedef vector<Edge> List_edges;
// typedef std::map<int, List_edges> My_Map;

/*const int OK = 0;
const int NOCROSS = 1;
const int EXIT_NOCROSS = 2;
const int ENTER_NOCROSS = 3;*/

//namespace essential_regions{

    class Essential_regions_search{
        public:
            explicit Essential_regions_search(map<int, vector<Region*> *>* pre_regions);
            ~Essential_regions_search();
            set<Region *> search();

        private:
            map<int, vector<Region*> *> * pre_regions;
            set<int> regions_union(vector<Region*> vec);
    };

//}