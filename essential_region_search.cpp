//
// Created by viktor on 14/04/18.
//

#include "essential_region_search.h"

//using namespace essential_regions;

Essential_regions_search::Essential_regions_search(map<int, vector<Region*> *>* pre_reg){
    pre_regions=pre_reg;
};

Essential_regions_search::~Essential_regions_search(){
};

set<Region *> Essential_regions_search::search(){
    //ALGORITMO:
    /*Per ogni evento
     *  unione delle pre regioni per creare un insieme di tutti gli stati
     *  per ogni stato
     *      controllo le regioni che non hanno tale stato
     *      se c'è una sola regione che non ha tale stato allora la regione è essenziale
     */

    vector<Region*> regions;
    set<int> temp_union;
    set<Region *> * essential_regions = new set<Region *> ();
    Region * last_essential_candidate;
    int counter;

    //per ogni stato
    for(auto record: *pre_regions){

        regions = *record.second;
        /*for(it=regions.begin(); it!=regions.end();++it){
            Region* region= *it;
        }*/
        //unisco tutte le pre-regioni
        temp_union = regions_union(regions);
        //per ogni stato dell'unione
        for(auto state: temp_union){
            counter = 0;
            //per ogni regione
            for(auto region: regions){
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
                essential_regions->insert(last_essential_candidate);
            }
        }
    }

    //ritornerò un vettore di puntatori a pre-regioni essenziali
    return *essential_regions;
}

//Region = set<int> ->ritorna un insieme di stati
set<int> Essential_regions_search::regions_union(vector<Region *> vec){
    //todo: verificare se il contenuto per siste dopo il return
    set<int>* all_states = new set<int>;
    for(auto region: vec){
        for(auto state: *region){
            all_states->insert(state);
        }
    }
    return *all_states;
}
