//
// Created by viktor on 12/04/18.
//

#ifndef PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H
#define PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H

#endif //PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H

#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <cassert>
#include <tuple>
#include <map>
#include <string>
#include <set>

namespace pre_region_gen {

    using namespace std;

    typedef std::pair<int, int> Edge;
    typedef set<int> Region;
    typedef set<int> *ER;
    typedef vector<Edge> Lista_archi;
    typedef std::map<int, Lista_archi> My_Map;


    const int OK = 0;
    const int NOCROSS = 1;
    const int EXIT_NOCROSS = 2;
    const int ENTER_NOCROSS = 3;
    const int EXIT = 2;
    const int ENTER = 3;

    int stati, eventi;

    class Minimal_pre_region_generator {

    public:

        Minimal_pre_region_generator(int num_stati,int num_eventi);
        ~Minimal_pre_region_generator();

        map<int, vector<Region *> *> *generate();

        vector<ER> *ER_set;
        vector<Region> *regions;
        vector<Region> *queue_temp_regions;

        struct Branches_states_to_add {
            set<int> *states_to_add_exit_or_enter;
            set<int> *states_to_add_nocross;
        };

        My_Map* ts_map;
        map<int, Branches_states_to_add> *map_states_to_add;
        Branches_states_to_add *struct_states_to_add;
        set<int> *states_to_add_enter;
        set<int> *states_to_add_exit;
        set<int> *states_to_add_nocross;
        map<int, vector<Region *> *> *pre_regions;


        void printRegion(const Region &region);

        ER createER(int event);

        bool is_pre_region(Lista_archi *list, Region *region, int event);

        int branch_selection(Lista_archi *list, Region *region, int event);

        bool minimal_region(Region &new_region);

        void remove_bigger_regions(Region &new_region);

        bool region_in_queue(Region &new_region);

        void expand(Region *region, int event);

        void create_pre_regions();

    };

}
