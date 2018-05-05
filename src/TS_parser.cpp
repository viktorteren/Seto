//
// Created by ciuchino on 13/04/18.
//

#include "../include/TS_parser.h"

My_Map* ts_map;
int num_states, num_transactions, initial_state,num_events;

void TS_parser::parse() {

    // Open the file:

    ts_map = new My_Map();

    std::ifstream fin("../input2.txt");

    assert(fin);

// Read defining parameters:
    fin >> num_states;
    fin >> num_transactions;
    fin >> initial_state;
    int src, dst, ev;

    //aggiungo gli archi al grafo
    for (int i = 0; i < num_transactions; ++i) {
        //add_edge(get<0>(transaction_array[i]), get<1>(transaction_array[i]),event(10), g);
        //add_edge(vertex_array[], vertex_array[],event(100), g);
        fin >> src;
        fin >> dst;
        fin >> ev;
        // add_edge(vertex_array[src], vertex_array[dst], event(ev), g);
        //non c'è l'entry relativa all'evento ev
        if (ts_map->find(ev) == ts_map->end()) {
            (*ts_map)[ev] = Lista_archi();
            //mappa.insert(Mappa::value_type(ev, Lista_archi()));
        }
        (*ts_map)[ev].push_back(std::make_pair(src, dst));

    }
    num_events = static_cast<int>((*ts_map).size());
    fin.close();

    cout<<"DEGUG TS_MAP"<<endl;
    for(auto record:*ts_map){
        cout<<"evento:" <<record.first;
        for(auto tr:record.second){
            cout<<"trans: "<< tr.first << ", " << tr.second <<endl;
        }
    }
}
