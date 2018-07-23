//
// Created by Viktor on 20/04/2018.
//

#include "../include/Pre_and_post_regions_generator.h"

using namespace Utilities;

Pre_and_post_regions_generator::Pre_and_post_regions_generator(
    vector<Region> *reg) {
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
    if (region->find(t->first) !=
        region->end()) { // il primo stato appartiene alla regione
      if (region->find(t->second) == region->end())
        return true;
    }
  }
  return false;
}

bool Pre_and_post_regions_generator::is_post_region(Edges_list *list,
                                                    Region *region) {
  /*cout<<"regione (is_post_region)"<<endl;
  println(*region);*/
  for (auto t : *list) {
    if (region->find(t->first) ==
        region->end()) { // il primo stato non appartiene alla regione
      if (region->find(t->second) != region->end())
        return true;
    }
  }
  return false;
}

void Pre_and_post_regions_generator::create_post_regions(
    map<int, set<Region *> *> *merged_pre_regions) {
  // record. first da ts_map è l'evento, record.second è la lista da passare a
  // is_post_region
  //cout << "mappa pre-regioni :" << endl;
  //print(*merged_pre_regions);
  post_regions = new map<int, set<Region *> *>();
  for (auto rec : *merged_pre_regions) {
    for (auto reg : *rec.second) {
      if (ts_map->find(rec.first) != ts_map->end()) {
        for (auto r : *ts_map) {
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
        cout << "Postregioni :" << endl;
        print(*post_regions);
        cout << "" << endl;
    }
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
  return post_regions;
}



