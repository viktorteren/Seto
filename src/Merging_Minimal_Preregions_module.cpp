//
// Created by ciuchino on 09/05/18.
//

#include "../include/Merging_Minimal_Preregions_module.h"

Merging_Minimal_Preregions_module::Merging_Minimal_Preregions_module(
        map<int, set<Region *> *> *essential_preregions,
        map<int, set<Region *> *> *irredundant_preregions, map<int, ER> *ER) {
    er = ER;

    merging_2_maps(essential_preregions, irredundant_preregions);
    merged_pre_regions_map = merging_preregions(ER);
}

Merging_Minimal_Preregions_module::~Merging_Minimal_Preregions_module() {
    for (auto el : *total_pre_regions_map)
        delete el.second;
    delete total_pre_regions_map;

    if (merged_pre_regions_map != nullptr) {
        for (auto el : *merged_pre_regions_map)
            delete el.second;
        delete merged_pre_regions_map;
        delete union_ptr;
    }

    for (auto el : *er)
        delete el.second;
    delete er;
}

void Merging_Minimal_Preregions_module::merging_2_maps(
        map<int, set<Region *> *> *first, map<int, set<Region *> *> *second) {

//  cout << "MERGIN ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

    total_pre_regions_map = new map<int, set<Region *> *>();

    /*cout << "first" << endl;
    for (auto el : *first) {
        cout << "ev " << el.first << endl;
        for (auto reg : *el.second)
            println(*reg);
    }*/

    //second può essere nullptr
    /*cout << "  SECOND" << endl;
    for (auto el:*second) {
        cout << "ev " << el.first << endl;
        for (auto reg:*el.second)
            println(*reg);
    }*/

    if (second != nullptr) {
        for (int event = 0; event < num_events_after_splitting; event++) {
            // cout << "evento:" << event << endl;

            // trovo entrambi gli eventi
            if (first->find(event) != first->end() &&
                second->find(event) != second->end()) {

                auto merged_vector = new vector<Region *>(first->at(event)->size() +
                                                          second->at(event)->size());

                set_union(first->at(event)->begin(), first->at(event)->end(),
                          second->at(event)->begin(), second->at(event)->end(),
                          merged_vector->begin());

                /*cout << "merged vector: ";
                for (auto el : *merged_vector)
                  println(*el);*/

                (*total_pre_regions_map)[event] =
                        new set<Region *>(merged_vector->begin(), merged_vector->end());
                delete merged_vector;
                /*cout<<"entrambe"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                    println(*el);*/
            }
                // l'evento è solo in first(essential)
            else if (first->find(event) != first->end()) {
                auto merged_vector = new vector<Region *>(first->at(event)->size());

                (*total_pre_regions_map)[event] = new set<Region *>(
                        first->at(event)->begin(), first->at(event)->end());
                delete merged_vector;
                /*cout<<"first"<<endl;
                for(auto el: *total_pre_regions_map->at(event))
                        println(*el);*/
            }
                // l'evento è solo in second(irredundant)
            else if (second->find(event) != second->end()) {
                auto merged_vector = new vector<Region *>(second->at(event)->size());

                (*total_pre_regions_map)[event] = new set<Region *>(
                        second->at(event)->begin(), second->at(event)->end());
                delete merged_vector;
                /* cout<<"secodn"<<endl;
                 for(auto el: *total_pre_regions_map->at(event))
                     println(*el);*/
            }
        }
    } else {
        for (auto record : *first) {
            auto event = record.first;
            (*total_pre_regions_map)[event] =
                    new set<Region *>(first->at(event)->begin(), first->at(event)->end());
        }
    }
    /*
    cout << "debug merging 2 mappe" << endl;
    for (auto el : *total_pre_regions_map) {
        cout << "ev: " << el.first << endl;
        for (auto r : *el.second)
            println(*r);
    }*/
}

map<int, set<Region *> *> *Merging_Minimal_Preregions_module::get_merged_preregions_map() {
    return merged_pre_regions_map;
}

map<int, set<Region *> *> *Merging_Minimal_Preregions_module::get_total_preregions_map() {
    return total_pre_regions_map;
}

map<int, set<Region *> *> *Merging_Minimal_Preregions_module::merging_preregions(map<int, set<int> *> *ER_map) {

    //cout << "MERGING PREREGIONS___________" << endl;

    set<Region *> *preregions_set = copy_map_to_set(total_pre_regions_map);
    Region *reg_union = nullptr;
    int cont_preregions = 0;

    /* for (auto el : *preregions_set) {
       println(*el);
     }*/

    // per ogni coppia di preregioni
    // per ogni evento
    // controlla che valga EC con la nuova regione
    // se vale

    auto tmp_map = new map<int, set<Region *> *>();
    for (auto record : *total_pre_regions_map) {
        auto set = record.second;
        (*tmp_map)[record.first] = new std::set<Region *>(set->begin(), set->end());
    }

    set<Region *>::iterator it;
    set<Region *>::iterator it2;

    for (it = preregions_set->begin(); it != preregions_set->end(); ++it) {
        for (it2 = next(it); it2 != preregions_set->end(); ++it2) {
            auto reg1 = *it;
            auto reg2 = *it2;

             /*cout << "r1: ";
             println(*reg1);
             cout << "r2: ";
             println(*reg2);*/
            cont_preregions = 0;
            // le regioni sono disgiunte!!!
            auto inter = regions_intersection(reg1, reg2);
            if (inter->empty()) {
                delete inter;
                if (!are_equal(reg1, reg2)) {

                    reg_union = regions_union(reg1, reg2);

                    if(reg_union->size() != num_states) {
                        bool ec_and_pre_region = false;


                        for (auto record : *total_pre_regions_map) {
                            auto event = record.first;

                            /*if(Pre_and_post_regions_generator::is_pre_region(&ts_map->at(event), reg_union)){
                                //cout << "---" << endl;
                                cout << "preregione per " << event << endl;
                                println(*reg_union);
                                cout << "---" << endl;
                                cont_preregions++;
                            }*/

                            // cout << "evento: " << event;
                            // auto tmp_set = new set<Region
                            // *>(total_pre_regions_map->at(event)->begin(),
                            //  total_pre_regions_map->at(event)->end());

                            auto tmp_set = new set<Region *>();

                            bool event_contains_reg = false;
                            for (auto reg : *total_pre_regions_map->at(event)) {
                                if (!are_equal(reg, reg1) && !are_equal(reg, reg2))
                                    tmp_set->insert(reg);
                                else
                                    event_contains_reg = true;
                            }
                            // inserisco l'unione se l'evento conteneva almeno una delle 2 regioni da unire
                            if (event_contains_reg) {
                                //if(Pre_and_post_regions_generator::is_pre_region(&ts_map->at(event), reg_union)) {
                                tmp_set->insert(reg_union);

                                // tmp_set->erase(tmp_set->find(reg1));
                                // tmp_set->erase(tmp_set->find(reg2));
                                // tmp_set->insert(reg_union);
                                /*if(event == 4){
                                    cout << "event 0" << endl;
                                    println(*tmp_set);
                                }*/

                                auto intersection = regions_intersection(tmp_set);
                                auto er = ER_map->at(event);

                                /* cout << "intersection:" << endl;
                                 println(*intersection);*/

                                // cout << "er:" << endl;
                                //println(*er);

                                // controlla ec ER(ev)==intersec(prereg(ev))
                                ec_and_pre_region = are_equal(intersection, er);
                                //cout << "ec and pre region = " << ec_and_pre_region << endl;

                                delete intersection;

                                // provo altre 2 regioni perchè per un evento non vale la EC
                                if (!ec_and_pre_region) {
                                    // cout << "BREAK l'evento non soddisfa EC-provo altre regioni"
                                    //<< endl;
                                    delete tmp_set;
                                    delete reg_union;
                                    break;
                                } else {
                                    delete (*tmp_map)[event];
                                    (*tmp_map)[event] = tmp_set;
                                    union_ptr = reg_union;
                                }
                                //}
                                //else{
                                //   ec_and_pre_region = false;
                                //   delete tmp_set;
                                //    break;
                                // }
                            } else {
                                delete tmp_set;
                                // cout << "event:" << event << endl;
                                // cout << "l'evento è ok perchè non ha cambiato le sue preregioni "
                                //   "con questa unione"
                                //  << endl;
                            }
                        }

                        // se per tutti gli eventi la coppia è ok faccio il merge effettivo
                        if (ec_and_pre_region) {
                            delete preregions_set;
                            cout << "merging ok" << endl;
                            //println(*reg1);
                            //println(*reg2);

                            /* for (auto el : *tmp_map) {
                              // cout << "ev: " << el.first << endl;
                               for (auto r : *el.second) {
                                 println(*r);
                               }
                             }*/

                            return tmp_map;
                        }
                    }
                }
            } else {
                delete inter;
                // cout << "le regioni non sono disgiunte" << endl;
            }
        }
    }

    // non ho fatto il merge
    cout << "not merging" << endl;

    for (auto el : *tmp_map) {
        delete el.second;
    }
    delete tmp_map;
    delete preregions_set;

    return nullptr;
}
