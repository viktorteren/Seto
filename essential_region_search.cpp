//
// Created by viktor on 14/04/18.
//

#include "essential_region_search.h"
#include <algorithm>

Essential_regions_search::Essential_regions_search(map<int, vector<Region*> *>* pre_reg){ pre_regions = pre_reg; };

Essential_regions_search::~Essential_regions_search() = default;

set<Region *> Essential_regions_search::search(){
	cout << "--------------------------------------------------- ESSENTIAL REGION SEARCH --------------------------------------------" << endl;
    //ALGORITMO:
    /*Per ogni evento
     *  unione delle pre regioni per creare un insieme di tutti gli stati
     *  per ogni stato
     *      controllo le regioni che non hanno tale stato
     *      se c'è una sola regione che non ha tale stato allora la regione è essenziale
     */

    set<int> temp_union;
    auto essential_regions = new set<Region *> ();
    Region * last_essential_candidate;
    int counter;

    //per ogni evento
    cout << "num eventi: " << pre_regions->size() << endl;
    for(auto record: *pre_regions){
        cout << endl <<  "evento: " << record.first << endl;

		Region::iterator it2;
        //regions = *record.second;
        /*for(it=regions.begin(); it!=regions.end();++it){
            Region* region= *it;
        }*/
        //unisco tutte le pre-regioni
        temp_union = regions_union(record.second);

        //per ogni stato dell'unione
        for(auto state: temp_union){
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
                cout << "trovato regione essenziale: "<< endl;
                //aggiungendo il ciclo for seguente il codice torna a non essere funzionante per colpa di auto che probabilmente utilizza un iteratore in comune con altri cicli
                /*for(auto state: *last_essential_candidate){
                	cout << state << " ";
                }*/
                it2 = last_essential_candidate->begin();
                int size = last_essential_candidate->size();
	            for(int i = 0; i < size; ++i) {
		            cout  << *it2 << " ";
		            ++it2;
	            }
	            cout << endl;
                essential_regions->insert(last_essential_candidate);
            }
        }
    }

    //ritornerò un vettore di puntatori a pre-regioni essenziali
    return *essential_regions;
}

//Region = set<int> ->ritorna un insieme di stati
set<int> Essential_regions_search::regions_union(vector<Region *>* vec){
    cout << "region union" << endl;
    Region* all_states = new Region();
	int size;
    Region::iterator it;
    for(Region* region: *vec){
        //cout << "region with size: " << region->size() << endl;
	    it=region->begin();
	    size = region->size();
	    for(int i = 0; i < size; ++i) {
	    	//cout << "Stato: " << *it << endl;
		    all_states->insert(*it);
		    ++it;
	    }
        //cout << "region size: " << region-> size() << endl;
    }
    //controllo per debug
    //cout << "unione: " << endl;
	for(auto state: *all_states){
    	cout << "st: " << state << endl;
    }
    return *all_states;
}




