//
// Created by ciuchino on 09/05/18.
//

#include "../include/Merging_Minimal_Preregions_module.h"

Merging_Minimal_Preregions_module::Merging_Minimal_Preregions_module(map<int,set<Region*>*> * essential_preregions,map<int,set<Region*>*> * irredundant_preregions){
    merging_2_maps(essential_preregions,irredundant_preregions);
    merging_preregions();
}

Merging_Minimal_Preregions_module::~Merging_Minimal_Preregions_module() {
    /*for(auto el:*total_pre_regions_map)
        delete el;*/
    //todo delete la nuova regione creata
    delete total_pre_regions_map;
}


void Merging_Minimal_Preregions_module::merging_2_maps(map<int,set<Region*>*>* first,map<int,set<Region*>*>* second){

    total_pre_regions_map=new map<int,set<Region*>*>();


    if(second!=nullptr){
        for(int event=0;event<num_events;event++){
            cout<<"evento:"<<event<<endl;
            auto merged_vector=new vector<Region*>(num_events);

            //trovo entrambi gli eventi
            if(first->find(event)!=first->end() && second->find(event)!=second->end()) {
                set_union(first->at(event)->begin(), first->at(event)->end(), second->at(event)->begin(), second->at(event)->end(), merged_vector->begin());
                (*total_pre_regions_map)[event] = new set<Region *>(merged_vector->begin(), merged_vector->end());
                //cout<<"entrambe"<<endl;
            }
                //l'evento è solo in first(essential)
            else if(first->find(event)!=first->end()){
                (*total_pre_regions_map)[event] = new set<Region*>(first->at(event)->begin(),first->at(event)->end());
                //cout<<"first"<<endl;
            }
                //l'evento è solo in second(irredundant)
            else if(second->find(event)!=second->end()){
                (*total_pre_regions_map)[event] = new set<Region *>(second->at(event)->begin(),second->at(event)->end());
                //cout<<"secodn"<<endl;
            }
        }
    }
    else {
        for(auto record: *first){
            auto event=record.first;
            (*total_pre_regions_map)[event]=new set<Region*>(first->at(event)->begin(),first->at(event)->end());
        }
    }
}

map<int,set<Region*>*>*  Merging_Minimal_Preregions_module::get_total_preregions_map() {
    return total_pre_regions_map;
}

void Merging_Minimal_Preregions_module::merging_preregions() {

}

