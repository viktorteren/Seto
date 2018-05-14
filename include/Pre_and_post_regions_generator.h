//
// Created by Viktor on 20/04/2018.
//


#pragma once

#include "Utilities.h"

class Pre_and_post_regions_generator {
public:
	Pre_and_post_regions_generator(vector<Region> * reg);
	Pre_and_post_regions_generator(vector<Region> * reg,vector<Region>* candidate_regions,map<int,ER>* Er_set,set<int>* events_to_split);
	~Pre_and_post_regions_generator();
	map<int, set<Region*> *> *get_pre_regions();
	map<int, set<Region*> *> *get_post_regions();
	map<int,ER>* get_new_ER();
	map<int, int>& get_events_alias();
	void create_post_regions();
private:
	vector<Region> *regions;
	map<int, set<Region*> *> *pre_regions;
	map<int, set<Region*> *> *post_regions = nullptr;
    //todo usa questa per avere una sola occorrenza delle nuove regioni splittate
    set<Region*>* added_regions_ptrs;
    set<int>* events_to_split;
    //alias vecchio -> nuovo
    map<int,int>* events_alias;
	map<int,ER>* er_set= nullptr;

	void create_pre_and_post_regions(vector<Region>* candidate_regions);
	//void create_pre_and_post_regions_with_splitting(map<int,Region>* candidate_regions);
	bool is_pre_region(Edges_list*list, Region *region);
	bool is_post_region(Edges_list *list, Region *region);
	void remove_bigger_regions(Region &new_region);
	map<int,ER>* create_ER_after_splitting(map<int,ER>*,set<int>*);

};
