//
// Created by Viktor on 20/04/2018.
//

#include "Pre_regions_generator.h"

Pre_regions_generator::Pre_regions_generator(vector<Region> * reg){
	regions = reg;
	pre_regions= new map < int , vector<Region*>* > ();
}

Pre_regions_generator::~Pre_regions_generator(){}

bool Pre_regions_generator::is_pre_region(List_edges *list, Region *region, int event) {
	for(auto t: *list){
		if( region->find(t.first) != region->end()){ //il primo stato appartiene alla regione
			if(region->find(t.second) == region->end()) { //il secondo stato non appartiene alla regione
				return true;
			}
			else
				return false;
		} else
			return false;
	}
	return false;
}

bool Pre_regions_generator::minimal_region(Region& new_region) {
	int cont;
	for (auto region: *regions) {
		cont = 0;
		if(region.size() <= new_region.size()){
			for (auto state: region) {
				if(new_region.find(state) == new_region.end()){
					break;
				}
				else{
					cont ++;
				}
			}
			if(cont == region.size())
				return false;
		}
	}
	return true;
}

void Pre_regions_generator::remove_bigger_regions(Region& new_region){
	int cont;
	Region region;


	for(int i = 0; i< regions->size(); i++){
		region = regions->at(i);
		cont = 0;
		if(region.size() > new_region.size()){
			for (auto state: new_region) {
				if(region.find(state) == region.end()){
					break;
				}
				else{
					cont ++;
				}
			}
			if(cont == region.size()){
				cout << "eliminazione regione vecchia" << endl;
				//remove old too big region
				regions->erase(regions->begin()+i);
				i--;
			}

		}
	}
}

map<int, vector<Region*> *> * Pre_regions_generator::create_pre_regions(){
	cout << "--------------------------------------------------- CREATION OF PRE-REGIONS --------------------------------------------" << endl;
	//per ogni evento
	//per ogni regione
	//guardo se è una pre-regione per tale evento
	//se si aggiungo alla mappa

	vector<Region>::iterator it;
	for(auto record: *ts_map){
		//cout << "evento: " << record.first << endl;
		/*for(auto region: *regions){
			//printRegion(region);
			if(is_pre_region(&record.second, &region, record.first)){*/
		for(it=regions->begin(); it!=regions->end();++it){
			Region* region= &(*it);
			if(is_pre_region(&record.second, region, record.first)){
				//aggiungo la regione alla mappa
				if (pre_regions->find(record.first) == pre_regions->end()){
					(*pre_regions)[record.first] = new vector<Region *> ();
				}

				cout << &region << endl;
				//cout << ((*pre_regions)[record.first]) << endl;
				(*pre_regions)[record.first]->push_back(region);
				cout << "pre_regions size " << (*pre_regions).size() << endl;
				/* for(auto region: *((*pre_regions)[record.first])){
					 cout << "Evento: " << record.first << endl;
					 printRegion(*region);
				 }*/
			}
		}

	}

	for(auto record: *pre_regions){
		cout << "Event: " << record.first << endl;
		for(auto region: *record.second){
			Utilities::print(*region);
		}
	}
	return pre_regions;
}