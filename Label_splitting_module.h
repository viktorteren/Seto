//
// Created by ciuchino on 13/04/18.
//

#ifndef PROGETTO_SSE_LABEL_SPLITTING_MODULE_H
#define PROGETTO_SSE_LABEL_SPLITTING_MODULE_H




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
typedef vector<Edge> List_edges;
// typedef std::map<int, List_edges> My_Map;

/*const int OK = 0;
const int NOCROSS = 1;
const int EXIT_NOCROSS = 2;
const int ENTER_NOCROSS = 3;*/

class Label_splitting_module{
public:
    Label_splitting_module(map<int, vector<Region*> *>* pre_regions,vector<ER> *er_set);
    ~Label_splitting_module();
    bool is_exitation_closed();

private:
    map<int, vector<Region*> *> * pre_regions;
    vector<ER> *ER_set;
    //bool is_exitation_closed();
    map<int, set<int>* > * regions_intersection();
    bool is_equal_to(ER er,set<int>* intersection);

};

#endif
