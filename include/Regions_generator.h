//
// Created by viktor on 12/04/18.
//

#ifndef PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H
#define PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H


#include "Utilities.h"


using namespace std;

class Region_generator {

public:
    Region_generator();
    ~Region_generator();

    map<int, vector<Region> *>* generate();

    vector<ER>* get_ER_set();

    struct Branches_states_to_add {
        set<int> *states_to_add_exit_or_enter= nullptr;
        set<int> *states_to_add_nocross= nullptr;
        ~Branches_states_to_add(){
        	if(states_to_add_exit_or_enter!= nullptr) delete states_to_add_exit_or_enter;
        	if(states_to_add_nocross != nullptr) delete states_to_add_nocross;
		}
    };

    map<int, vector<Region*> *>* get_middle_set_of_states();
    map<int,vector< int >* >* get_number_of_bad_events();

private:
    //int events,states;

    vector<ER> *ER_set;
    map<int,vector<Region>*> *regions;
    vector<Region> *queue_temp_regions;

    map<int, Branches_states_to_add*> *map_states_to_add;
    Branches_states_to_add *struct_states_to_add;
    set<int> *states_to_add_enter;
    set<int> *states_to_add_exit;
    set<int> *states_to_add_nocross;

    map<int, vector<Region*> *>* middle_set_of_states;
    //per ogni evento vector di coppie numero eventi che violano la regione e ptr_regione
    map<int,vector< int > *> *number_of_bad_events;

    ER createER(int event);
    int branch_selection(List_edges *list, Region *region, int event);
    bool region_in_queue(Region &new_region,int init_pos);
    void expand(Region *region, int event,bool is_ER,int init_pos);
    void set_middle_set_of_states(map<int,int>* queue_event_index);
    void set_number_of_bad_events(int* event_type,int l,int event);

};

#endif //PROGETTO_SSE_MINIMAL_PRE_REGION_GENERATOR_H