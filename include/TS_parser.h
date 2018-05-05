//
// Created by ciuchino on 13/04/18.
//

#ifndef PROGETTO_SSE_TS_PARSER_H
#define PROGETTO_SSE_TS_PARSER_H


#include "Utilities.h"

typedef std::pair<int, int> Edge;

using namespace std;

typedef vector<Edge> Lista_archi;
typedef std::map<int, Lista_archi> My_Map;

extern My_Map* ts_map;
extern int num_states, num_transactions, initial_state,num_events;

class TS_parser{
public:
    static void parse();
};

#endif