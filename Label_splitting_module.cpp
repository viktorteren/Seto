//
// Created by ciuchino on 13/04/18.
//

#include "Label_splitting_module.h"

Label_splitting_module::Label_splitting_module(map<int, vector<Region> *>* regions,vector<ER>* er_set){
    this->regions=regions;
    this->ER_set=er_set;
    //this->number_of_bad_events=number_of_bad_events;
};

Label_splitting_module::~Label_splitting_module(){
};

void printRegion(const Region& region){
    for(auto state: region){
        cout << "state " << state <<endl;
    }
    cout << endl;
}


bool Label_splitting_module::is_equal_to(ER er,set<int>* intersection){
    cout<<"ER";
    printRegion(*er);
    cout << "INTER";
    printRegion(*intersection);


    if(er->size()!=intersection->size()) {
        cout<<"if"<<endl;
        return false;
    }
    else {
         cout<<"else"<<endl;
        for (auto state_er : *er) {
             if(intersection->find(state_er) == intersection->end()) {   //non l'ho trovato
                return false;
             }
        }
    }

    return true;
}


bool Label_splitting_module::is_excitation_closed() {

    regions_intersection=do_regions_intersection(regions);

    cout<<"trovata intersezione" <<endl;

    //per ogni evento
    //se per un evento non vale che l'intersezione è uguale all'er la TS non è exitation-closed

    for(auto item: *regions) {
        cout<<"event: " <<item.first;
        auto event=item.first;
        auto er=ER_set->at((event));
        auto intersec=regions_intersection->at(event);
        if( ! (is_equal_to(er, intersec) )){
             return false;
        }
    }

return true;

}

bool Label_splitting_module::is_bigger_than(Region* region ,set<int>* intersection){

   if(region->size() >= intersection->size()){
       cout<<"TRUE**************"<<endl;
       return true;
   }

   for(auto elem: *intersection){
       //nella regione non trovo un elem delll'intersez
       if( region->find(elem) != region->end()){
           cout<<"FALSE**************"<<endl;
           return false;
       }
   }

    //nella regione trovo tutti gli stati dell'intersezione
    cout<<"TRUE**************"<<endl;
   return true;
}


vector<Region>* Label_splitting_module::do_label_splitting(map<int, vector<Region*> *>* middle_set_of_states,map<int,vector< pair<int,Region*> >*>* number_of_bad_events){

    //per ogni evento
    //per ogni stato intermedio se è compreso nel set delle intersezioni
    //prendilo per label splitting

    cout<<"ordine:ptr regions:"<<endl;
    auto vec_pt= number_of_bad_events->at(2);

    for(auto reg: *vec_pt)
        cout<<"C: "<< reg.second;


    cout<<"ptr middle set"<<endl;
    auto middle_vec_ptr=middle_set_of_states->at(2);
    for(auto reg: * middle_vec_ptr)
        cout<<" M: "<<reg;


    cout<<"middle TOT: " <<endl;
    vector<Region*>::iterator it;
    //per ogni evento
    for(auto e: *middle_set_of_states) {
        cout<<"EVENTO: "<<e.first<<endl;
        int pos=0;
        //auto set_forced_to_be_a_region;
        auto event=e.first;
        for (it = middle_set_of_states->at(event)->begin(); it < middle_set_of_states->at(event)->end(); ++it) {
            cout << "middle" << endl;
            if (is_bigger_than(*it, regions_intersection->at(event))) {
                cout << "erase" << endl;
                *middle_set_of_states->at(event)->erase(it, middle_set_of_states->at(event)->end());
            }
            else { //lo stato mi va bene
                //cache e ricalcolo solo se non c'è(avevo fatto break per no cross) mi prendo una coppia evento che viola e vettore di
                // transazioni e anche il numero di eventi che violano per ogni set di stati!!
                cout<<"eveent: " <<event << endl;
                auto vec_ptr= number_of_bad_events->at(event);
                auto pair= (*vec_ptr)[pos];
                if( pair.first ==-1 ) {
                    //ricalcola
                }

            }
            for (auto el:**it)
                cout << "S: " << el << endl;

            pos++;
        }

        cout << "intesection" << endl;
        for (auto inte: *regions_intersection->at(event)) {
            cout << "S:" << inte << endl;
        }


        // aggiungo set_forced_to_be_a_region alle regioni del mio evento
        //e splitto l'etichetta(modifico il grafo originale?)


    }




}