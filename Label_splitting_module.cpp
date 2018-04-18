//
// Created by ciuchino on 13/04/18.
//

#include "Label_splitting_module.h"

Label_splitting_module::Label_splitting_module(map<int, vector<Region*> *>* pre_regions,vector<ER>* er_set){
    this->pre_regions=pre_regions;
    this->ER_set=er_set;
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

    map<int, set<int>* > *pre_regions_intersection=regions_intersection(pre_regions);

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


void Label_splitting_module::do_label_splitting(map<int, vector<Region*> *>* middle_set_of_states){

    //per ogni stato intermedio se è compreso nel set delle intersezioni
    //prendilo per label splitting
    cout<<"middle: " <<endl;
    for(auto middle: *(*middle_set_of_states).at(0))
        cout<<"S: " << middle<<endl;
    cout<<"intesection"<<endl;
    for(auto inte: *(regions_intersection(pre_regions)->at(0)) ){
        cout<<"S:" << inte<<endl;
    }

}