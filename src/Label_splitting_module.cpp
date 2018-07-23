//
// Created by ciuchino on 13/04/18.
//

#include "../include/Label_splitting_module.h"

Label_splitting_module::Label_splitting_module(
        map<int, set<Region*> *> *pre_regions, map<int, ER> *er_set,map<int, vector<Region *> *> *middle_set_of_states) {
    this->pre_regions = pre_regions;
    this->ER_set = er_set;
    this->middle_set_of_states=middle_set_of_states;
};

Label_splitting_module::~Label_splitting_module() {
    for (auto el : *regions_intersection_map) {
        delete el.second;
    }
    delete regions_intersection_map;

};

set<int> *Label_splitting_module::is_excitation_closed() {

    //cout << "*************IS EXCITATION CLOSED*****************" << endl;

    regions_intersection_map = new map<int, set<int> *>() ;

    auto events_not_satisfy_EC = new set<int>;

    // per ogni evento
    // se per un evento non vale che l'intersezione è uguale all'er la TS non è
    // exitation-closed
    // bool res=true;

    //for (auto item : *pre_regions) {
    //cout<<"num evens after splitting "<<num_events_after_splitting<<endl;
    for(int event=0;event<num_events_after_splitting;++event){
        //cout << "event: " << item.first;
        //auto event = item.first;
        auto er = ER_set->at((event));
        //cout << "ER at" << event << " : " << endl;
        //println(*er);
        //l'evento non ha preregioni allora non vale la EC
        if(pre_regions->find(event)==pre_regions->end()){
            events_not_satisfy_EC->insert(event);
            (*regions_intersection_map)[event] = new set<int>();
        } else {
            auto intersec = regions_intersection(pre_regions->at(event));
            (*regions_intersection_map)[event] = intersec;
            //cout << "Intersec at" << event << " :" << endl;
            //println(*intersec);
            if (!(are_equal(er, intersec))) {
                // cout << "regione delle'evento:" << event;
                events_not_satisfy_EC->insert(event);
                // res=false;
            }
        }
    }

    // ritorna chi non soddisfa così faccio lo splitting solo per quegli eventi
    // la mappa contiene le regioni candidate solo per gli eventi che le hanno!!
     /*for (auto ev : *events_not_satisfy_EC) {
       cout << "event not sat EC----------" << ev << endl;
     }*/

    if(events_not_satisfy_EC->size()!=0){
        cout<<"Ts non è chiuso rispetto all'eccitazione"<<endl;
        cout << "" << endl;
    }
    else{
        cout<<"Ts è chiuso rispetto all'eccitazione"<<endl;
        cout << "" << endl;
    }

    return events_not_satisfy_EC;
    // return true;
}

map<int, pair<int,Region*>* > * Label_splitting_module::candidate_search(
        map<int, vector<int> *> *number_of_bad_events,
        set<int> *events_not_satisfy_EC) {

    // per ogni evento
    // per ogni stato intermedio se è compreso nel set delle intersezioni
    // prendilo per label splitting

    // modifica con mappa non vettore perchè per alcuni eventi faccio erase di
    // tutto!!!
    vector<int> *events_type = new vector<int>(middle_set_of_states->size());
    Region *candidate_region = nullptr;
    pair<int,Region*>* pair_reg= nullptr;

    int num_bad_event_min;
    auto candidate_regions = new map<int , pair<int,Region*>*>();

    //cout << "middle TOT: " << endl;
    vector<Region *>::iterator it;
    // per ogni evento che non soddisfa EC
    for (auto event : *events_not_satisfy_EC) {
        if(pair_reg != nullptr){
            delete pair_reg;
            pair_reg = nullptr;
        }


        //cout << "EVENTO: " << event << "*************" << endl;

        int pos = 0;
        candidate_region = nullptr;
        num_bad_event_min = -1;

        //caso speciale intersezione vuota perchè evento non ha preregioni,scelgo ER come candidata
        if(regions_intersection_map->at(event)->empty()){
            candidate_region=*middle_set_of_states->at(event)->begin();
            pair_reg=new pair<int,Region*>(pos,candidate_region);
        }else {

            // auto set_forced_to_be_a_region;
            for (it = middle_set_of_states->at(event)->begin();
                 it < middle_set_of_states->at(event)->end(); ++it) {
                //cout << "middle:" << endl;
                //println(**it);

                if (is_bigger_than_or_equal_to(*it, regions_intersection_map->at(event))) {
                    // cout << "erase" << endl;
                    *middle_set_of_states->at(event)->erase(it, it);
                } else { // lo stato mi va bene
                    // cache e ricalcolo solo se non c'è(avevo fatto break per no cross)
                    // mi prendo il numero di eventi che violano per ogni set di stati!!
                    //cout << "event: " << event << endl;
                    auto vec_ptr = number_of_bad_events->at(event);
                    if ((*vec_ptr)[pos] == -1) {

                        // println(**it);
                        // ricalcola
                        for (auto e : *middle_set_of_states)
                            (*events_type)[e.first] =
                                    branch_selection(&(ts_map->at(e.first)), *it);

                        set_number_of_bad_events(events_type, event, vec_ptr, pos);
                    }

                    if ((*vec_ptr)[pos] != 0) {
                        if (candidate_region == nullptr ||
                            num_bad_event_min > (*vec_ptr)[pos]) {
                            candidate_region = *it;
                            delete pair_reg;
                            pair_reg = new pair<int, Region *>(pos, candidate_region);
                            num_bad_event_min = (*vec_ptr)[pos];

                        } else if (num_bad_event_min == (*vec_ptr)[pos]) {
                            // controlla in base alla size delle 2 regioni
                            if (candidate_region->size() >= (*it)->size()) {
                                candidate_region = *it;
                                delete pair_reg;
                                pair_reg = new pair<int, Region *>(pos, candidate_region);
                                num_bad_event_min = (*vec_ptr)[pos];
                            }
                        }
                    }

                    //cout << "NUMBER candidate: " << num_bad_event_min << endl;
                }

                pos++;
            }

        }

        // aggiungo candidate_region alle regioni del mio evento
        /*cout << "___________________________REGIONE CANDIDATA__________________"
             << endl;*/
        if (candidate_region != nullptr) {
            //println(*candidate_region);
            //cout << " num: " << num_bad_event_min << endl;
            //cout << "pos: " << pair_reg->first << endl;
            (*candidate_regions)[event]=new pair<int, Region *>(pair_reg->first, pair_reg->second);
        }
    }

    delete pair_reg;

    delete events_type;

    /*cout << "Regioni candidate******" << endl;
    for (auto rec : *candidate_regions) {
        cout << "pos: " << rec.second->first << " ";
        println(*rec.second->second);
    }*/
//  cout << "******" << endl;

    return candidate_regions;
}

int Label_splitting_module::branch_selection(Edges_list *list, Region *region) {
    // quale ramo devo prendere tra ok, nocross oppure 2 rami? (per un evento)
    vector<int> *trans = new vector<int>(4, 0);
    // cout << "DENTRO" << endl;

    // num in-out-exit-enter
    const int in = 0;
    const int out = 1;
    const int exit = 2;
    const int enter = 3;

    for (auto t : *list) {
        if (region->find(t->first) !=
            region->end()) { // il primo stato appartiene alla regione
            if (region->find(t->second) !=
                region->end()) { // anche il secondo stato appartiene alla regione
                (*trans)[in]++;
                //  cout << t->first << "->" << t->second << " IN " << endl;
                // per no cross è ok, gli altri non si possono fare
            } else {
                (*trans)[exit]++;
                //  cout << t->first << "->" << t->second << " EXIT" << endl;
            }
        } else { // il primo non ci appartiene
            if (region->find(t->second) !=
                region->end()) { // il secondo stato appartiene alla regione
                (*trans)[enter]++;
                // cout << t->first << "->" << t->second << " ENTER" << endl;
            } else {
                (*trans)[out]++;
                // cout << t->first << "->" << t->second << " OUT" << endl;
            }
        }
    }

    int it = 0;
    // cout << ">> IN = 0/OUT = 1/EXIT = 2/ENTER = 3" << endl;
    /*for (auto i : *trans) {
      cout << "num trans " << it << ": " << i << endl;
      it++;
    }*/

    // gli Enter+in devono diventare per forza in(nocross)
    if (((*trans)[in] > 0 && (*trans)[enter] > 0) ||
        ((*trans)[in] > 0 && (*trans)[exit] > 0) ||
        ((*trans)[enter] > 0 && (*trans)[exit] > 0)) {
        //cout << "return no cross" << endl;
        delete trans;
        return NOCROSS;
    } else if ((*trans)[exit] > 0 && (*trans)[out] > 0) { //(exit-out)
        // cout << "return exit_no cross" << endl;
        delete trans;
        return EXIT_NOCROSS;
    } else if ((*trans)[enter] > 0 && (*trans)[out] > 0) { //(enter-out)
        // cout << "return enter_no cross" << endl;
        delete trans;
        return ENTER_NOCROSS;
    } else {
        //cout << "return ok" << endl;
        delete trans;
        return OK;
    }
}

void Label_splitting_module::set_number_of_bad_events(
        vector<int> *event_type, int event, vector<int> *number_of_bad_events, int pos) {
    // conta per ogni set di stati gli eventi bad
    // pair<int,Region*> *bad_events=new pair<int,Region*>;

    // cout << "SET BAD NUMBER per " << event << "********" << endl;

    int counter = 0;
    for (auto n : *event_type) {
        if (n != OK) {
            counter++;
        }
    }

    (*number_of_bad_events)[pos] = counter;

    //cout << "COUNTER: " << counter << endl;
}

void Label_splitting_module::split_ts_map(map<int, pair<int, Region *> *> *candidate_regions,
                                          map<int, int> *event_alias,
                                          map<int, map<int, int> *> *event_violations,
                                          map<int, map<int, vector<Edge *> *> *> *trans_violations) {
    //per la regione candidata migliore per ogni regione se è contenuta, eventi uscente da candidate rimangono cosi evnti uscenti da new region vengono aggiunti
    Region *best_region = nullptr;
    map<int, pair<int, Region *> *>::iterator it;

    int best_region_root_event = -1;
    int best_region_id;
    //map<int, pair<int,Region*>* > *
    for (it = candidate_regions->begin(); it != candidate_regions->end(); ++it) {
        //cout<<"regione cand"<<endl;
        //println(*(*it).second->second);
        if (best_region == nullptr) {
            best_region = (*it).second->second;
            best_region_root_event = (*it).first;
            best_region_id = (*it).second->first;
        } else {
            if (best_region->size() > (*it).second->second->size()) {
                best_region = (*it).second->second;
                best_region_root_event = (*it).first;
                best_region_id = (*it).second->first;
            }
        }
    }

    auto regions_vec = Utilities::copy_map_to_vector2(pre_regions);
    vector<Region*>::iterator it2;
    //Region *violations_region;
    for (it2 = regions_vec->begin(); it2 < regions_vec->end(); ++it2) {
        auto reg = (*it2);
        if (is_bigger_than(reg, best_region)) {
            //Region *new_region = region_difference(*it2, *best_region);
            map<int, int>::iterator it;

            //per l'evento che violava
            // elimino transazioni che violavano se sono per quell'evento e creo quelle nuove più l'alias altrimenti niente

            auto total_events = static_cast<int>(ts_map->size());
            //cout << "total events !!!: " << total_events << endl;
            auto event = (*event_violations)[best_region_root_event]->at(best_region_id);

            if (ts_map->find(total_events) == ts_map->end()) {
                (*ts_map)[total_events] = Edges_list();
            }
            // cout << "effettuo uno splitting" << endl;
            //crea alias e nuove trans poi erase
            //ts size è il nuovo evento
            if (event_alias->find(event) != event_alias->end()) {
                (*event_alias)[total_events] = event_alias->at(event);
            } else {
                (*event_alias)[total_events] = event;
            }
             // cout << "aggiungo ad aliases: " << total_events << " : " << event << endl;

            auto to_erase = new set<Edge *>();

            auto transitions = trans_violations->at(event)->at(best_region_id);

            for (auto tr: *transitions) {

                auto pair = new Edge();
                pair->first = tr->first;
                pair->second = tr->second;
                (*ts_map)[total_events].insert(pair);
                to_erase->insert(tr);
            }

            for (auto el : *to_erase) {
                delete el;
                ts_map->at(event).erase(el);
            }
            delete to_erase;

            break;
        }
    }

        delete regions_vec;

        //cout << "ts_map size dopo lo split: " << ts_map->size() << endl;
   // }

}

