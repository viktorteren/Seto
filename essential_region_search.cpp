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

Region getUnion(Region& a, Region& b) {
    Region result = a;
    //todo: questa riga fa impallare l'esecuzione senza dare core dump
    result.insert(b.begin(), b.end());

	return result;
}

//Region = set<int> ->ritorna un insieme di stati
set<int> Essential_regions_search::regions_union(vector<Region *>* vec, int cont){
    if(cont == 2){
    	//per debug
        cout << endl;
    }
    cout << "region union" << endl;
    Region* all_states = new Region();
    //Region* temp_ptr = new Region();
    //int state;
    Region::iterator it;
    for(Region* region: *vec){
        //cout << "region with size: " << region->size() << endl;
	    /*it=region->begin();
	    while(it!=region->end()){
            //state = *it;
        //for(auto state: *region){
		    cout << "region begin: " << &*(region->begin()) << endl;
		    cout << "region end: " << &*(region->end()) << endl;
		    cout << "region size: " << region-> size() << endl;
            cout << "it pointer: " << &(*it) << endl;
            cout << "it pointer: " << &(*it) << endl;
		    all_states.insert(*it);
		    //cout << "it pointer: " << &(*it) << endl;
	        //cout << "it pointer: " << &(*it) << endl;
		    it = next(it, 1);
        }*/
	    *all_states = getUnion(*all_states, *region);
	    //*all_states = *temp_ptr;
        cout << "region size: " << region-> size() << endl;

    }
    return *all_states;
}




