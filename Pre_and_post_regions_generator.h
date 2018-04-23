//
// Created by Viktor on 20/04/2018.
//

#ifndef PRE_AND_POST_REGIONS_GENERATOR_H
#define PRE_AND_POST_REGIONS_GENERATOR_H

#include "Utilities.h"

class Pre_and_post_regions_generator {
public:
	Pre_and_post_regions_generator(vector<Region> * reg);
	~Pre_and_post_regions_generator();
	map<int, vector<Region*> *> *get_pre_regions();
	map<int, vector<Region*> *> *get_post_regions();

private:
	vector<Region> *regions;
	map<int, vector<Region*> *> *pre_regions;
	map<int, vector<Region*> *> *post_regions;
	void create_pre_and_post_regions();
	bool is_pre_region(List_edges *list, Region *region, int event);
	bool is_post_region(List_edges *list, Region *region, int event);
	bool minimal_region(Region &new_region);
	void remove_bigger_regions(Region &new_region);
};


#endif //PRE_AND_POST_REGIONS_GENERATOR_H
