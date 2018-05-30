//
// Created by viktor on 14/04/18.
//

#include "../include/Essential_region_search.h"

Essential_regions_search::Essential_regions_search(map<int, set<Region*> *>* pre_reg){ pre_regions = pre_reg; };

Essential_regions_search::~Essential_regions_search(){
	for(auto record: *pre_regions){
		delete (*essential_map)[record.first];
	}
	delete essential_map;
}

set<Region *>* Essential_regions_search::search(){

	cout << "--------------------------------------------------- ESSENTIAL REGION SEARCH --------------------------------------------" << endl;
    //ALGORITMO:
    /*Per ogni evento
     *  unione delle pre regioni per creare un insieme di tutti gli stati
     *  per ogni stato
     *      controllo le regioni che non hanno tale stato
     *      se c'è una sola regione che non ha tale stato allora la regione è essenziale
     */


    set<int> *temp_union;

	auto essential_regions = new set<Region *> ();
	essential_map = new map<int, set<Region *>*>();

	Region * last_essential_candidate;
    int counter;

    //per ogni evento
    //cout << "num eventi: " << pre_regions->size() << endl;
    for(auto record: *pre_regions){
        //cout <<  "evento: " << record.first << endl;

		if(record.first==4){
			cout<<"debug pre regions"<<endl;
			print(*pre_regions);
		}

		Region::iterator it2;

        //se ho una sola regione, tale regione è per forza essenziale
		if(record.second->size() == 1) {
			for (auto reg: *record.second) {
				//cout << "trovato regione essenziale: ";
				//println(*reg);
				essential_regions->insert(reg);
				//non ho ancora creato nessun record relativo ad un certo evento
				if(essential_map->find(record.first) == essential_map->end()){
					(*essential_map)[record.first] = new set<Region *>();
				}
				(*essential_map)[record.first]->insert(reg);
			}
		}
		else{
			//unisco tutte le pre-regioni
			temp_union = regions_union(record.second);
			//cout << "union: ";
			//println(temp_union);

			//per ogni stato dell'unione
			for(auto state: *temp_union){
				counter = 0;
				//per ogni regione
				for(auto region: *record.second){
					//controllo se la regione non contiene lo stato
					if (region->find(state) == region->end()) {
						if(counter == 0) {
							last_essential_candidate = region;
							counter ++;
						}
						else{
							counter = -1;
							break;
						}
					}
				}
				//se ho avuto un solo stato candidato per essere essenziale allora è davvero essenziale
				if(counter == 1){
					//cout << "trovato regione essenziale: ";
					//println(*last_essential_candidate);
					essential_regions->insert(last_essential_candidate);
					if(essential_map->find(record.first) == essential_map->end()){
						(*essential_map)[record.first] = new set<Region *>();
					}
					(*essential_map)[record.first]->insert(last_essential_candidate);
				}
			}
			delete temp_union;
		}

    }

    //print per debug

    cout << "prove essential regions:" << endl;
    for(auto rec: *essential_map){
    	cout << "set di regioni dell'evento: " << rec.first << endl;
    	for(auto reg: *rec.second){
    		println(*reg);
    	}
    }

    cout << "Regioni essenziali: " << endl;
    for (auto reg: *essential_regions) {
    	println(*reg);
    }

    //ritornerò un vettore di puntatori a pre-regioni essenziali
    return essential_regions;
}


map<int, set<Region*>*> * Essential_regions_search::get_essential_regions_map(){
	if(essential_map == nullptr){
		search();
	}
	return essential_map;
};



