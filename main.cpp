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

typedef std::pair<int,int> Edge;

using namespace std;
using namespace boost;

typedef adjacency_list<vecS, vecS, bidirectionalS> Graph;

int main()
{
    // Open the file:
    std::ifstream fin("input.txt");

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

    typedef property<edge_name_t, int> event;
    typedef adjacency_list<mapS, vecS, undirectedS,no_property,event> Graph;

    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

    // declare a graph object
    Graph g(0);


   // typedef int Vertex_id;
   // typedef std::pair<Vertex_id , Vertex_id> Transaction;

    //Transaction* transaction_array = new Transaction[num_transazioni];

    Vertex* vertex_array=new Vertex[num_stati];

    for(int i =0; i<num_stati;i++){
        vertex_array[i]=boost::add_vertex(g);
    }

    //aggiungo gli archi al grafo
    for (int i = 0; i < num_transazioni; ++i)
        //add_edge(get<0>(transaction_array[i]), get<1>(transaction_array[i]),event(10), g);
        //add_edge(vertex_array[], vertex_array[],event(100), g);
        add_edge(vertex_array[0], vertex_array[1],event(100), g);

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
