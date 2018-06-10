
#include "../include/Label_splitting_module.h"
#include "../include/Merging_Minimal_Preregions_module.h"
#include "../include/Place_irredundant_pn_creation_module.h"
#include "../include/Pre_and_post_regions_generator.h"
#include "../include/Regions_generator.h"
#include <time.h>

int main(int argc, char **argv) {
  vector<string> args(argv, argv + argc);
  string file;
  if (argc == 1) {
    // default input
    file = "../test/input8.ts";
  } else if (argc == 2) {
    file = args[1];
  } else {
    cout << "Too many input arguments" << endl;
    exit(0);
  }
  TS_parser::parse(file);

  vector<Region> *candidate_regions;

   clock_t tStart = clock();

    auto tStart_partial = clock();

    auto rg = new Region_generator();
  map<int, vector<Region> *> *regions = rg->generate();

  vector<Region> *vector_regions = copy_map_to_vector(regions);

  auto t_region_gen= (double)(clock() - tStart_partial)/CLOCKS_PER_SEC;

    tStart_partial=clock();
    auto t_splitting=0.0;

  auto ls = new Label_splitting_module(regions, rg->get_ER_set());

  Pre_and_post_regions_generator *pprg;

  bool excitation_closure;

  set<int> *events = ls->is_excitation_closed();

  excitation_closure = events->empty();
  //cout << "EC*************" << excitation_closure << endl;

  set<int> *events_not_satify_EC = nullptr;
  map<int, ER> *new_ER;


  if (!excitation_closure) {
    //cout << " not exitation closed " << endl;
    candidate_regions = ls->do_label_splitting(
        rg->get_middle_set_of_states(), rg->get_number_of_bad_events(), events);
    //cout << "___________label splitting ok" << endl;

    events_not_satify_EC = new set<int>();
    auto pairs = rg->get_trees_init();

    for (auto el : *events) {
      events_not_satify_EC->insert(pairs->at(el));
    }

    t_splitting=(double)(clock() - tStart_partial)/CLOCKS_PER_SEC;

      tStart_partial=clock();

    pprg = new Pre_and_post_regions_generator(vector_regions, candidate_regions,
                                              rg->get_ER_set(),
                                              events_not_satify_EC);
    delete candidate_regions;

    new_ER = pprg->get_new_ER();
    delete rg->get_ER_set();
  } else {
      t_splitting=(double)(clock() - tStart_partial)/CLOCKS_PER_SEC;
      tStart_partial=clock();
    pprg = new Pre_and_post_regions_generator(vector_regions);
    new_ER = rg->get_ER_set();
  }

  delete events;
  delete events_not_satify_EC;

    auto t_pre_region_gen=(double)(clock() - tStart_partial)/CLOCKS_PER_SEC;

    map<int, set<Region *> *> *pre_regions = pprg->get_pre_regions();

    tStart_partial=clock();
    if(!excitation_closure) {

      ls->split_ts_map(&pprg->get_events_alias(), pre_regions);

      t_splitting = t_splitting + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

      /*cout << "main DEBUG TS_MAP SPLITTED" << endl;
      for (auto record : *ts_map) {
          cout << "evento:" << record.first << endl;
          for (auto tr : record.second) {
              cout << "trans: " << tr->first << ", " << tr->second << endl;
          }
      }*/
  }


  delete ls;
  delete rg;

  tStart_partial=clock();
  // Inizio modulo: ricerca di set irridondanti di regioni
  auto pn_module = new Place_irredundant_pn_creation_module(pre_regions);
  auto t_irred= (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

    tStart_partial=clock();

    auto essential_regions = pn_module->get_essential_regions();
  map<int, set<Region *> *> *irredundant_regions =
      pn_module->get_irredundant_regions();

  Merging_Minimal_Preregions_module *merging_module = nullptr;

  if (irredundant_regions != nullptr) {
    merging_module = new Merging_Minimal_Preregions_module(
        essential_regions, irredundant_regions, new_ER);
    // print_PN(essential_regions,irredundant_regions);
  } else {
    merging_module = new Merging_Minimal_Preregions_module(essential_regions,
                                                           nullptr, new_ER);
    // print_PN(essential_regions, nullptr);
  }

  auto merged_map = merging_module->get_merged_preregions_map();

  if (merged_map == nullptr) {
    merged_map = merging_module->get_total_preregions_map();
  }

  auto t_merge=(double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

  pprg->create_post_regions(merged_map);
  // pprg->create_post_regions(pprg->get_pre_regions());
  auto post_regions = pprg->get_post_regions();

  // restore_default_labels(merged_map, pprg->get_events_alias());
  // print_pn_dot_file( pprg->get_pre_regions(), post_regions,
  // pprg->get_events_alias(), file);
  print_pn_dot_file(merged_map, post_regions, pprg->get_events_alias(), file);



  // dealloco regions e tutti i suoi vettori
  for (auto record : *regions) {
    delete record.second;
  }
  delete regions;

  delete vector_regions;

  // cout << "fine ricerca " << endl;

  delete pn_module;
  delete pprg;

  delete merging_module;

  //int c = 0;
  for (auto el : *ts_map) {
    for (auto p : el.second) {
      delete p;
      // c++;
    }
  }
  // cout<<"C: "<<c<<endl;
  delete ts_map;

    printf("Time total: %.5fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    printf("Time region gen: %.5fs\n", t_region_gen);
    printf("Time splitting: %.5fs\n", t_splitting);
    printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
    printf("Time essential+irredundant: %.5fs\n", t_irred);
    printf("Time merge: %.5fs\n", t_merge);
}
