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

    cout<<"first"<<endl;
    for(auto el:*first) {
        cout << "ev " << el.first << endl;
        for (auto reg:*el.second)
            Utilities::println(*reg);
    }

    cout<<"  SECOND"<<endl;
    for(auto el:*second) {
        cout << "ev " << el.first << endl;
        for (auto reg:*el.second)
            Utilities::println(*reg);
    }


    if(second!=nullptr){
        for(int event=0;event<num_events;event++){
            cout<<"evento:"<<event<<endl;

            //trovo entrambi gli eventi
            if(first->find(event)!=first->end() && second->find(event)!=second->end()) {

                auto merged_vector=new vector<Region*>(first->at(event)->size()+second->at(event)->size());

                set_union(first->at(event)->begin(), first->at(event)->end(), second->at(event)->begin(), second->at(event)->end(), merged_vector->begin());

                for(auto el: *merged_vector)
                    Utilities::println(*el);

                (*total_pre_regions_map)[event] = new set<Region *>(merged_vector->begin(), merged_vector->end());
                cout<<"entrambe"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                    Utilities::println(*el);
            }
                //l'evento è solo in first(essential)
            else if(first->find(event)!=first->end()){
                auto merged_vector=new vector<Region*>(first->at(event)->size());

                (*total_pre_regions_map)[event] = new set<Region*>(first->at(event)->begin(),first->at(event)->end());
                cout<<"first"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                        Utilities::println(*el);
            }
                //l'evento è solo in second(irredundant)
            else if(second->find(event)!=second->end()){
                auto merged_vector=new vector<Region*>(second->at(event)->size());

                (*total_pre_regions_map)[event] = new set<Region *>(second->at(event)->begin(),second->at(event)->end());
                cout<<"secodn"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                    Utilities::println(*el);
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

ER Merging_Minimal_Preregions_module::create_ER_after_splitting(int event){
    //todo
}

map<int,set<Region*>*>*  Merging_Minimal_Preregions_module::merging_preregions() {

    cout<<"MERGING PREREGIONS___________"<<endl;

    set<Region*>* preregions_set=Utilities::copy_map_to_set(total_pre_regions_map);
    Region* reg_union= nullptr;

    //per ogni coppia di preregioni
    //per ogni evento
    //controlla che valga EC con la nuova regione
    //se vale

    map<int,set<Region*>* >* tmp_map=new map<int,set<Region*>*>();

    for(auto reg1:*preregions_set) {
        for (auto reg2:*preregions_set) {
            cout<<"r1: " <<reg1<<endl;
            cout<<"r2: " <<reg2<<endl;
            if(!Utilities::are_equals(reg1,reg2)) {
                reg_union = Utilities::regions_union(reg1, reg2);
                cout << "end" << endl;
                bool ec=false;
                for (auto record:*total_pre_regions_map) {
                    auto event = record.first;
                    //auto tmp_set = new set<Region *>(total_pre_regions_map->at(event)->begin(),
                                                   //  total_pre_regions_map->at(event)->end());

                    auto tmp_set=new set<Region*>();

                    bool event_contains_reg=true;
                    for(auto reg: *total_pre_regions_map->at(event)){
                        if(!Utilities::are_equals(reg,reg1) && !Utilities::are_equals(reg,reg2) )
                            tmp_set->insert(reg);
                        else event_contains_reg=true;
                    }
                    //inserisco l'unione se l'evento conteneva almeno una delle 2 regioni da unire
                    if(event_contains_reg) {
                        tmp_set->insert(reg_union);

                        //tmp_set->erase(tmp_set->find(reg1));
                        //tmp_set->erase(tmp_set->find(reg2));
                        //tmp_set->insert(reg_union);

                        auto intersection = Utilities::regions_intersection(tmp_set);
                        auto er = create_ER_after_splitting(event);

                        //controlla ec ER(ev)==intersec(prereg(ev))
                        ec = Utilities::are_equals(intersection, er);

                        delete intersection;
                        delete er;

                        //provo altre 2 regioni perchè per un evento non vale la EC
                        if (ec == false) {
                            delete tmp_set;
                            break;
                        } else {
                            (*tmp_map)[event] = tmp_set;
                        }
                    }
                }

                //se per tutti gli eventi la coppia è ok faccio il merge effettivo
                if(ec==true){
                    delete preregions_set;
                    return tmp_map;
                }

            }
        }
    }

    //non ho fatto il merge
    return nullptr;


}

