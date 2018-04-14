//
// Created by viktor on 12/04/18.
//
#include "Minimal_pre_region_generator.h"
#include "TS_parser.h"

using namespace pre_region_gen;

Minimal_pre_region_generator::Minimal_pre_region_generator() {
    //ts_map = input_map;
    ER_set = new  vector<ER>;
    regions = new vector<Region>;
    queue_temp_regions= new vector<Region>;
    map_states_to_add= new map< int , Branches_states_to_add > ();
    pre_regions= new map < int , vector<Region*>* > ();
    //states = num_states;
    //events = num_events;

}

Minimal_pre_region_generator::~Minimal_pre_region_generator() {

    //delete ts_map;
    delete ER_set;
    delete map_states_to_add;
    delete queue_temp_regions;
    delete regions;

}

struct Branches_states_to_add {
    set<int> *states_to_add_exit_or_enter;
    set<int> *states_to_add_nocross;
};

ER Minimal_pre_region_generator::createER(int event){

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

bool Minimal_pre_region_generator::is_pre_region(List_edges *list, Region *region, int event) {
    for(auto t: *list){
        if( region->find(t.first) != region->end()){ //il primo stato appartiene alla regione
            if(region->find(t.second) == region->end()) { //il secondo stato non appartiene alla regione
                return true;
            }
            else
                return false;
        } else
            return false;
    }
    return false;
}


void Minimal_pre_region_generator::printRegion(const Region& region){
    for(auto state: region){
        cout << state << ", ";
    }
    cout << endl;
}

int Minimal_pre_region_generator::branch_selection(List_edges *list, Region *region, int event){
    // quale ramo devo prendere tra ok, nocross oppure 2 rami? (per un evento)
    vector<int> *trans= new vector<int>(4,0);

   //delete struct_states_to_add;
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
        //delete states_to_add_nocross;
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
        delete states_to_add_exit;

        //delete states_to_add_nocross;
        delete trans;
        return ENTER_NOCROSS;
    }
    else {
        cout<<"return ok"<<endl;

        //non ho aggiunto alla struttura - elimino per non perdere
        delete states_to_add_nocross;
        delete states_to_add_exit;
        delete states_to_add_enter;

        delete struct_states_to_add;

        delete trans;
        return OK;
    }
}

bool Minimal_pre_region_generator::minimal_region(Region& new_region) {
    int cont;
    for (auto region: *regions) {
        cont = 0;
        if(region.size() <= new_region.size()){
            for (auto state: region) {
                if(new_region.find(state) == new_region.end()){
                    break;
                }
                else{
                    cont ++;
                }
            }
            if(cont == region.size())
                return false;
        }
    }
    return true;
}


void Minimal_pre_region_generator::remove_bigger_regions(Region& new_region){
    int cont;
    Region region;


    for(int i = 0; i< regions->size(); i++){
        region = regions->at(i);
        cont = 0;
        if(region.size() > new_region.size()){
            for (auto state: new_region) {
                if(region.find(state) == region.end()){
                    break;
                }
                else{
                    cont ++;
                }
            }
            if(cont == region.size()){
                cout << "eliminazione regione vecchia" << endl;
                //remove old too big region
                regions->erase(regions->begin()+i);
                i--;
            }

        }
    }
}
bool Minimal_pre_region_generator::region_in_queue(Region& new_region){
    int cont=0;
    for (auto region: *queue_temp_regions) {
        cont = 0;
        if(region.size() == new_region.size()){
            for (auto state: region) {
                if(new_region.find(state) == new_region.end()){
                    break;
                }
                else{
                    cont ++;
                }
            }
            if(cont == region.size())
                return true;
        }
    }
    return false;
}

void Minimal_pre_region_generator::expand(Region *region, int event){
    int* event_types = new int[num_events];
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
            event_types[e.first] = branch_selection(&e.second, region, e.first);

            //se è no cross non controllo gli altri eventi
            if(event_types[e.first]==NOCROSS) break;
        }
            //è un ER non controllo l'evento relativo all'ER
        else if(e.first == event) {
            cout << " è un ER di " << event <<endl;
            event_types[event] = OK;
            //event_types[e.first] = branch_selection(&e.second,region, e.first);
        }
    }
    int branch = OK;
    int type;
    for(int i = 0; i < num_events; i ++){
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
        if(minimal_region(*region)) {
            cout << "adding minimal region" << endl;
            //metodo che elimina regioni che contengono la regione appena inserita (NON TESTATO)
            //todo: fare dei test
            remove_bigger_regions(*region);
            (*regions).push_back(*region); //aggiunta regione giusta
        }
        else{
            cout << "not adding region" << endl;
        }

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

        //TODO se la temp regione da inserire c'è già non la inserisco
        if( !region_in_queue(*expanded_regions) ) {
            queue_temp_regions->push_back(*expanded_regions);
            cout<<"Regione aggiunta alla coda"<<endl;
        }
        else {
            cout<<"Regione non aggiunta alla coda(già presente)"<<endl;
        }

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

        if( !region_in_queue(*expanded_regions) ) {
            queue_temp_regions->push_back(*expanded_regions);
            cout<<"Ramo1: Regione aggiunta alla coda"<<endl;
        }
        else {
            cout<<"Ramo1: Regione non aggiunta alla coda(già presente)"<<endl;
        }

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


        if( !region_in_queue( *(expanded_regions+1) ) ) {
            queue_temp_regions->push_back( *(expanded_regions+1) );
            cout<<"Ramo2: Regione aggiunta alla coda"<<endl;
        }
        else {
            cout<<"Ramo 2 :Regione non aggiunta alla coda(già presente)"<<endl;
        }

        //}

    }

    delete[] event_types;
    delete[] expanded_regions;
}

void Minimal_pre_region_generator::create_pre_regions(){
    cout << "--------------------------------------------------- CREATING OF PRE-REGIONS --------------------------------------------" << endl;
    //per ogni evento
        //per ogni regione
            //guardo se è una pre-regione per tale evento
                //se si aggiungo alla mappa

    vector<Region>::iterator it;
    for(auto record: *ts_map){
        //cout << "evento: " << record.first << endl;
        /*for(auto region: *regions){
            //printRegion(region);
            if(is_pre_region(&record.second, &region, record.first)){*/
        for(it=regions->begin(); it!=regions->end();++it){
            Region* region= &(*it);
                if(is_pre_region(&record.second, region, record.first)){
                    //aggiungo la regione alla mappa
                    if (pre_regions->find(record.first) == pre_regions->end()){
                        (*pre_regions)[record.first] = new vector<Region *> ();
                    }

                cout << &region << endl;
                //cout << ((*pre_regions)[record.first]) << endl;
                (*pre_regions)[record.first]->push_back(region);
                cout << "pre_regions size " << (*pre_regions).size() << endl;
               /* for(auto region: *((*pre_regions)[record.first])){
                    cout << "Evento: " << record.first << endl;
                    printRegion(*region);
                }*/
                }
        }

    }

    for(auto record: *pre_regions){
        cout << "Event: " << record.first << endl;
        for(auto region: *record.second){
            printRegion(*region);
        }
    }

}

map<int, vector<Region*> *>* Minimal_pre_region_generator::generate(){

    int pos=0;

    for(auto e : *ts_map ){
        ER er_temp = createER(e.first);
        // ER er_temp = createER(2);
        (*ER_set).push_back(er_temp);

        //espando la prima volta - la regione coincide con ER
        expand(er_temp, e.first);
        //expand(er_temp, 2);

        cout<< "*********************************: pos: " << pos <<" reg queue size " << queue_temp_regions->size() << endl;
        while(pos < queue_temp_regions->size() /*&& queue_temp_regions->size()<2*/) {

            if ((*queue_temp_regions)[pos].size() != num_transactions)
                expand(&((*queue_temp_regions)[pos]), -1);
            else  (*regions).push_back((*queue_temp_regions)[pos]);

            cout<< "POSIZIONEEEE**********************************: ";
            cout<< "POSIZIONEEEE**********************************: " << pos <<"reg size " << queue_temp_regions->size() << endl;
            pos++;

            //tolgo l'elemento espanso dalla coda
            // queue_temp_regions->pop_front();
        }
        queue_temp_regions->clear();

        delete er_temp;
    }

    for(auto pre_reg: *regions){
        //todo: aggiungere il collegamento qui tra la pre-regione e gli eventi associati ???
        cout<< "PREREGION ER di : ???" << endl;
        for(auto state: pre_reg){
            cout<<" State: "<< state<<endl;
        }
    }

    //creazione delle pre-regioni
    create_pre_regions();

    for(auto item: *pre_regions){
        for(auto item1: *item.second){
            cout << "prova" << item1 <<endl;
        }
    }

    return pre_regions;
};
