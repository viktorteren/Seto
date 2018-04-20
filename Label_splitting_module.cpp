//
// Created by ciuchino on 13/04/18.
//

#include "Label_splitting_module.h"

Label_splitting_module::Label_splitting_module(map<int, vector<Region*> *>* pre_regions,vector<ER>* er_set,vector<pair<int,Region*>>* number_of_bad_events){
    this->pre_regions=pre_regions;
    this->ER_set=er_set;
    this->number_of_bad_events=number_of_bad_events;
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

    pre_regions_intersection=regions_intersection(pre_regions);

    cout<<"trovata intersezione" <<endl;

    //per ogni evento
    //se per un evento non vale che l'intersezione è uguale all'er la TS non è exitation-closed

    for(auto item: *pre_regions) {
        cout<<"event: " <<item.first;
        auto event=item.first;
        auto er=ER_set->at((event));
        auto intersec=pre_regions_intersection->at(event);
        if( ! (is_equal_to(er, intersec) )){
             return false;
        }
    }

return true;

}

bool Label_splitting_module::is_bigger_than(Region* region ,set<int>* intersection){

   if(region->size() > intersection->size()){
       return false;
   }

   for(auto elem: *intersection){
       //nella regione non trovo un elem delll'intersez
       if( region->find(elem) != region->end()){
           return false;
       }
   }

    //nell'intersezione trovo tutti gli stati della regione
   return true;
}


void Label_splitting_module::do_label_splitting(map<int, vector<Region*> *>* middle_set_of_states){

    //per ogni evento
    //per ogni stato intermedio se è compreso nel set delle intersezioni
    //prendilo per label splitting

    cout<<"middle TOT: " <<endl;
    vector<Region*>::iterator it;
    //per ogni evento
    for(auto e: *middle_set_of_states) {

        //auto set_forced_to_be_a_region;
        auto event=e.first;
        for (it = middle_set_of_states->at(event)->begin(); it < middle_set_of_states->at(event)->end(); ++it) {
            cout << "middle" << endl;
            if (is_bigger_than(*it, pre_regions_intersection->at(event))) {
                cout << "erase" << endl;
                *middle_set_of_states->at(event)->erase(it, middle_set_of_states->at(event)->end());
            }
            else { //lo stato mi va bene
                //cache e ricalcolo solo se non c'è(avevo fatto break per no cross) mi prendo una coppia evento che viola e vettore di
                // transazioni e anche il numero di eventi che violano per ogni set di stati!!
            }
            for (auto el:**it)
                cout << "S: " << el << endl;
        }
        cout << "intesection" << endl;
        for (auto inte: *pre_regions_intersection->at(event)) {
            cout << "S:" << inte << endl;
        }


        // aggiungo set_forced_to_be_a_region alle regioni del mio evento
        //e splitto l'etichetta(modifico il grafo originale?)


    }




}