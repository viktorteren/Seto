#include <boost/graph/edge_list.hpp>
#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <cassert>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <tuple>
#include <map>

typedef std::pair<int,int> Edge;

using namespace std;
using namespace boost;

//map: evento -> lista di coppie: (srcId, dstId)
typedef vector<Edge> Lista_archi;
typedef std::map<int, Lista_archi> Mappa;

typedef property<edge_name_t, int> event;
typedef adjacency_list<mapS, vecS, undirectedS,no_property,event> Graph;

typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

Mappa* mappa = new Mappa();
// declare a graph object
Graph g(0);
Vertex* vertex_array;
vector<set<int>*>* ER_set = new  vector<set<int>*>;
vector<set<int>*>* pre_regions = new vector<set<int>*>;

void parser(){
    // Open the file:
    std::ifstream fin("../input.txt");

    assert(fin);
// Declare variables:
    int num_stati, num_transazioni, stato_iniziale;

// Read defining parameters:
    fin >> num_stati ;
    fin >> num_transazioni;
    fin >> stato_iniziale;

    /* cout << num_stati << endl;
     cout << num_transazioni << endl;
     cout << stato_iniziale << endl;*/

    vertex_array = new Vertex[num_stati];

    for(int i = 0; i < num_stati; i++){
        vertex_array[i]=boost::add_vertex(g);
    }

    int src, dst, ev;

    //aggiungo gli archi al grafo
    for (int i = 0; i < num_transazioni; ++i) {
        //add_edge(get<0>(transaction_array[i]), get<1>(transaction_array[i]),event(10), g);
        //add_edge(vertex_array[], vertex_array[],event(100), g);
        fin >> src;
        fin >> dst;
        fin >> ev;
        add_edge(vertex_array[src], vertex_array[dst], event(ev), g);
        //non c'è l'entry relativa all'evento ev
        if (mappa->find(ev) == mappa->end()){
            (*mappa)[ev] = Lista_archi();
            //mappa.insert(Mappa::value_type(ev, Lista_archi()));
        }
        (*mappa).at(ev).push_back(std::make_pair(src, dst));


    }

    property_map<Graph, edge_name_t>::type
            eventMap = get(edge_name_t(), g);

    boost::graph_traits< Graph >::edge_iterator e_it, e_end;
    for(std::tie(e_it, e_end) = boost::edges(g); e_it != e_end; ++e_it)
    {
        std::cout << boost::source(*e_it, g) << " "
                  << boost::target(*e_it, g) << " "
                  << eventMap[*e_it] << std::endl;
    }


    fin.close();
}

set<int>* createER(int event){
    set<int>* er = new set<int>;
    for(auto edge: (*mappa)[event]){
        (*er).insert(edge.first);
    }
}


int main()
{

    parser();

    for(auto e : *mappa){
        (*ER_set).push_back(createER(e.first));
    }



}
