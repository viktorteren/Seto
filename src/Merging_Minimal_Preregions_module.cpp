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
                cout<<"entrambe"<<endl;
            }
                //l'evento è solo in first(essential)
            else if(first->find(event)!=first->end()){
                (*total_pre_regions_map)[event] = new set<Region*>(first->at(event)->begin(),first->at(event)->end());
                cout<<"first"<<endl;
            }
                //l'evento è solo in second(irredundant)
            else if(second->find(event)!=second->end()){
                (*total_pre_regions_map)[event] = new set<Region *>(second->at(event)->begin(),second->at(event)->end());
                cout<<"secodn"<<endl;
            }
        }
    }
    else {
        for(auto record: *first){
            auto event=record.first;
            (*total_pre_regions_map)[event]=new set<Region*>(first->at(event)->begin(),first->at(event)->end());
        }
    }

    for(auto el:*total_pre_regions_map){
        for(auto reg:*el.second){
            Utilities::println(*reg);
        }
    }
}

map<int,set<Region*>*>*  Merging_Minimal_Preregions_module::get_total_preregions_map() {
    return total_pre_regions_map;
}

void Merging_Minimal_Preregions_module::merging_preregions() {

    cout<<"MERGING PREREGIONS___________"<<endl;

    set<Region*>* preregions_set=Utilities::copy_map_to_set(total_pre_regions_map);
    Region* reg_union= nullptr;

    //per ogni coppia di preregioni
    //per ogni evento
    //controlla che valga EC con la nuova regione
    //se vale

    for(auto reg1:*preregions_set) {
        for (auto reg2:*preregions_set) {
            cout<<"r1: " <<reg1<<endl;
            cout<<"r2: " <<reg2<<endl;
            if(!Utilities::are_equals(reg1,reg2)) {
                reg_union = Utilities::regions_union(reg1, reg2);
                cout << "end" << endl;
                for (auto record:*total_pre_regions_map) {
                    auto event = record.first;
                    auto tmp_set = new set<Region *>(total_pre_regions_map->at(event)->begin(),
                                                     total_pre_regions_map->at(event)->end());
                    tmp_set->erase(tmp_set->find(reg1));
                    tmp_set->erase(tmp_set->find(reg2));
                    tmp_set->insert(reg_union);

                    //todo controlla ec

                }
            }
        }
    }


}

