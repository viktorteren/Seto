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
  create_pre_regions(nullptr);
}

Pre_and_post_regions_generator::Pre_and_post_regions_generator(
    vector<Region> *reg, vector<Region> *candidate_regions,
    map<int, ER> *Er_set, set<int> *events) {
  regions = reg;
  pre_regions = new map<int, set<Region *> *>();

  events_to_split = events;

  create_pre_regions(candidate_regions);
  er_set = create_ER_after_splitting(Er_set, events);
}

Pre_and_post_regions_generator::~Pre_and_post_regions_generator() {
  for (auto el : *pre_regions) {
    delete el.second;
  }
  delete pre_regions;

  for (auto el : *post_regions) {
    delete el.second;
  }
  delete post_regions;

  for (auto el : *added_regions_ptrs) {
    delete el;
  }
  delete added_regions_ptrs;

  delete events_alias;
}

bool Pre_and_post_regions_generator::is_pre_region(Edges_list *list,
                                                   Region *region) {
  cout << "regione (is_pre_region)" << endl;
  println(*region);
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

void Pre_and_post_regions_generator::remove_bigger_regions(Region &new_region) {
  unsigned int cont;
  Region region;

  for (unsigned int i = 0; i < regions->size(); i++) {
    region = regions->at(i);
    cont = 0;
    if (region.size() > new_region.size()) {
      for (auto state : new_region) {
        if (region.find(state) == region.end()) {
          break;
        } else {
          cont++;
        }
      }
      if (cont == new_region.size()) {
        cout << "eliminazione regione vecchia ";
        print(region);
        cout << " a causa di: ";
        println(new_region);
        // remove old too big region
        regions->erase(regions->begin() + i);
        i--;
      }
    }
  }
}

void Pre_and_post_regions_generator::create_post_regions(
    map<int, set<Region *> *> *merged_pre_regions) {
  // record. first da ts_map è l'evento, record.second è la lista da passare a
  // is_post_region
  cout << "mappa pre-regioni :" << endl;
  print(*merged_pre_regions);
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
        cout << "ts_map non contiene " << rec.first << endl;
      }
    }
  }
  cout << "mappa post-regioni :" << endl;
  print(*post_regions);
}

void Pre_and_post_regions_generator::create_pre_regions(
    vector<Region> *candidate_regions) {
  cout << "------------------------------------------------------------ "
          "DELETING OF NON MINIMAL REGIONS "
          "-------------------------------------------"
       << endl;
  vector<Region>::iterator it;
  for (it = regions->begin(); it < regions->end(); ++it) {
    Region *region = &(*it);
    remove_bigger_regions(*region);
  }

  cout << "--------------------------------------------------- CREATION OF "
          "PRE-REGIONS AND POST-REGIONS "
          "--------------------------------------------"
       << endl;
  // per ogni evento
  // per ogni regione
  // guardo se è una pre-regione per tale evento
  // se si aggiungo alla mappa
  added_regions_ptrs = new set<Region *>();
  int total_events_counter = ts_map->size() - 1;

  cout << "total events:" << total_events_counter << endl;

  events_alias = new map<int, int>();

  for (auto record : *ts_map) {
    cout << "_______________evento: " << record.first << endl;
    for (it = regions->begin(); it != regions->end(); ++it) {
      Region *region = &(*it);

      // se l'evento non era presente nella mappa creo lo spazio per il relativo
      // set di regioni
      if (pre_regions->find(record.first) == pre_regions->end()) {
        (*pre_regions)[record.first] = new set<Region *>();
      }

      if (candidate_regions != nullptr) {

        bool pre_region = is_pre_region(&record.second, region);
        // bool post_region = is_post_region(&record.second, region);
        cout << "dopo post region" << endl;

        // if (pre_region || post_region) {
        println(*it);
        // cout << "is pre region of" << record.first << endl;

        bool split = false;
        // bool to_delete=false;

        for (auto cand_reg : *candidate_regions) {
          cout << "regione candidata ";
          println(cand_reg);
          if (is_bigger_than(region, &cand_reg)) {

            Region *candidate_region = nullptr;
            Region *new_region = region_difference(*region, cand_reg);

            cout << "debug unica regione:" << endl;
            println(*new_region);

            if (!contains((*pre_regions)[record.first], new_region)) {
              if (is_pre_region(&record.second, new_region)) {

                auto reg_ptr = get_ptr_into(added_regions_ptrs, new_region);
                if (reg_ptr != nullptr) {
                  delete new_region;
                  new_region = reg_ptr;
                }

                // se l'evento era quello da splittare aggiungi la nuova regione
                // ad un nuovo evento e crea la mappa degli alias
                if (events_to_split->find(record.first) !=
                    events_to_split->end()) {
                  cout << "event to split" << record.first << endl;
                  (*pre_regions)[total_events_counter + 1] =
                      new set<Region *>();
                  (*pre_regions)[total_events_counter + 1]->insert(new_region);
                  (*events_alias)[record.first] = total_events_counter + 1;
                  total_events_counter++;
                } else {
                  (*pre_regions)[record.first]->insert(new_region);
                }

                added_regions_ptrs->insert(new_region);
                cout << "ho inserito new region(difference)" << endl;
                println(*new_region);
              } else
                delete new_region;

            } else
              delete new_region;

            if (!contains((*pre_regions)[record.first], &cand_reg)) {
              if (is_pre_region(&record.second, &cand_reg)) {

                auto reg2_ptr = get_ptr_into(added_regions_ptrs, &cand_reg);
                if (reg2_ptr != nullptr) {
                  candidate_region = reg2_ptr;
                } else {
                  candidate_region = new set<int>(cand_reg);
                }

                (*pre_regions)[record.first]->insert(candidate_region);
                added_regions_ptrs->insert(candidate_region);
                // cout << "ho inserito new region(cand reg)" << endl;
                // println(cand_reg);
                pre_region = true;
              }
            }

            split = true;
          }

          if (!split) {
            if (pre_region)
              (*pre_regions)[record.first]->insert(region);
            // if (post_region) (*post_regions)[record.first]->insert(region);
          }
        }

        cout << "qui" << endl;
      } else if (is_pre_region(&record.second, region)) {
        cout << "qui pre" << endl;
        (*pre_regions)[record.first]->insert(region);
      }
    }
  }

  // Per DEBUG:
  cout << "Pre regions:" << endl;
  for (auto e : *pre_regions) {
    cout << "event " << e.first << endl;
    for (auto r : *e.second)
      println(*r);
  }

  cout << "ALIAS " << endl;
  for (auto e : *events_alias) {
    cout << "event: " << e.first << "alias: " << e.second << endl;
  }

  num_events_after_splitting = pre_regions->size();
  cout << "NUM EVENTS AFTER SPLIT " << num_events_after_splitting << endl;
}

map<int, set<Region *> *> *Pre_and_post_regions_generator::get_pre_regions() {
  return pre_regions;
}

map<int, set<Region *> *> *Pre_and_post_regions_generator::get_post_regions() {
  return post_regions;
}

map<int, ER> *Pre_and_post_regions_generator::get_new_ER() { return er_set; };

map<int, ER> *Pre_and_post_regions_generator::create_ER_after_splitting(
    map<int, ER> *er_set_old, set<int> *splitted_events) {

  cout << "ER after splitting" << endl;
  auto er_set = new map<int, ER>();

  map<int, ER>::iterator it = er_set_old->begin();

  for (unsigned int i = 0; i < er_set_old->size(); i++) {
    // se l'evento non è stato splittato l'ER è quello di prima
    if (splitted_events->find(i) == splitted_events->end()) {
      (*er_set)[i] = (*it).second; // prendo il set dell'er vecchio
    }
    // l'ER è l'intersezione delle preregioni dell'evento(EC è valida)
    else {
      // cout<<"evento"<<i<<endl;
      auto event1 = i;
      delete (*it).second;
      (*er_set)[i] = regions_intersection(pre_regions->at(event1));
      if (events_alias->find(i) != events_alias->end()) {
        auto event2 = events_alias->at(i);
        (*er_set)[event2] = regions_intersection(pre_regions->at(event2));
      }
    }
    ++it;
  }

  cout << "debug: er" << endl;
  for (auto ev : *er_set) {
    println(*ev.second);
  }

  return er_set;
}

map<int, int> &Pre_and_post_regions_generator::get_events_alias() {
  return *events_alias;
}
