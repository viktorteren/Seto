//
// Created by Viktor on 20/04/2018.
//


#pragma once

#include "Utilities.h"

class Pre_and_post_regions_generator {
public:
	Pre_and_post_regions_generator(vector<Region> * reg);
	Pre_and_post_regions_generator(vector<Region> * reg,vector<Region>* candidate_regions,vector<ER>* Er_set,set<int>* events_to_split);
	~Pre_and_post_regions_generator();
	map<int, set<Region*> *> *get_pre_regions();
	map<int, set<Region*> *> *get_post_regions();

private:
	vector<Region> *regions;
	map<int, set<Region*> *> *pre_regions;
	map<int, set<Region*> *> *post_regions;
    set<Region*>* added_regions_ptrs;
    set<int>* events_to_split;
    set<pair<int,int>*>* events_alias;

	void create_pre_and_post_regions(vector<Region>* candidate_regions);
	//void create_pre_and_post_regions_with_splitting(map<int,Region>* candidate_regions);
	bool is_pre_region(List_edges *list, Region *region);
	bool is_post_region(List_edges *list, Region *region);
	void remove_bigger_regions(Region &new_region);

};
