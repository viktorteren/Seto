//
// Created by ciuchino on 13/04/18.
//

#pragma once

#include "Utilities.h"
#include <map>
#include <set>

using namespace std;

typedef set<int> Region;
typedef std::pair<int, int> Edge;
typedef set<int> *ER;
typedef set<Edge *> Edges_list;

typedef std::map<int, Edges_list> My_Map;

extern My_Map *ts_map;
extern int num_states, initial_state, num_events, num_events_after_splitting;
extern unsigned int num_transactions;
extern int max_label;

class TS_parser {
  static void parse_TS(ifstream &fin);
  static void parse_DOT(ifstream &fin);
    static void parse_APT(ifstream &fin);
public:
  static void parse(string file);
};