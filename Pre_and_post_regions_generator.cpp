//
// Created by Viktor on 20/04/2018.
//

#include "Pre_and_post_regions_generator.h"

Pre_and_post_regions_generator::Pre_and_post_regions_generator(vector<Region> * reg){
	regions = reg;
	pre_regions= new map < int , set<Region*>* > ();
	post_regions= new map < int , set<Region*>* > ();
	create_pre_and_post_regions(nullptr);
}

Pre_and_post_regions_generator::Pre_and_post_regions_generator(vector<Region> * reg, vector<Region> * candidate_regions){
	regions = reg;
	pre_regions= new map < int , set<Region*>* > ();
	post_regions= new map < int , set<Region*>* > ();
	//create_pre_and_post_regions_with_splitting(candidate_regions);
	create_pre_and_post_regions(candidate_regions);
}

Pre_and_post_regions_generator::~Pre_and_post_regions_generator(){
	delete pre_regions;
	delete post_regions;
}

bool Pre_and_post_regions_generator::is_pre_region(List_edges *list, Region *region) {
	for(auto t: *list){
		if( region->find(t.first) != region->end()){ //il primo stato appartiene alla regione
			return region->find(t.second) == region->end();
		} else
			return false;
	}
	return false;
}

bool Pre_and_post_regions_generator::is_post_region(List_edges *list, Region *region) {
	for(auto t: *list){
		if( region->find(t.first) == region->end()){ //il primo stato non appartiene alla regione
			return region->find(t.second) != region->end();
		} else
			return false;
	}
	return false;
}

void Pre_and_post_regions_generator::remove_bigger_regions(Region& new_region){
	int cont;
	Region region;

	for(unsigned int i = 0; i< regions->size(); i++){
		region = regions->at(i);
		cont = 0;
		if(region.size() > new_region.size()){
			for (auto state: new_region) {
				if(region.find(state) == region.end()){
					break;
				}
				else{
					cont++;
				}
			}
			if(cont == new_region.size()){
				cout << "eliminazione regione vecchia " ;
				Utilities::print(region);
				cout << " a causa di: ";
				Utilities::println(new_region);
				//remove old too big region
				regions->erase(regions->begin()+i);
				i--;
			}

		}
	}
}

void Pre_and_post_regions_generator::create_pre_and_post_regions(vector<Region>* candidate_regions) {
	cout
			<< "------------------------------------------------------------ DELETING OF NON MINIMAL REGIONS -------------------------------------------"
			<< endl;
	vector<Region>::iterator it;
	for (it = regions->begin(); it < regions->end(); ++it) {
		Region *region = &(*it);
		remove_bigger_regions(*region);
	}

	cout
			<< "--------------------------------------------------- CREATION OF PRE-REGIONS AND POST-REGIONS --------------------------------------------"
			<< endl;
	//per ogni evento
	//per ogni regione
	//guardo se è una pre-regione per tale evento
	//se si aggiungo alla mappa
	for (auto record: *ts_map) {
		cout << "_______________evento: " << record.first << endl;
		for (it = regions->begin(); it != regions->end(); ++it) {
			Region *region = &(*it);
			if (is_pre_region(&record.second, region)) {

				//se l'evento non era presente nella mappa creo lo spazioo per il relativo set di regioni
				if (pre_regions->find(record.first) == pre_regions->end()) {
					(*pre_regions)[record.first] = new set<Region *>();
				}
				if (candidate_regions != nullptr) {

					//auto to_erase= new set<Region*>();
					//auto to_add= new set<Region*>();

					//for (it = pre_regions->begin(); it < pre_regions->end(); it++) {
					bool split = false;

					for (auto cand_reg: *candidate_regions) {
						if (Utilities::is_bigger_than(region, &cand_reg)) {
							Region *new_region = Utilities::region_difference(*region, cand_reg);

							(*pre_regions)[record.first]->insert(new_region);

							cout << "ho inserito new region(difference)" << endl;
							Utilities::println(*new_region);

							Region *candidate_region = new set<int>(cand_reg);
							(*pre_regions)[record.first]->insert(candidate_region);

							cout << "ho inserito new region(cand reg)" << endl;
							Utilities::println(cand_reg);

							split = true;
						}
					}
					if(!split){
						(*pre_regions)[record.first]->insert(region);
					}

				}
				else{
					(*pre_regions)[record.first]->insert(region);
				}



			}
			if (is_post_region(&record.second, region)) {
				//aggiungo la regione alla mappa
				if (post_regions->find(record.first) == post_regions->end()) {
					(*post_regions)[record.first] = new set<Region *>();
				}
				(*post_regions)[record.first]->insert(region);
			}
		}

	}

	if (candidate_regions != nullptr) {



			for (auto record: *post_regions) {
				for (auto region: *record.second)
					for (auto cand_reg: *candidate_regions) {
						if (Utilities::is_bigger_than(region, &cand_reg)) {
							//auto difference = Utilities::region_difference(*region, cand_reg);
                            Region *new_region =  Utilities::region_difference(*region, cand_reg);

							(*post_regions)[record.first]->insert(new_region);

							cout << "ho inserito new region(difference)" << endl;
							Utilities::println(*new_region);

                            Region*candidate_region= new set<int>(cand_reg);
							(*post_regions)[record.first]->insert(candidate_region);

							cout << "ho inserito new region(cand reg)" << endl;
							Utilities::println(cand_reg);


							(*post_regions)[record.first]->erase(region);
						}
					}
			}
		}

		//Per DEBUG:
		cout << "Pre regions:" << endl;
		for (auto e: *pre_regions) {
			for (auto r: *e.second)
				Utilities::println(*r);
		}


		cout << "Post regions:" << endl;
		for (auto record: *post_regions) {
			for (auto region: *record.second) {
				Utilities::println(*region);
			}
		}

}


map<int, set<Region*> *> * Pre_and_post_regions_generator::get_post_regions(){
	return post_regions;
}

map<int, set<Region*> *> * Pre_and_post_regions_generator::get_pre_regions(){
	return pre_regions;
}