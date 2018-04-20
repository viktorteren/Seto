//
// Created by Viktor on 20/04/2018.
//

#ifndef PROGETTO_SSE_PRE_REGIONS_GENERATOR_H
#define PROGETTO_SSE_PRE_REGIONS_GENERATOR_H

#include "Utilities.h"

class Pre_regions_generator {
public:
	Pre_regions_generator(vector<Region> * reg);
	~Pre_regions_generator();
	map<int, vector<Region*> *> * create_pre_regions();
private:
	vector<Region> *regions;
	map<int, vector<Region*> *> *pre_regions;
	bool is_pre_region(List_edges *list, Region *region, int event);
	bool minimal_region(Region &new_region);
	void remove_bigger_regions(Region &new_region);
};


#endif //PROGETTO_SSE_PRE_REGION_GENERATOR_H
