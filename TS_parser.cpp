//
// Created by ciuchino on 13/04/18.
//

#include "TS_parser.h"

My_Map* ts_map;
int num_states, num_transactions, initial_state,num_events;

void TS_parser::parse() {

    // Open the file:

    ts_map = new My_Map();

    std::ifstream fin("../input4.txt");

    assert(fin);

// Read defining parameters:
    fin >> num_states;
    fin >> num_transactions;
    fin >> initial_state;

    /* cout << num_stati << endl;
     cout << num_transazioni << endl;
     cout << stato_iniziale << endl;*/

//    vertex_array = new Vertex[num_stati];

    /*for(int i = 0; i < num_stati; i++){
        vertex_array[i]=boost::add_vertex(g);
    }*/

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

    num_events = (*ts_map).size();

    /* property_map<Graph, edge_name_t>::type
             eventMap = get(edge_name_t(), g);*/

    /* boost::graph_traits< Graph >::edge_iterator e_it, e_end;
     for(std::tie(e_it, e_end) = boost::edges(g); e_it != e_end; ++e_it)
     {
         std::cout << boost::source(*e_it, g) << " "
                   << boost::target(*e_it, g) << " "
                   << eventMap[*e_it] << std::endl;
     }*/


    fin.close();
}
