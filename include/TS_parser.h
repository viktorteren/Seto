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
extern int num_states, num_transactions, initial_state,num_events;

class TS_parser{
	static void parse_TS(ifstream& fin);
	static void parse_DOT(ifstream& fin);
public:
    static void parse(string file);
};