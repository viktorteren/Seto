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
    int cont = 0;
    for(auto record: *pre_regions){
        cont ++;
        cout << "cont: " << cont << endl;
        if(cont == 3){
            cout << endl;
        }

        //regions = *record.second;
        /*for(it=regions.begin(); it!=regions.end();++it){
            Region* region= *it;
        }*/
        //unisco tutte le pre-regioni
        temp_union = regions_union(record.second, cont);
        if(cont == 3){
        	//per debug
            cout << endl;
        }
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
                cout << "trovato region essenziale"<< endl;
                essential_regions->insert(last_essential_candidate);
            }
        }
    }

    //ritornerò un vettore di puntatori a pre-regioni essenziali
    return *essential_regions;
}


//Region = set<int> ->ritorna un insieme di stati
set<int> Essential_regions_search::regions_union(vector<Region *>* vec, int cont){
    if(cont == 2){
    	//per debug
        cout << endl;
    }
    cout << "region union" << endl;
    set<int> all_states = {};
    int state;
    Region::iterator it;
    for(Region* region: *vec){
        //cout << "region with size: " << region->size() << endl;

	    for(it=region->begin(); it!=region->end();++it){
            state = *it;
        //for(auto state: *region){
		    /*cout << "region begin: " << &*(region->begin()) << endl;
		    cout << "region end: " << &*(region->end()) << endl;
		    cout << "region size: " << region-> size() << endl;
            cout << "it pointer: " << &(*it) << endl;*/
            cout << "state: " << state << endl;
	        cout << "it pointer: " << &(*it) << endl;
		    //todo: QUI ciclo infinito, it contiene sempre 5 o 6 a causa dell'aggiunta che non adovrebbe avere niente a che fare con le variabili del ciclo
		    all_states.insert(state);
		    cout << "it pointer: " << &(*it) << endl;
	        //cout << "it pointer: " << &(*it) << endl;
        }
    }
    return all_states;
}


