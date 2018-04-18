//
// Created by viktor on 18/04/18.
//

#include "Irredundant_sets_creation_module.h"
#include "Essential_region_search.h"

Irredundant_sets_creation_module::Irredundant_sets_creation_module(map<int, vector<Region*> *>* pre_reg){
	pre_regions = pre_reg;
	not_essential_regions = new map<int, vector<Region*> *>();
	Essential_regions_search *ers = new Essential_regions_search(pre_regions);
	essential_regions = ers->search();
	search_events_with_not_essential_regions();
}

void Irredundant_sets_creation_module::search_events_with_not_essential_regions() {
	for(auto record: *pre_regions){
		cout << "nuovo evento: " << record.first << endl;
		for(auto region: *record.second){
			cout << &(*region) << endl;
			//regione non essenziale
			if(essential_regions.find(&(*region)) == essential_regions.end()){
				if (not_essential_regions->find(record.first) == not_essential_regions->end()) {
					(*not_essential_regions)[record.first] = new vector<Region*>();
				}
				(*not_essential_regions)[record.first]->push_back(region);
			}
		}
	}
	//per debug:
	/*cout << "prova" << endl;
	for(auto record: *not_essential_regions){
		cout << "evento: " << record.first << endl;
		for(auto region: *record.second){
			cout << &(*region) << endl;
		}
	}*/
}