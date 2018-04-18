//
// Created by viktor on 18/04/18.
//

#ifndef PROGETTO_SSE_IRREDUNDANT_SETS_CREATION_MODULE_H
#define PROGETTO_SSE_IRREDUNDANT_SETS_CREATION_MODULE_H

#include "Utilities.h"
using namespace std;

class Irredundant_sets_creation_module {
public:
	explicit Irredundant_sets_creation_module(map<int, vector<Region*> *>* pre_reg);
	~Irredundant_sets_creation_module();
private:
	map<int, vector<Region*> *> * pre_regions;
	set<Region *> essential_regions;
	void search_events_with_not_essential_regions();
	map<int, vector<Region*> *> * not_essential_regions;
};


#endif
