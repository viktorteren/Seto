//
// Created by ciuchino on 09/05/18.
//

#include "../include/Merging_Minimal_Preregions_module.h"

Merging_Minimal_Preregions_module::Merging_Minimal_Preregions_module(map<int,set<Region*>*> * essential_preregions,map<int,set<Region*>*> * irredundant_preregions,map<int,ER>* ER){
    er=ER;

    merging_2_maps(essential_preregions,irredundant_preregions);
    merged_pre_regions_map=merging_preregions(ER);
}

Merging_Minimal_Preregions_module::~Merging_Minimal_Preregions_module() {
    for(auto el:*total_pre_regions_map)
        delete el.second;
    delete total_pre_regions_map;

    if(merged_pre_regions_map!= nullptr) {
        for (auto el: *merged_pre_regions_map)
            delete el.second;
        delete merged_pre_regions_map;
        delete union_ptr;
    }

    for(auto el:*er)
        delete el.second;
    delete er;


}


void Merging_Minimal_Preregions_module::merging_2_maps(map<int,set<Region*>*>* first,map<int,set<Region*>*>* second){

    cout<<"MERGIN ESSENTIAL AND IRREDUNDANT REGIONS**********"<<endl;

    total_pre_regions_map=new map<int,set<Region*>*>();

    cout<<"first"<<endl;
    for(auto el:*first) {
        cout << "ev " << el.first << endl;
        for (auto reg:*el.second)
            Utilities::println(*reg);
    }

    /*cout<<"  SECOND"<<endl;
    for(auto el:*second) {
        cout << "ev " << el.first << endl;
        for (auto reg:*el.second)
            Utilities::println(*reg);
    }*/


    if(second!=nullptr){
        //todo num_events deve diventare quello dop olabel splitting
        for(int event=0;event<num_events;event++){
            cout<<"evento:"<<event<<endl;

            //trovo entrambi gli eventi
            if(first->find(event)!=first->end() && second->find(event)!=second->end()) {

                auto merged_vector=new vector<Region*>(first->at(event)->size()+second->at(event)->size());

                set_union(first->at(event)->begin(), first->at(event)->end(), second->at(event)->begin(), second->at(event)->end(), merged_vector->begin());

                for(auto el: *merged_vector)
                    Utilities::println(*el);

                (*total_pre_regions_map)[event] = new set<Region *>(merged_vector->begin(), merged_vector->end());
                delete merged_vector;
                /*cout<<"entrambe"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                    Utilities::println(*el);*/
            }
                //l'evento è solo in first(essential)
            else if(first->find(event)!=first->end()){
                auto merged_vector=new vector<Region*>(first->at(event)->size());

                (*total_pre_regions_map)[event] = new set<Region*>(first->at(event)->begin(),first->at(event)->end());
                delete merged_vector;
                /*cout<<"first"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                        Utilities::println(*el);*/
            }
                //l'evento è solo in second(irredundant)
            else if(second->find(event)!=second->end()){
                auto merged_vector=new vector<Region*>(second->at(event)->size());

                (*total_pre_regions_map)[event] = new set<Region *>(second->at(event)->begin(),second->at(event)->end());
                delete merged_vector;
               /* cout<<"secodn"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                    Utilities::println(*el);*/
            }

        }
    }
    else {
        for(auto record: *first){
            auto event=record.first;
            (*total_pre_regions_map)[event]=new set<Region*>(first->at(event)->begin(),first->at(event)->end());
        }
    }


    cout<<"debug merging 2 mappe"<<endl;
    for(auto el: *total_pre_regions_map){
        cout<<"ev: " <<el.first<<endl;
        for(auto r:*el.second)
        Utilities::println(*r);
    }

}

map<int,set<Region*>*>*  Merging_Minimal_Preregions_module::get_merged_preregions_map() {
    return merged_pre_regions_map;
}

map<int,set<Region*>*>*  Merging_Minimal_Preregions_module::get_total_preregions_map() {
	return total_pre_regions_map;
}

map<int,set<Region*>*>*  Merging_Minimal_Preregions_module::merging_preregions(map<int,set<int>*>* ER_map) {

    cout<<"MERGING PREREGIONS___________"<<endl;

    set<Region*>* preregions_set=Utilities::copy_map_to_set(total_pre_regions_map);
    Region* reg_union= nullptr;


    for(auto el:*preregions_set){
        Utilities::println(*el);
    }

    //per ogni coppia di preregioni
    //per ogni evento
    //controlla che valga EC con la nuova regione
    //se vale

    map<int,set<Region*>* >* tmp_map=new map<int,set<Region*>*>();
    for(auto record: *total_pre_regions_map){
        auto set=record.second;
        (*tmp_map)[record.first]=new std::set<Region*>(set->begin(),set->end());
    }

	set<Region*>::iterator it;
	set<Region*>::iterator it2;

	for(it=preregions_set->begin();it!=preregions_set->end();++it){
		for(it2=next(it);it2!=preregions_set->end();++it2){
			auto reg1=*it;
			auto reg2=*it2;

            cout<<"r1: ";
			Utilities::println(*reg1);
            cout<<"r2: ";
			Utilities::println(*reg2);
            if(!Utilities::are_equals(reg1,reg2)) {

                reg_union = Utilities::regions_union(reg1, reg2);
                bool ec=false;
                for (auto record:*total_pre_regions_map) {
                    auto event = record.first;
					cout<<"evento: " <<event;
                    //auto tmp_set = new set<Region *>(total_pre_regions_map->at(event)->begin(),
                                                   //  total_pre_regions_map->at(event)->end());

                    auto tmp_set=new set<Region*>();

                    bool event_contains_reg=false;
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
                        auto er = ER_map->at(event);

                        cout<<"intersection:"<<endl;
                        Utilities::println(*intersection);

                        cout<<"er:"<<endl;
                        Utilities::println(*er);

                        //controlla ec ER(ev)==intersec(prereg(ev))
                        ec = Utilities::are_equals(intersection, er);

                        delete intersection;

                        //provo altre 2 regioni perchè per un evento non vale la EC
                        if (ec == false) {
                            cout<<"BREAK l'evento non soddisfa EC-provo altre regioni"<<endl;
                            delete tmp_set;
                            delete reg_union;
                            break;
                        } else {
                            delete (*tmp_map)[event];
                            (*tmp_map)[event] = tmp_set;
                            union_ptr=reg_union;
                        }
                    }
					else {
                        delete tmp_set;
						cout<<"event:"<<event<<endl;
						cout<<"l'evento è ok perchè non ha cambiato le sue preregioni con questa unione"<<endl;
					}
                }

                //se per tutti gli eventi la coppia è ok faccio il merge effettivo
                if(ec==true){
                    delete preregions_set;
                    cout<<"merging ok"<<endl;
                    Utilities::println(*reg1);
                    Utilities::println(*reg2);


                    for(auto el:*tmp_map) {
                        cout << "ev: " << el.first<<endl;
                        for (auto r: *el.second) {
                            Utilities::println(*r);
                        }
                    }

                    return tmp_map;
                }

            }

        }
    }

    //non ho fatto il merge
    cout<<"not merging"<<endl;


    for(auto el: *tmp_map){
        delete el.second;
    }
    delete tmp_map;
    delete preregions_set;

    return nullptr;


}

