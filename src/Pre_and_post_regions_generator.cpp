/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Pre_and_post_regions_generator.h"

using namespace Utilities;

Pre_and_post_regions_generator::Pre_and_post_regions_generator(vector<Region> *reg) {
  regions = reg;
  pre_regions = new map<int, set<Region *> *>();
  // post_regions= new map < int , set<Region*>* > ();
    create_pre_regions();
}

/*
Pre_and_post_regions_generator::Pre_and_post_regions_generator(
    vector<Region> *reg, vector<Region*> *candidate_regions,
    map<int, ER> *Er_set, set<int> *events) {
  regions = reg;
  pre_regions = new map<int, set<Region*> *>();

  events_to_split = events;

    create_pre_regions();
  //create_pre_regions(candidate_regions);
  er_set = create_ER_after_splitting(Er_set, events);
}
*/

Pre_and_post_regions_generator::~Pre_and_post_regions_generator() {
  for (auto el : *pre_regions) {
    delete el.second;
  }
  delete pre_regions;

  if(post_regions != nullptr){
      for (auto el : *post_regions) {
          delete el.second;
      }
      delete post_regions;
  }


  /*for (auto el : *added_regions_ptrs) {
    delete el;
  }*/
  //delete added_regions_ptrs;

  //delete events_alias;
}

bool Pre_and_post_regions_generator::is_pre_region(Edges_list *list,
                                                   Region *region) {
    //cout << "regione (is_pre_region)" << endl;
    //println(*region);
    for (auto t : *list) {
        if (region->find(t->first) != region->end()) { // il primo stato appartiene alla regione
            if (region->find(t->second) == region->end())
                return true;
        }
    }
    return false;
}

__attribute__((unused)) bool Pre_and_post_regions_generator::is_pre_region(const Edges_list& list,
                                                   Region region) {
    //cout << "regione (is_pre_region)" << endl;
    //println(*region);
    for (auto t : list) {
        if (region.find(t->first) !=
            region.end()) { // il primo stato appartiene alla regione
            if (region.find(t->second) == region.end())
                return true;
        }
    }
    return false;
}

bool Pre_and_post_regions_generator::is_post_region(Edges_list *list,
                                                    Region *region) {
    for (auto t : *list) {
        if (region->find(t->first) == region->end()) { // il primo stato non appartiene alla regione
            if (region->find(t->second) != region->end())
                return true;
        }
    }
    return false;
}

void Pre_and_post_regions_generator::create_post_regions(map<int, set<Region *> *> *merged_pre_regions) {
    // record. first da ts_map è l'evento, record.second è la lista da passare a
    // is_post_region
    //cout << "mappa pre-regioni :" << endl;
    //print(*merged_pre_regions);
    if(post_regions != nullptr){
        for(auto rec: *post_regions){
            delete rec.second;
        }
        delete post_regions;
    }
    post_regions = new map<int, set<Region *> *>();
    for (auto rec : *merged_pre_regions) {
        for (auto reg : *rec.second) {
            if (ts_map->find(rec.first) != ts_map->end()) {
                for (const auto& r : *ts_map) {
                    if (is_post_region(&(ts_map->at(r.first)), reg)) {
                        if (post_regions->find(r.first) == post_regions->end()) {
                            (*post_regions)[r.first] = new set<Region *>();
                        }
                        (*post_regions)[r.first]->insert(reg);
                    }
                }
            } else {
                //cout << "ts_map non contiene " << rec.first << endl;
            }
        }
    }
    if(print_step_by_step) {
        cout << "Postregions :" << endl;
        print(*post_regions);
        cout << "" << endl;
    }
}

map<int, Region *>* Pre_and_post_regions_generator::create_post_regions_for_SM(
        map<int, set<Region *> *> *pre_regions_SM) {
    // record. first da ts_map è l'evento, record.second è la lista da passare a
    // is_post_region
    //cout << "mappa pre-regioni :" << endl;
    //print(*merged_pre_regions);
    auto post_regions_SM = new map<int, Region *>();
    for (auto rec : *pre_regions_SM) {
        for (auto reg : *rec.second) {
            if (ts_map->find(rec.first) != ts_map->end()) {
                for (const auto& r : *ts_map) {
                    if (is_post_region(&(ts_map->at(r.first)), reg)) {
                        (*post_regions_SM)[r.first] = reg;
                    }
                }

            } else {
                //cout << "ts_map non contiene " << rec.first << endl;
            }
        }
    }
    /*if(print_step_by_step) {
        cout << "Postregions :" << endl;
        print(*post_regions_SM);
        cout << "" << endl;
    }*/
    return post_regions_SM;
}

map<int, set<Region *>* >* Pre_and_post_regions_generator::create_post_regions_for_FCPN(
        map<int, set<Region *> *> *pre_regions_FCPN) {
    // record. first da ts_map è l'evento, record.second è la lista da passare a
    // is_post_region
    //cout << "mappa pre-regioni :" << endl;
    //print(*merged_pre_regions);
    auto post_regions_FCPN = new map<int, set<Region *>*>();
    for (auto rec : *pre_regions_FCPN) {
        for (auto reg : *rec.second) {
            if (ts_map->find(rec.first) != ts_map->end()) {
                for (const auto& r : *ts_map) {
                    if (is_post_region(&(ts_map->at(r.first)), reg)) {
                        if((*post_regions_FCPN).find(r.first) == (*post_regions_FCPN).end()){
                            (*post_regions_FCPN)[r.first] = new set<Region *>();
                        }
                        (*post_regions_FCPN)[r.first]->insert(reg);
                    }
                }

            } else {
                //cout << "ts_map non contiene " << rec.first << endl;
            }
        }
    }
    /*if(print_step_by_step) {
        cout << "Postregions :" << endl;
        print(*post_regions_SM);
        cout << "" << endl;
    }*/
    return post_regions_FCPN;
}

map<int, Region *>* Pre_and_post_regions_generator::create_post_regions_for_SM(map<int, Region *> *pre_regions_SM) {
    // record. first da ts_map è l'evento, record.second è la lista da passare a
    // is_post_region
    //cout << "mappa pre-regioni :" << endl;
    //print(*merged_pre_regions);
    auto post_regions_SM = new map<int, Region *>();
    for (auto rec : *pre_regions_SM) {
        auto reg = rec.second;
        if (ts_map->find(rec.first) != ts_map->end()) {
            for (const auto& r : *ts_map) {
                if (is_post_region(&(ts_map->at(r.first)), reg)) {
                    (*post_regions_SM)[r.first] = reg;
                }
            }
        } else {
            cerr << "ts_map non contiene " << rec.first << endl;
        }
    }
    /*if(print_step_by_step) {
        cout << "Postregions :" << endl;
        print(*post_regions_SM);
        cout << "" << endl;
    }*/
    return post_regions_SM;
}

map<int, Region *> * Pre_and_post_regions_generator::create_pre_regions_for_SM(SM *sm, set<int> *removed_events){
    auto pre_regions_SM = new map<int, Region *>;
    set<Region*>::iterator it;
    for(auto record: *ts_map){
        int event = record.first;
        if(removed_events->find(event) == removed_events->end()) {
            for (it = sm->begin(); it != sm->end(); ++it) {
                Region *region = *it;
                if (is_pre_region(&record.second, region)) {
                    (*pre_regions_SM)[event] = region;
                }
            }
        }
    }
    return pre_regions_SM;
}

map<int, set<Region*> *> * Pre_and_post_regions_generator::create_pre_regions_for_FCPN(SM *FCPN, set<int> *removed_events){
    auto pre_regions_FCPN = new map<int, set<Region*> *>;
    set<Region*>::iterator it;
    for(auto record: *ts_map){
        int event = record.first;
        if(removed_events->find(event) == removed_events->end()) {
            for (it = FCPN->begin(); it != FCPN->end(); ++it) {
                Region *region = *it;
                if (is_pre_region(&record.second, region)) {
                    if(pre_regions_FCPN->find(event) == pre_regions_FCPN->end()){
                        (*pre_regions_FCPN)[event] = new set<Region *>();
                    }
                    (*pre_regions_FCPN)[event]->insert(region);
                }
            }
        }
    }
    return pre_regions_FCPN;
}

void Pre_and_post_regions_generator::create_pre_regions() {
    //cout << "--------------------------------------------------- CREATING OF PRE-REGIONS --------------------------------------------" << endl;

    //per ogni evento
    //per ogni regione
    //guardo se è una pre-regione per tale evento
    // se si aggiungo alla mappa

    vector<Region>::iterator it;
    for(auto record: *ts_map){
        //cout << "evento: " << record.first << endl;
        for(it=regions->begin(); it!=regions->end();++it){
            Region* region= &(*it);
            if(is_pre_region(&record.second, region)){
                //se l'evento non era presente nella mappa creo lo spazioo per il relativo set di regioni
                if (pre_regions->find(record.first) == pre_regions->end()){
                    (*pre_regions)[record.first] = new set<Region *> ();
                }

                //cout << &region << endl;
                //cout << ((*pre_regions)[record.first]) << endl;
                //aggiungo la regione alla mappa
                if((*pre_regions)[record.first]->find(region) == (*pre_regions)[record.first]->end()){
                    (*pre_regions)[record.first]->insert(region);
                    //cout << "inserisco " << &(*region) << endl;
                    //println(*region);
                }
            }
        }

    }

    /*for (auto record: *pre_regions) {
        cout << "Event: " << record.first << endl;
        for (auto region: *record.second) {
            println(*region);
        }
    }*/
}

map<int, set<Region *> *> *Pre_and_post_regions_generator::get_pre_regions() {
  return pre_regions;
}

map<int, set<Region *> *> *Pre_and_post_regions_generator::get_post_regions() {
    if(post_regions == nullptr)
        create_post_regions(pre_regions);
    return post_regions;
}



