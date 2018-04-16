//
// Created by ciuchino on 13/04/18.
//

#include "Label_splitting_module.h"
#include "TS_parser.h"

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


bool Label_splitting_module::is_exitation_closed() {

    map<int, set<int>* > *pre_regions_intersection=regions_intersection();

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


map<int, set<int>* > * Label_splitting_module::regions_intersection(){

    map<int, set<int>* > *pre_regions_intersection=new map<int,set<int>*>;

    std::vector<Region*>::iterator it;

    //per ogni evento
    for(auto item: *pre_regions) {

        for(auto i: *item.second){
            cout<<"Preregion di " << item.first<<endl;
            for(auto pre: *i)
                cout<< "S: " << pre <<endl;
        }
        //se c'è più di una preregione
        if(item.second->size()>1) {
            //per ogni preregione
            for (it = item.second->begin(); it != item.second->end(); ++it) {
                //for (auto set_ptr: *item.second) {
                auto set_ptr = *it; //puntatore al set
                if (it == item.second->begin()) {
                    auto set_ptr2 = *(++it);
                    //cout << "setprt" << set_ptr << endl;
                    for (auto state: *set_ptr) {
                        if (set_ptr2->find(state) != set_ptr2->end()) {//trovo lo stato (appartiene a entrambe)
                            if (pre_regions_intersection->find(item.first) == pre_regions_intersection->end()) {
                                (*pre_regions_intersection)[item.first] = new set<int>();
                            }
                            pre_regions_intersection->at(item.first)->insert(state);
                        }
                    }
                } else {
                    cout << "else" << endl;
                    for (auto state: *set_ptr) {
                        if (pre_regions_intersection->find(state) !=
                            pre_regions_intersection->end()) {//trovo lo stato (appartiene a entrambe)
                            //aggiungo la regione alla mappa
                            if (pre_regions_intersection->find(item.first) == pre_regions_intersection->end()) {
                                (*pre_regions_intersection)[item.first] = new set<int>();
                            }
                            pre_regions_intersection->at(item.first)->insert(state);
                        }
                    }
                }

                for (auto el: *pre_regions_intersection->at(item.first)) {
                    cout << "INTERSEZIONE: " << el << endl;
                }
            }
        }
            //c'è solo una preregione tutti gli stati appartengono all'intersezione
        else {
            for(auto state: *((*item.second)[0])){
                if (pre_regions_intersection->find(item.first) == pre_regions_intersection->end()) {
                    (*pre_regions_intersection)[item.first] = new set<int>();
                }
                pre_regions_intersection->at(item.first)->insert(state);
            }

            for (auto el: *pre_regions_intersection->at(item.first)) {
                cout << "INTERSEZIONE: " << el << endl;
            }
        }
    }

    return pre_regions_intersection;


}