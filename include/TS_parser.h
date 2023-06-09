/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#pragma once

#include "Utilities.h"
#include <map>
#include <set>
#include <string>

using namespace std;

typedef set<int> Region;
typedef std::pair<int, int> Edge;
typedef set<int> *ES;
typedef set<Edge *> Edges_list;

typedef std::map<int, Edges_list> My_Map;

extern My_Map *ts_map;
extern unsigned int num_states;
extern int initial_state, num_events_before_label_splitting, num_events_after_splitting;
extern unsigned int num_transactions;
extern map<int, string> *aliases_map_number_name;
extern map< string, int> *aliases_map_name_number;
extern map<int, string> *aliases_map_state_number_name;
extern map<string, int> *aliases_map_state_name_number;
extern bool g_input;
extern set<string> inputs;
extern set<string> outputs;
extern set<string> internals;
extern set<string> dummies;

class TS_parser {
    static void parse_TS(ifstream &fin);
    static void parse_SIS(ifstream &fin);
    static void add_new_label_with_alias(int num, const string& name);
    static void add_new_state_with_alias(int num, const string& name);
    static void check_wrong_end(const string& previous_temp, const string& temp);
public:
  static void parse(string file);
};