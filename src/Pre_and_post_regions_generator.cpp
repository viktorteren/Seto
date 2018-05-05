//
// Created by Viktor on 20/04/2018.
//

#include "../include/Pre_and_post_regions_generator.h"

using namespace Utilities;

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
	create_pre_and_post_regions(candidate_regions);
}

Pre_and_post_regions_generator::~Pre_and_post_regions_generator(){
	/*for(auto record: *pre_regions){
		delete record.second;
	}*/
	delete pre_regions;
	delete post_regions;
}

bool Pre_and_post_regions_generator::is_pre_region(List_edges *list, Region *region) {
    cout<<"regione (is_pre_region)"<<endl;
    println(*region);
	for(auto t: *list){
		if( region->find(t.first) != region->end()){ //il primo stato appartiene alla regione
			if(region->find(t.second) == region->end())
                return true;
		}
	}
	return false;
}

bool Pre_and_post_regions_generator::is_post_region(List_edges *list, Region *region) {
    cout<<"regione (is_post_region)"<<endl;
    println(*region);
	for(auto t: *list){
		if( region->find(t.first) == region->end()){ //il primo stato non appartiene alla regione
			if(region->find(t.second) != region->end())
				return true;
		}
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
				print(region);
				cout << " a causa di: ";
				println(new_region);
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

            //se l'evento non era presente nella mappa creo lo spazioo per il relativo set di regioni
            if (pre_regions->find(record.first) == pre_regions->end()) {
                (*pre_regions)[record.first] = new set<Region *>();
            }

            //se l'evento non era presente nella mappa creo lo spazioo per il relativo set di regioni
            if (post_regions->find(record.first) == post_regions->end()) {
                (*post_regions)[record.first] = new set<Region *>();
            }

            if (candidate_regions != nullptr) {
                bool pre_region = is_pre_region(&record.second, region);
                cout<<"dopo preregioni"<<endl;
                bool post_region = is_post_region(&record.second, region);
                cout<<"dopo postregioni"<<endl;
                if (pre_region || post_region) {
                    println(*it);
                    cout << "is pre region of" << record.first << endl;

                    bool split = false;
					bool to_delete=false;

                    for (auto cand_reg: *candidate_regions) {
                        if (is_bigger_than(region, &cand_reg)) {

                            Region *candidate_region = nullptr;
                            Region *new_region = region_difference(*region, cand_reg);

                            if (!contains((*pre_regions)[record.first], new_region)) {
                                if (is_pre_region(&record.second, new_region)) {
                                    (*pre_regions)[record.first]->insert(new_region);
                                    cout << "ho inserito new region(difference)" << endl;
                                    println(*new_region);
                                }
								else
									to_delete=true;

                            }
							else
								to_delete=true;

                            if (!contains((*post_regions)[record.first], new_region)) {
                                if (is_post_region(&record.second, new_region)) {
                                    (*post_regions)[record.first]->insert(new_region);

                                    cout << "ho inserito new region(difference)" << endl;
                                    println(*new_region);
                                }
								else
									if(to_delete){
                                	    delete new_region;
                                        to_delete=false;
									}
                            }
							else
								if(to_delete)
									delete new_region;

                            if (!contains((*pre_regions)[record.first], &cand_reg)) {
                                if (is_pre_region(&record.second, &cand_reg)) {
                                    candidate_region = new set<int>(cand_reg);
                                    (*pre_regions)[record.first]->insert(candidate_region);
                                    //cout << "ho inserito new region(cand reg)" << endl;
                                    //println(cand_reg);
                                    pre_region = true;
                                }
                            }


                            if (!contains((*post_regions)[record.first], &cand_reg)) {
                                if (is_post_region(&record.second, &cand_reg)) {
                                	if(candidate_region == nullptr)
                                		candidate_region = new set<int>(cand_reg);
                                    (*post_regions)[record.first]->insert(candidate_region);
                                    //cout << "ho inserito new region(cand reg)" << endl;
                                    //println(cand_reg);
                                    post_region = true;
                                }
                            }
                            split = true;
                        }
                    }

                    if (!split) {
                        if (pre_region) (*pre_regions)[record.first]->insert(region);
                        if (post_region) (*post_regions)[record.first]->insert(region);
                    }

                }
            }
            else if (is_pre_region(&record.second, region)) {
                (*pre_regions)[record.first]->insert(region);
            }
            else if (is_post_region(&record.second, region)) {
                (*post_regions)[record.first]->insert(region);
            }
		}
	}

	//Per DEBUG:
	cout << "Pre regions:" << endl;
	for (auto e: *pre_regions) {
		cout<<"event "<< e.first<<endl;
		for (auto r: *e.second)
			println(*r);
	}


	cout << "Post regions:" << endl;
	for (auto record: *post_regions) {
		cout<<"event "<< record.first<<endl;
		for (auto region: *record.second) {
			println(*region);
		}
	}
}


map<int, set<Region*> *> * Pre_and_post_regions_generator::get_post_regions(){
	return post_regions;
}

map<int, set<Region*> *> * Pre_and_post_regions_generator::get_pre_regions(){
	return pre_regions;
}