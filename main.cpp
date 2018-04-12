
#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <cassert>

#include "Minimal_pre_region_generator.h"

typedef std::pair<int,int> Edge;

using namespace std;
//using namespace boost;
using namespace pre_region_gen;

//map: evento -> lista di coppie: (srcId, dstId)
typedef vector<Edge> Lista_archi;
typedef std::map<int, Lista_archi> My_Map;

/*typedef property<edge_name_t, int> event;
typedef adjacency_list<mapS, vecS, undirectedS,no_property,event> Graph;*/

int num_states, num_transactions, initial_state,num_events;
My_Map* ts_map;

//typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;


// declare a graph object
/*Graph g(0);
Vertex* vertex_array;*/


void parse(){

    // Open the file:

    ts_map = new My_Map();

    std::ifstream fin("../input.txt");

    assert(fin);

// Read defining parameters:
    fin >> num_states ;
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
        if (ts_map->find(ev) == ts_map->end()){
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


int main()
{
    bool first;
    parse();
    int pos=0;
    Minimal_pre_region_generator* mg = new Minimal_pre_region_generator(ts_map,num_events,num_states);
    //Minimal_pre_region_generator::Minimal_pre_region_generator(num_stati,num_eventi);
    mg->generate();

    //la generate non ritorna la mappa che è pubblica(visibile) alle altre classi e dovrà essere deallocata alla fine


    delete mg;
}
