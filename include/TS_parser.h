//
// Created by ciuchino on 13/04/18.
//

#pragma once

#include "Utilities.h"

typedef std::pair<int, int> Edge;

using namespace std;

typedef vector<Edge> Edge_list;
typedef std::map<int, Edge_list> My_Map;

extern My_Map* ts_map;
extern int num_states, initial_state,num_events,num_events_after_splitting;
extern unsigned int num_transactions;

class TS_parser{
	static void parse_TS(ifstream& fin);
	static void parse_DOT(ifstream& fin);
public:
    static void parse(string file);
};