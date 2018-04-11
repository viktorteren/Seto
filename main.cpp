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
typedef std::map<int, Lista_archi> My_Map;

typedef property<edge_name_t, int> event;
typedef adjacency_list<mapS, vecS, undirectedS,no_property,event> Graph;

/*#define OK 0
#define NOCROSS 1
#define EXIT_NOCROSS 2
#define ENTER_NOCROSS 3*/


const int OK=0;
const int NOCROSS= 1;
const int EXIT_NOCROSS= 2;
const int ENTER_NOCROSS= 3;

int num_stati, num_transazioni, stato_iniziale, num_eventi;

typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

My_Map* ts_map = new My_Map();

// declare a graph object
Graph g(0);
Vertex* vertex_array;

typedef set<int> Region;
typedef set<int>* ER;

vector<ER>* ER_set = new  vector<ER>;
vector<Region>* pre_regions = new vector<Region>;

vector<Region> *queue_temp_regions= new vector<Region>;


struct Branches_states_to_add{
    set<int>* states_to_add_exit_or_enter;
    set<int>* states_to_add_nocross;
};

//map< int , vector< set<int>* > * > *map_states_to_add= new map< int , vector< set<int>* > * > ();
map< int , Branches_states_to_add > *map_states_to_add= new map< int , Branches_states_to_add > ();
Branches_states_to_add* struct_states_to_add;
set<int>* states_to_add_enter;
set<int>* states_to_add_exit;
set<int>* states_to_add_nocross;

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
        if (ts_map->find(ev) == ts_map->end()){
            (*ts_map)[ev] = Lista_archi();
            //mappa.insert(Mappa::value_type(ev, Lista_archi()));
        }
        (*ts_map).at(ev).push_back(std::make_pair(src, dst));

    }

    num_eventi = (*ts_map).size();

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
    for(auto edge: (*ts_map)[event]){
        (*er).insert(edge.first);
        cout<< "CREATE ER: Insert state: " << edge.first <<endl;
    }

    for(auto i: *er){
        cout<< "S: " << i <<endl;
    }

    return er;
}

int event_type(Lista_archi* list, Region *region, int event){
 // quale ramo devo prendere tra ok, nocross oppure 2 rami? (per un evento)
    vector<int> *trans= new vector<int>(4,0);
    //prima di allocare nuovo spazio elimino il vecchio contenuto puntato
    delete struct_states_to_add;
    struct_states_to_add= new Branches_states_to_add();

    states_to_add_enter=new set<int>;
    states_to_add_exit=new set<int>;
    states_to_add_nocross=new set<int>;

    //num in-out-exit-enter
    const int in=0;
    const int out=1;
    const int exit=2;
    const int enter=3;


    for(auto t: *list){
        if( region->find(t.first) != region->end()){ //il primo stato appartiene alla regione
            if(region->find(t.second) != region->end()) { //anche il secondo stato appartiene alla regione
                (*trans)[in]++;
                cout<< t.first << "->" <<t.second << " IN " <<endl;
                //per no cross è ok, gli altri non si possono fare
            }
            else {
                (*trans)[exit]++;
                cout<< t.first << "->" <<t.second << " EXIT" <<endl;
                //per exit è ok
                //per no cross:
                (*states_to_add_nocross).insert(states_to_add_nocross-> begin(), t.second);
                cout<< "inserisco " << t.second << " per nocross " << endl;
            }
        }
        else {//il primo non ci appartiene
            if(region->find(t.second) != region->end()) { //il secondo stato appartiene alla regione
                (*trans)[enter]++;
                cout<< t.first << "->" <<t.second << " ENTER" << endl;
                //per il no cross devo aggiungere la sorgente di tutti gli archi entranti nella regione(enter diventa in)
                //mappa di int(evento) e vettore di puntatori a insiemi di stati da aggiungere
                (*states_to_add_nocross).insert(states_to_add_nocross-> begin(), t.first);
                cout<< "inserisco " << t.first << " per nocross " << endl;
                //per enter è già ok
                //per exit non si può fare
            }
            else {
                (*trans)[out]++;
                cout<< t.first << "->" <<t.second << " OUT" << endl;
                //per enter devo aggiungere la destinazione degli archi che erano out dalla regione
                (*states_to_add_enter).insert(states_to_add_enter-> begin(), t.second);
                cout<< "inserisco " << t.second << " per enter " <<endl;
                //per no cross è già ok
                //per exit:
                (*states_to_add_exit).insert(states_to_add_exit-> begin(), t.first);
                cout<< "inserisco " << t.first << " per exit " <<endl;
            }
        }
    }

    int it=0;
    cout<< ">> IN = 0/OUT = 1/EXIT = 2/ENTER = 3" << endl;
    for(auto i: *trans){
        cout<<"num trans "<< it <<": " << i <<endl ;
        it++;
    }

    //gli Enter+in devono diventare per forza in(nocross)
    if( ( (*trans)[in]>0 && (*trans)[enter]>0) || ((*trans)[in]>0 && (*trans)[exit]>0) || ( (*trans)[enter]>0 && (*trans)[exit]>0 ) ) {
        cout<<"return no cross"<<endl;

        struct_states_to_add->states_to_add_nocross=states_to_add_nocross;
        (*map_states_to_add)[event]= *struct_states_to_add;
        delete states_to_add_enter;
        delete states_to_add_exit;
       // delete states_to_add_nocross;
        delete trans;
        return NOCROSS;
    }
    else if( (*trans)[exit]>0  && (*trans)[out]>0 ){ //(exit-out)
        cout<<"return exit_no cross"<<endl;

        struct_states_to_add->states_to_add_exit_or_enter=states_to_add_exit;
        struct_states_to_add->states_to_add_nocross=states_to_add_nocross;
        (*map_states_to_add)[event]= *struct_states_to_add;
        delete states_to_add_enter;
      //  delete states_to_add_exit;
        //delete states_to_add_nocross; //NON POSSO DEALLOCARE PERCHé DOPO CI PUNTO E DA SEGFAULT (ma da memoryLeak--booo)
        delete trans;
        return EXIT_NOCROSS;
    }
    else if( (*trans)[enter]>0 && (*trans)[out]>0 ){ //(enter-out)
        cout<<"return enter_no cross"<<endl;

        //aggiungo gli stati da aggiungere per entry e no cross (ma li aggiunge alla coda la expand per controllare che sia il ramo giusto da prendere)
        struct_states_to_add->states_to_add_exit_or_enter=states_to_add_enter;
        struct_states_to_add->states_to_add_nocross=states_to_add_nocross;
        (*map_states_to_add)[event]= *struct_states_to_add;

        //delete states_to_add_enter;
        //delete states_to_add_exit;
        delete states_to_add_nocross;
        delete trans;
        return ENTER_NOCROSS;
    }
    else {
        cout<<"return ok"<<endl;
        //se DEALLOCO DA INVALID FREE ALTIRMENTI MEMLEAK ???!!!
       /* delete states_to_add_enter;
        delete states_to_add_exit;
        delete states_to_add_nocross;*/
        delete trans;
        return OK;
    }


}

void expand(Region *region, int event){
    int* event_types = new int[num_eventi];
    int last_event_2braches=-1;
    int last_event_nocross=-1;
    Region* expanded_regions = new Region[2];

    cout << "|||REGIONE: " ;
    for(auto i: (*region)) {
        cout << i << " ";
    }
    cout << endl;


    for(auto e: *ts_map){
        cout<< "EVENTO: " << e.first << endl;
        //controllo tutti, non è un ER
        if(e.first != event || event == -1) {
            cout << "Non è ER" << endl;
            event_types[e.first] = event_type(&e.second, region, e.first);

            //se è no cross non controllo gli altri eventi
            if(event_types[e.first]==NOCROSS) break;
        }
        //è un ER non controllo l'evento relativo all'ER
        else if(e.first == event) {
            cout << " è un ER di " << event <<endl;
            event_types[event] = OK;
            //event_types[e.first] = event_type(&e.second,region, e.first);
        }
    }
    int branch = OK;
    int type;
    for(int i = 0; i < num_eventi; i ++){
        type=event_types[i];
        if(type == NOCROSS){
            cout<<"Break per no_cross " <<endl;
            branch = NOCROSS;
            last_event_nocross=i;
            break;
        }
        if(type == EXIT_NOCROSS){
            if(branch == OK) {
                branch = EXIT_NOCROSS;
                last_event_2braches=i;
            }
            cout<<"2 rami exit" <<endl;
        }
        else if(type == ENTER_NOCROSS){
            if(branch == OK) {
                branch = ENTER_NOCROSS;
                last_event_2braches=i;
            }
            cout<<"2 rami enter" <<endl;
            cout << "branch: " << branch << endl;
        }

    }

    if(branch == OK){
        cout<<"OK" <<endl;
        (*pre_regions).push_back(*region); //aggiunta pre-regione giusta
    }
    else if (branch == NOCROSS){
        cout<<"RAMO UNICO NO CROSS" <<endl;
        for(auto state: *region){
            (*expanded_regions).insert(state);
            cout<< "inserisco nella extended Reg: " << state << endl;
        }
        for(auto i: *region){
            cout << "Stati region " << i <<endl ;
        }

        cout << "map states to add size: " << (*map_states_to_add).size() << endl;


        //todo: qui dovrei aggiungere gli stati giusti

        Branches_states_to_add branches=(*map_states_to_add)[last_event_nocross];

        cout<<"qui";

        cout << "dim primo set vettore: " << branches.states_to_add_nocross->size() << endl;
       /* for(auto state : *branches.states_to_add_nocross) {
            cout << "stati vet: " << state <<endl;
        }*/

        for(auto state : *branches.states_to_add_nocross ) {
            expanded_regions[0].insert(state);
        }

        for(auto i: expanded_regions[0]){
            cout << "Stato della regione espansa NOCROSS " << i <<endl ;
        }

        //todo: il seguente push_back dovregge aggiungere l'elemento mancante nella coda che deve essere preparato nel modo giusto
        queue_temp_regions->push_back(*expanded_regions);

        for(auto i: *queue_temp_regions){
            cout << "coda:"  <<endl ;
            for(auto state : i)
                cout << "stati" << state <<endl ;
        }
        //capire gli stati da aggiungere
        //l'operazione sta nella copia della regione puntata, l'espansione di tale regione e il ritorno di una nuova regione più grande
        //mettere l'unico ramo (regione successiva)
    }
    else{
        //aggiungere alla coda i 2 prossimi rami (2 regioni successive)
        //if(branch==ENTER_NOCROSS){

            //per il no cross devo aggiungere la sorgente di tutti gli archi entranti nella regione(enter diventa in)
            //per enter devo aggiungere la destinazione degli archi che erano out dalla regione

            cout<< "RAMO EXIT/ENTER_NOCROSS " << endl;
           // (*region).insert(region->begin(), 1);
            cout << "dim region " << (*region).size() << endl;

            cout<< "point reg " << region << endl;
            //cout << "pint exp " << (*expanded_regions)[0];

            //RAMO 1 (NO_CROSS)
            for(auto state: *region){
                (*expanded_regions).insert(state);
                cout<< "inserisco nella extended Reg: " << state << endl;
            }

            for(auto i: *region){
                cout << "Stati region " << i <<endl ;
            }

            cout << "map states to add size: " << (*map_states_to_add).size() << endl;

            cout<<"last event 2 branch "<< last_event_2braches << endl;

            Branches_states_to_add branches=(*map_states_to_add)[last_event_2braches];

            cout<<"qui";

            cout << "dim primo set vettore: " << branches.states_to_add_nocross->size() << endl;
            for(auto state : *branches.states_to_add_nocross) {
                cout << "stati vet: " << state <<endl;
            }

           for(auto state : *branches.states_to_add_nocross ) {
               expanded_regions[0].insert(state);
           }

            for(auto i: expanded_regions[0]){
                cout << "Stato della regione espansa NOCROSS " << i <<endl ;
            }

            queue_temp_regions->push_back(*expanded_regions);

            for(auto i: *queue_temp_regions){
                cout << "coda:"  <<endl ;
                for(auto state : i)
                cout << "stati" << state <<endl ;
            }

            //RAMO 2 (EXIT/ENTER)

            for(auto state: *region){
                (*(expanded_regions+1)).insert(state);
                cout<< "inserisco nella extended Reg: " << state << endl;
            }

            for(auto state : *branches.states_to_add_exit_or_enter ) {
                expanded_regions[1].insert(state);
            }

            for(auto i: expanded_regions[1]){
                cout << "Stato della regione espansa ENTER " << i <<endl ;
            }

            queue_temp_regions->push_back(*(expanded_regions+1));

        //}

    }
    delete[] event_types;
    delete[] expanded_regions;
}


int main()
{
    bool first;
    parser();
    int pos=0;

    for(auto e : *ts_map){
        ER er_temp = createER(e.first);
        //ER er_temp = createER(2);
        (*ER_set).push_back(er_temp);

        //espando la prima volta - la regione coincide con ER
        expand(er_temp, e.first);
        //expand(er_temp, 2);

    cout<< "*********************************: pos: " << pos <<" reg queue size " << queue_temp_regions->size() << endl;
        while(pos < queue_temp_regions->size() /*&& queue_temp_regions->size()<2*/){

            if((*queue_temp_regions)[pos].size()!=num_stati)
                expand( &((*queue_temp_regions)[pos]), -1 );
            else  (*pre_regions).push_back((*queue_temp_regions)[pos]);

            cout<< "POSIZIONEEEE**********************************: ";
            cout<< "POSIZIONEEEE**********************************: " << pos <<"reg size " << queue_temp_regions->size() << endl;
                pos++;

                //tolgo l'elemento espanso dalla coda
               // queue_temp_regions->pop_front();
        }
        queue_temp_regions->clear();

        delete er_temp;
    }

    for(auto pre_reg: *pre_regions){
        cout<< "PREGEGION ER di : " << endl;
        for(auto state: pre_reg){
            cout<<" State: "<< state<<endl;
        }
    }

    delete ER_set;
    delete map_states_to_add;
    delete queue_temp_regions;
    delete pre_regions;
    delete states_to_add_enter;
    delete states_to_add_exit;
    delete states_to_add_nocross;
}
