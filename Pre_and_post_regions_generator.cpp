//
// Created by Viktor on 20/04/2018.
//

#include "Pre_and_post_regions_generator.h"

Pre_and_post_regions_generator::Pre_and_post_regions_generator(vector<Region> * reg){
	regions = reg;
	pre_regions= new map < int , set<Region*>* > ();
	post_regions= new map < int , set<Region*>* > ();
	create_pre_and_post_regions();
}

Pre_and_post_regions_generator::~Pre_and_post_regions_generator(){}

bool Pre_and_post_regions_generator::is_pre_region(List_edges *list, Region *region, int event) {
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

bool Pre_and_post_regions_generator::is_post_region(List_edges *list, Region *region, int event) {
	for(auto t: *list){
		if( region->find(t.first) == region->end()){ //il primo stato non appartiene alla regione
			if(region->find(t.second) != region->end()) { //il secondo stato appartiene alla regione
				return true;
			}
			else
				return false;
		} else
			return false;
	}
	return false;
}

void Pre_and_post_regions_generator::remove_bigger_regions(Region& new_region){
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

void Pre_and_post_regions_generator::create_pre_and_post_regions(){
	cout << "------------------------------------------------------------ DELETING OF NON MINIMAL REGIONS -------------------------------------------" << endl;
	vector<Region>::iterator it;
	for(it=regions->begin(); it!=regions->end();++it){
		Region* region= &(*it);
		remove_bigger_regions(*region);
	}

	cout << "--------------------------------------------------- CREATION OF PRE-REGIONS AND POST-REGIONS --------------------------------------------" << endl;
	//per ogni evento
	//per ogni regione
	//guardo se è una pre-regione per tale evento
	//se si aggiungo alla mappa

	//todo: prima di creare le pre-regioni verificare se le regioni sono minime


	for(auto record: *ts_map){
		//cout << "evento: " << record.first << endl;
		for(it=regions->begin(); it!=regions->end();++it){
			Region* region= &(*it);
			if(is_pre_region(&record.second, region, record.first)){
				//se l'evento non era presente nella mappa creo lo spazioo per il relativo set di regioni
				if (pre_regions->find(record.first) == pre_regions->end()){
					(*pre_regions)[record.first] = new set<Region *> ();
				}

				//cout << &region << endl;
				//cout << ((*pre_regions)[record.first]) << endl;
				//aggiungo la regione alla mappa
				if((*pre_regions)[record.first]->find(region) == (*pre_regions)[record.first]->end()){
					(*pre_regions)[record.first]->insert(region);
					/*cout << "inserisco " << &(*region) << endl;
					Utilities::println(*region);*/
				}
			}
			if(is_post_region(&record.second, region, record.first)){
				//aggiungo la regione alla mappa
				if (post_regions->find(record.first) == post_regions->end()){
					(*post_regions)[record.first] = new set<Region *> ();
				}
				if((*post_regions)[record.first]->find(region) == (*post_regions)[record.first]->end()){
					(*post_regions)[record.first]->insert(region);
				}

			}
		}

	}

	//Per DEBUG:
	cout << "Pre regions:" << endl;
	for(auto record: *pre_regions){
		cout << "Event: " << record.first << endl;
		for(auto region: *record.second){
			Utilities::println(*region);
		}
	}
	/*cout  << "Post regions:" << endl;
	for(auto record: *post_regions){
		cout << "Event: " << record.first << endl;
		for(auto region: *record.second){
			Utilities::println(*region);
		}
	}*/

}

map<int, set<Region*> *> * Pre_and_post_regions_generator::get_post_regions(){
	return post_regions;
}

map<int, set<Region*> *> * Pre_and_post_regions_generator::get_pre_regions(){
	return pre_regions;
}