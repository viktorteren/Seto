//
// Created by viktor on 12/04/18.
//

#ifndef PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H
#define PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H


#include "Utilities.h"



//namespace pre_region_gen {

    using namespace std;

    class Region_generator {

    public:

        Region_generator();
        ~Region_generator();

        map<int, vector<Region*> *>* generate();
		vector<Region>* generate_vector();

        vector<ER>* get_ER_set();

        struct Branches_states_to_add {
            set<int> *states_to_add_exit_or_enter= nullptr;
            set<int> *states_to_add_nocross= nullptr;
            /*~Branches_states_to_add(){
                if(states_to_add_exit_or_enter!= nullptr) delete states_to_add_exit_or_enter;
                if(states_to_add_nocross!= nullptr) delete states_to_add_nocross;
            }*/
        };

        map<int, vector<Region*> *>* get_middle_set_of_states();
        vector< pair<int,Region*> >* get_number_of_bad_events();

    private:
        //int events,states;

        vector<ER> *ER_set;
        vector<Region> *regions;
        vector<Region> *queue_temp_regions;

        map<int, Branches_states_to_add> *map_states_to_add;
        Branches_states_to_add *struct_states_to_add;
        set<int> *states_to_add_enter;
        set<int> *states_to_add_exit;
        set<int> *states_to_add_nocross;

        map<int, vector<Region*> *>* middle_set_of_states;
        //coppia numero eventi che violano la regione e ptr_regione
        vector< pair<int,Region*> >* number_of_bad_events;

        void printRegion(const Region &region);
        ER createER(int event);
        int branch_selection(List_edges *list, Region *region, int event);
        bool region_in_queue(Region &new_region);
        void expand(Region *region, int event);
        void set_middle_set_of_states(map<int,int>* queue_event_index);
        void set_number_of_bad_events(int* event_type,int l,set<int>* set);

    };

//}

#endif