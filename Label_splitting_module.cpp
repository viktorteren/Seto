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
    delete ER_set;
    delete regions_intersection;
};

void printRegion(const Region& region){
    for(auto state: region){
        cout << "state " << state <<endl;
    }
    cout << endl;
}


bool Label_splitting_module::is_equal_to(ER er,const set<int>* intersection){

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
bool Label_splitting_module::is_bigger_than(Region* region ,set<int>* intersection){

    if(region->size() >= intersection->size()){
        cout<<"TRUE**************"<<endl;
        return true;
    }

    for(auto elem: *intersection){
        //nella regione non trovo un elem delll'intersez
        if( region->find(elem) == region->end()){
            cout<<"FALSE**************"<<endl;
            return false;
        }
    }

    //nella regione trovo tutti gli stati dell'intersezione
    cout<<"TRUE**************"<<endl;
    return true;
}

vector<int>* Label_splitting_module::is_excitation_closed() {

    regions_intersection=do_regions_intersection(regions);

    cout<<"trovata intersezione____________" <<endl;
    for(auto ev:*regions_intersection) {
        cout << "evento " << ev.first<<endl;
        println(*ev.second);
    }

    vector<int>* events_not_satisfy_EC=new vector<int>;

    //per ogni evento
    //se per un evento non vale che l'intersezione è uguale all'er la TS non è exitation-closed
    //bool res=true;

    for(auto item: *regions) {
        cout<<"event: " <<item.first;
        auto event=item.first;
        auto er=ER_set->at((event));
        auto intersec=regions_intersection->at(event);
        if( ! (is_equal_to(er, intersec) )){
             cout<<"regione delle'evento:"<<event;
             events_not_satisfy_EC->push_back(event);
             //res=false;
        }
    }


    //delete regions_intersection;

    // ritorna chi non soddisfa così faccio lo splitting solo per quegli eventi
    //la mappa contiene le regioni candidate solo per gli eventi che le hanno!!
    for(auto ev: *events_not_satisfy_EC){
        cout<<"event not sat EC----------"<< ev<<endl;
    }


    return events_not_satisfy_EC;
	//return true;

}


vector<Region>* Label_splitting_module::do_label_splitting(map<int, vector<Region*> *>* middle_set_of_states,map<int,vector< int >*>* number_of_bad_events,vector<int>* events_not_satisfy_EC) {

    //per ogni evento
    //per ogni stato intermedio se è compreso nel set delle intersezioni
    //prendilo per label splitting

    //modifica con mappa non vettore perchè per alcuni eventi faccio erase di tutto!!!
    vector<int>* events_type=new vector<int>(middle_set_of_states->size());
    Region* candidate_region= nullptr;
    int num_bad_event_min=-1;
    vector<Region>* candidate_regions=new vector<Region>();

    cout << "middle TOT: " << endl;
    vector<Region *>::iterator it;
    //per ogni evento
    for (auto event: *events_not_satisfy_EC) {
        cout << "EVENTO: " << event << "*************"<<endl;
        int pos = 0;
        candidate_region= nullptr;
        num_bad_event_min=-1;

        //auto set_forced_to_be_a_region;
        for (it = middle_set_of_states->at(event)->begin(); it < middle_set_of_states->at(event)->end(); ++it) {
            cout << "middle:" << endl;
            printRegion(**it);
            if ( is_bigger_than(*it, regions_intersection->at(event)) ) {
                cout << "erase" << endl;
                *middle_set_of_states->at(event)->erase(it, it);
            } else { //lo stato mi va bene
                //cache e ricalcolo solo se non c'è(avevo fatto break per no cross)
                // mi prendo il numero di eventi che violano per ogni set di stati!!
                cout << "eveent: " << event << endl;
                auto vec_ptr = number_of_bad_events->at(event);
                if ((*vec_ptr)[pos] == -1) {

                    printRegion( **it );
                    //ricalcola
                    for(auto e  : *middle_set_of_states)
                        (*events_type)[e.first]=branch_selection(&(ts_map->at(e.first)),*it);

                    set_number_of_bad_events(events_type,event, vec_ptr+pos);
                }

                if( (*vec_ptr)[pos]!=0 ){
                    if(candidate_region == nullptr || num_bad_event_min>(*vec_ptr)[pos]){

                        candidate_region= *it;
                        num_bad_event_min=(*vec_ptr)[pos];

                    }else if(num_bad_event_min==(*vec_ptr)[pos]){
                        //controlla in base alla size delle 2 regioni
                        if(candidate_region->size()>= (*it)->size()){
                            candidate_region= *it;
                            num_bad_event_min=(*vec_ptr)[pos];
                        }
                    }
                }

                cout<<"NUMBER candidate: "<< num_bad_event_min<<endl;
            }

            pos++;
        }


        // aggiungo candidate_region alle regioni del mio evento
        cout<<"___________________________REGIONE CANDIDATA__________________"<<endl;
        if(candidate_region!= nullptr)
            print(*candidate_region);
        cout<<"num: " << num_bad_event_min<<endl;

        candidate_regions->push_back(*candidate_region);

    }

    delete events_type;
    delete middle_set_of_states;

    for(auto elem:*number_of_bad_events){
        delete elem.second;
    }
    delete number_of_bad_events;

    cout<<"Regioni candidate******"<<endl;
    for(const auto &reg: *candidate_regions){
        printRegion(reg);
    }
    cout<<"******"<<endl;

    return candidate_regions;
}


int Label_splitting_module::branch_selection(List_edges *list, Region *region) {
    // quale ramo devo prendere tra ok, nocross oppure 2 rami? (per un evento)
    vector<int> *trans = new vector<int>(4, 0);
    cout<<"DENTRO"<<endl;

    //num in-out-exit-enter
    const int in = 0;
    const int out = 1;
    const int exit = 2;
    const int enter = 3;

    for (auto t: *list) {
        if (region->find(t.first) != region->end()) { //il primo stato appartiene alla regione
            if (region->find(t.second) != region->end()) { //anche il secondo stato appartiene alla regione
                (*trans)[in]++;
                cout << t.first << "->" << t.second << " IN " << endl;
                //per no cross è ok, gli altri non si possono fare
            } else {
                (*trans)[exit]++;
                cout << t.first << "->" << t.second << " EXIT" << endl;
            }
        } else {//il primo non ci appartiene
            if (region->find(t.second) != region->end()) { //il secondo stato appartiene alla regione
                (*trans)[enter]++;
                cout << t.first << "->" << t.second << " ENTER" << endl;
            } else {
                (*trans)[out]++;
                cout << t.first << "->" << t.second << " OUT" << endl;
            }
        }
    }

    int it = 0;
    cout << ">> IN = 0/OUT = 1/EXIT = 2/ENTER = 3" << endl;
    for (auto i: *trans) {
        cout << "num trans " << it << ": " << i << endl;
        it++;
    }

    //gli Enter+in devono diventare per forza in(nocross)
    if (((*trans)[in] > 0 && (*trans)[enter] > 0) || ((*trans)[in] > 0 && (*trans)[exit] > 0) ||
        ((*trans)[enter] > 0 && (*trans)[exit] > 0)) {
        cout << "return no cross" << endl;
        delete trans;
        return NOCROSS;
    } else if ((*trans)[exit] > 0 && (*trans)[out] > 0) { //(exit-out)
        cout << "return exit_no cross" << endl;
        delete trans;
        return EXIT_NOCROSS;
    } else if ((*trans)[enter] > 0 && (*trans)[out] > 0) { //(enter-out)
        cout << "return enter_no cross" << endl;
        delete trans;
        return ENTER_NOCROSS;
    } else {
        cout << "return ok" << endl;
        delete trans;
        return OK;
    }

}

void Label_splitting_module::set_number_of_bad_events(vector<int>* event_type,int event, vector<int> * number_of_bad_events){
    //conta per ogni set di stati gli eventi bad
    // pair<int,Region*> *bad_events=new pair<int,Region*>;

    cout<<"SET BAD NUMBER per " <<event<<"********"<<endl;

    int counter=0;
    for(auto n: *event_type) {
        if (n != OK ) {
            counter++;
        }
    }

    (*number_of_bad_events)[0]=counter;

    cout<<"COUNTER: " <<counter<<endl;
}
