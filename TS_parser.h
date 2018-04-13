//
// Created by ciuchino on 13/04/18.
//

#ifndef PROGETTO_SSE_TS_PARSER_H
#define PROGETTO_SSE_TS_PARSER_H

#endif //PROGETTO_SSE_TS_PARSER_H

#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <map>
#include "assert.h"


typedef std::pair<int, int> Edge;

using namespace std;

//map: evento -> lista di coppie: (srcId, dstId)
typedef vector<Edge> Lista_archi;
typedef std::map<int, Lista_archi> My_Map;

class TS_parser{
public:
    static int num_states, num_transactions, initial_state,num_events;
    static My_Map* ts_map;
    static void parse();
};