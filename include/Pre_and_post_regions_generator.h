/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once

#include "Utilities.h"

class Pre_and_post_regions_generator {
public:
  explicit Pre_and_post_regions_generator(vector<Region> *reg);
  ~Pre_and_post_regions_generator();
  map<int, set<Region *> *> *get_pre_regions();
  map<int, set<Region *> *> *get_post_regions();
  static bool is_pre_region(Edges_list *list, Region *region);
  void create_post_regions(map<int, set<Region *> *> *merged_pre_regions);
  map<int, Region *> *create_post_regions_for_SM(map<int, set<Region *> *> *pre_regions_SM);
  map<int, Region *> *create_post_regions_for_SM(map<int, Region *> *pre_regions_SM);
  map<int, Region *> *create_pre_regions_for_SM(SM *sm, set<int> *removed_events);
  map<int, set<Region *>* >* create_post_regions_for_FCPN(map<int, set<Region *> *> *pre_regions_FCPN);

private:
  vector<Region> *regions;
  map<int, set<Region *> *> *pre_regions; //the map contain <event e, regions which covers the event e>
  map<int, set<Region *> *> *post_regions = nullptr;

  void create_pre_regions();
  bool is_post_region(Edges_list *list, Region *region);

};
