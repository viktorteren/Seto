
#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <cassert>

#include "Minimal_pre_region_generator.h"
#include "TS_parser.h"

    typedef std::pair<int, int> Edge;

    using namespace std;
//using namespace boost;
    using namespace pre_region_gen;

//map: evento -> lista di coppie: (srcId, dstId)
    typedef vector<Edge> Lista_archi;
    typedef std::map<int, Lista_archi> My_Map;

/*typedef property<edge_name_t, int> event;
typedef adjacency_list<mapS, vecS, undirectedS,no_property,event> Graph;*/


//typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;


// declare a graph object
/*Graph g(0);
Vertex* vertex_array;*/


    int main() {
        bool first;
        TS_parser::parse();
        int pos = 0;
        Minimal_pre_region_generator *mg = new Minimal_pre_region_generator();
        //Minimal_pre_region_generator::Minimal_pre_region_generator(num_stati,num_eventi);
        mg->generate();

        //la generate non ritorna la mappa che è pubblica(visibile) alle altre classi e dovrà essere deallocata alla fine


        delete mg;
    }
