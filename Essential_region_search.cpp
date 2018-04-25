//
// Created by viktor on 14/04/18.
//

#include "Essential_region_search.h"

Essential_regions_search::Essential_regions_search(map<int, set<Region*> *>* pre_reg){ pre_regions = pre_reg; };

Essential_regions_search::~Essential_regions_search() = default;

set<Region *> Essential_regions_search::search(){
    //todo: fare debuf -> con input 4 ci sono problemi con la rilevazione di regioni essenziali: potrebbe essere collegato con i duplicati delle regioni (sicuramente)
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
        cout <<  "evento: " << record.first << endl;

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
                cout << "trovato regione essenziale: ";
                Utilities::println(*last_essential_candidate);
                essential_regions->insert(last_essential_candidate);
            }
        }
    }

    //ritornerò un vettore di puntatori a pre-regioni essenziali
    return *essential_regions;
}




