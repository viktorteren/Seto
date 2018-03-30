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
#include <queue>

typedef std::pair<int,int> Edge;

using namespace std;
using namespace boost;

//map: evento -> lista di coppie: (srcId, dstId)
typedef vector<Edge> Lista_archi;
typedef std::map<int, Lista_archi> Mappa;

typedef property<edge_name_t, int> event;
typedef adjacency_list<mapS, vecS, undirectedS,no_property,event> Graph;

#define OK 0
#define NOCROSS 1
#define EXIT_NOCROSS 2
#define ENTER_NOCROSS 3

int num_stati, num_transazioni, stato_iniziale, num_eventi;

typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

Mappa* mappa = new Mappa();
// declare a graph object
Graph g(0);
Vertex* vertex_array;

typedef set<int>* Region;
typedef set<int>* ER;
vector<ER>* ER_set = new  vector<ER>;
vector<Region>* pre_regions = new vector<Region>;

deque<Region> *queue_temp_regions;

void parser(){
    // Open the file:
    std::ifstream fin("../input.txt");

    assert(fin);

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

    num_eventi = (*mappa).size();

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

ER createER(int event){
    ER er = new set<int>;
    for(auto edge: (*mappa)[event]){
        (*er).insert(edge.first);
    }
    return er;
}

int event_type(Lista_archi* list){
 // quale ramo devo prendere tra ok, nocross oppure 2 rami?
}

void expand(Region region, int event){
    int* event_types = new int[num_eventi];
    for(auto e: *mappa){
        //controllo tutti, non è un ER
        if(e.first == -1)
            event_types[e.first] = event_type(&e.second);
        //è un ER non controllo l'evento relativo all'ER
        else if(e.first != event && e.first != -1) {
            event_types[event] = OK;
            event_types[e.first] = event_type(&e.second);
        }
    }
    int branch = OK;
    int pos;
    int type;
    for(int i = 0; i < num_eventi; i ++){
        type=event_types[i];
        if(type == NOCROSS){
            branch = NOCROSS;

            break;
        }
        if(type == EXIT_NOCROSS){
            if(branch == OK)
                branch = EXIT_NOCROSS;
        }
        else if(type == ENTER_NOCROSS){
            if(branch == OK)
                branch = ENTER_NOCROSS;
        }

    }
    if(branch == OK){
        (*pre_regions).push_back(region); //aggiunta pre-regione giusta
    }
    else if (branch == NOCROSS){
        //capire gli stati da aggiungere
        //l'operazione sta nella copia della regione puntata, l'espansione di tale regione e il ritorno di una nuova regione più grande
        //mettere l'unico ramo (regione successiva)
    }
    else{
        //aggiungere alla coda i 2 prossimi rami (2 regioni successive)
    }

}


int main()
{
    bool first;
    parser();

    queue_temp_regions = new deque<Region>;

    for(auto e : *mappa){
        ER er_temp = createER(e.first);
        (*ER_set).push_back(er_temp);
        first = true;

        while(!queue_temp_regions->empty()){
            //expand(); //sulla stessa regione  MA devo espandere su tutti se non è la prima volta!!!
            if(first){
                expand(er_temp, e.first);
                first = false;
            }
            else{
                expand(queue_temp_regions->front(), e.first);
                //tolgo l'elemento espanso dalla coda
                queue_temp_regions->pop_front();
            }


        }

    }



}
