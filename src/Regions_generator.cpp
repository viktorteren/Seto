//
// Created by viktor on 12/04/18.
//
#include "../include/Regions_generator.h"


Region_generator::Region_generator() {
    //ts_map = input_map;
    ER_set = new  vector<ER>;
    regions = new map<int,vector<Region>*>;
    queue_temp_regions= new vector<Region>;
    map_states_to_add= new map< int , Branches_states_to_add *> ();

    middle_set_of_states=new map<int, vector<Region*> *>;
    //states = num_states;
    //events = num_events;
    number_of_bad_events = new map< int,vector< int >*>();
}

Region_generator::~Region_generator() {
//    delete map_states_to_add;
    delete queue_temp_regions;
}

vector<ER>* Region_generator::get_ER_set(){
    return ER_set;
}

ER Region_generator::createER(int event){
    auto er = new set<int>;
    for(auto edge: (*ts_map)[event]){
        (*er).insert(edge .first);
        cout<< "CREATE ER di "<< event<<" Insert state: " << edge.first <<endl;
    }

    for(auto i: *er){
        cout<< "S: " << i <<endl;
    }
    return er;
}

int Region_generator::branch_selection(List_edges *list, Region *region, int event){
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
        (*map_states_to_add)[event]= struct_states_to_add;

        delete states_to_add_enter;
        delete states_to_add_exit;
        //delete struct_states_to_add;
        // delete states_to_add_nocross;
        delete trans;
        return NOCROSS;
    }
    else if( (*trans)[exit]>0  && (*trans)[out]>0 ){ //(exit-out)
        cout<<"return exit_no cross"<<endl;

        struct_states_to_add->states_to_add_exit_or_enter=states_to_add_exit;
        struct_states_to_add->states_to_add_nocross=states_to_add_nocross;
        (*map_states_to_add)[event]= struct_states_to_add;

        delete states_to_add_enter;
        //delete struct_states_to_add;
        //delete states_to_add_exit;
        //delete states_to_add_nocross;
        delete trans;
        return EXIT_NOCROSS;
    }
    else if( (*trans)[enter]>0 && (*trans)[out]>0 ){ //(enter-out)
        cout<<"return enter_no cross"<<endl;

        //aggiungo gli stati da aggiungere per entry e no cross (ma li aggiunge alla coda la expand per controllare che sia il ramo giusto da prendere)
        struct_states_to_add->states_to_add_exit_or_enter=states_to_add_enter;
        struct_states_to_add->states_to_add_nocross=states_to_add_nocross;
        (*map_states_to_add)[event]= struct_states_to_add;

        //delete states_to_add_enter;
        delete states_to_add_exit;
        //delete struct_states_to_add;

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


bool Region_generator::region_in_queue(Region& new_region,int init_pos){
    int cont=0;

    vector<Region>::iterator it;
    it=queue_temp_regions->begin()+init_pos;

    for(it;it!=queue_temp_regions->end();it++){
    //for (auto region: *queue_temp_regions) {
        auto region=*it;
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

void Region_generator::expand(Region *region, int event,bool is_ER, int init_pos){
    auto event_types = new int[num_events];
    int last_event_2braches=-1;
    int last_event_nocross=-1;
    auto expanded_regions = new Region[2];

    cout << "|||REGIONE: " << region << " --- ";
    for(auto i: (*region)) {
        cout << i << " ";
    }
    cout << endl;

    for(int i=0;i<num_events;i++){
        event_types[i]=-1;
    }

    for(auto e: *ts_map){
        cout<< "EVENTO: " << e.first << endl;
        //controllo tutti, non è un ER
        if(e.first != event || !is_ER) {
            cout << "Non è ER" << endl;
            event_types[e.first] = branch_selection(&e.second, region, e.first);

            for(int i=0;i<num_events;i++){
                cout<< "event_type: " <<event_types[i];
            }

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

    //qui ho l'array che dice per ogni evento il tipo di ramo tranne per gli eventi dopo il no cross
    //dove ho -1 vuol dire che ho fatto il break
    cout<<"region pass:" <<region;
    set_number_of_bad_events(event_types,num_events,event);

    for(int i = 0; i < num_events; i ++){
        type=event_types[i];
        if(type==OK){
            cout<<"prova "<<i<<endl;
            map_states_to_add->at(1);
        }
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
        regions->at(event)->push_back(*region);
        //cout<<"push ok"<<endl;

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


        Branches_states_to_add *branches=(*map_states_to_add)[last_event_nocross];

        cout << "dim primo set vettore: " << branches->states_to_add_nocross->size() << endl;
        /* for(auto state : *branches.states_to_add_nocross) {
             cout << "stati vet: " << state <<endl;
         }*/

        for(auto state : *branches->states_to_add_nocross ) {
            expanded_regions[0].insert(state);
        }

        for(auto i: expanded_regions[0]){
            cout << "Stato della regione espansa NOCROSS " << i <<endl ;
        }

        //se la temp regione da inserire c'è già non la inserisco
        if( !region_in_queue(*expanded_regions,init_pos) ) {
            queue_temp_regions->push_back(*expanded_regions);
            cout<<"Regione aggiunta alla coda"<<endl;
        }
        else {
            cout<<"Regione non aggiunta alla coda(già presente)"<<endl;
        }

        vector<Region>::iterator it;
        for(it=queue_temp_regions->begin();it!=queue_temp_regions->end();it++){
            Region i=*it;
            Region* i_ptr=&(*it);
            //for(auto i: *queue_temp_regions){
            cout << "coda:" <<i_ptr <<endl ;
            for(auto state : i)
                cout << "stati" << state <<endl ;
        }
        //capire gli stati da aggiungere
        //l'operazione sta nella copia della regione puntata, l'espansione di tale regione e il ritorno di una nuova regione più grande
        //mettere l'unico ramo (regione successiva)

        //todo:così elimino sono la struttura della mappa per il ramo(evento) che ho scelto (e gli altri eventi???!! (error double free))
        delete branches;

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


        for(auto state: *region){
            (*expanded_regions).insert(state);
            (*(expanded_regions+1)).insert(state);
            cout<< "inserisco nella extended Reg: " << state << endl;
        }

        for(auto i: *region){
            cout << "Stati region " << i <<endl ;
        }


        //RAMO 1 (NO_CROSS)
        cout << "map states to add size: " << (*map_states_to_add).size() << endl;

        cout<<"last event 2 branch "<< last_event_2braches << endl;

        Branches_states_to_add *branches=(*map_states_to_add)[last_event_2braches];

        cout<<"qui";

        cout << "dim primo set vettore: " << branches->states_to_add_nocross->size() << endl;
        for(auto state : *branches->states_to_add_nocross) {
            cout << "stati vet: " << state <<endl;
        }

        for(auto state : *branches->states_to_add_nocross ) {
            expanded_regions[0].insert(state);
        }

        for(auto i: expanded_regions[0]){
            cout << "Stato della regione espansa NOCROSS " << i <<endl ;
        }

        if( !region_in_queue(*expanded_regions,init_pos) ) {
            queue_temp_regions->push_back(*expanded_regions);
            cout<<"Ramo1: Regione aggiunta alla coda"<<endl;
        }
        else {
            cout<<"Ramo1: Regione non aggiunta alla coda(già presente)"<<endl;
        }


        //RAMO 2 (EXIT/ENTER)

        /*for(auto state: *region){
            (*(expanded_regions+1)).insert(state);
            cout<< "inserisco nella extended Reg: " << state << endl;
        }*/

        for(auto state : *branches->states_to_add_exit_or_enter ) {
            expanded_regions[1].insert(state);
        }

        for(auto i: expanded_regions[1]){
            cout << "Stato della regione espansa ENTER " << i <<endl ;
        }


        if( !region_in_queue( *(expanded_regions+1) ,init_pos) ) {
            queue_temp_regions->push_back( *(expanded_regions+1) );
            cout<<"Ramo2: Regione aggiunta alla coda"<<endl;
        }
        else {
            cout<<"Ramo 2 :Regione non aggiunta alla coda(già presente)"<<endl;
        }

        vector<Region>::iterator it;
        for(it=queue_temp_regions->begin();it!=queue_temp_regions->end();it++){
            Region i=*it;
            Region* i_ptr=&(*it);
            //for(auto i: *queue_temp_regions){
            cout << "coda:" <<i_ptr <<endl ;
            for(auto state : i)
                cout << "stati" << state <<endl ;
        }
        //}
        delete branches;
    }



    delete[] event_types;
    delete[] expanded_regions;
}



map<int, vector<Region> *>* Region_generator::generate(){

    int pos=0;int init_pos=0;
    //evento e indice di fine
    auto queue_event_index=new map<int,int>;

    for(auto e : *ts_map ){
        ER er_temp = createER(e.first);
        (*ER_set).push_back(er_temp);
        (*regions)[e.first]=new vector<Region>();

        init_pos=pos;

        queue_temp_regions->push_back(*er_temp);
        //espando la prima volta - la regione coincide con ER
        cout<<"ptr expand prima: " << &((*queue_temp_regions)[pos]) <<endl;
        expand( &((*queue_temp_regions)[pos]), e.first , true, init_pos);
        cout<<"ptr expand: " << &((*queue_temp_regions)[pos]) <<endl;
        pos++;

        //expand(er_temp, 2);
        //aggiungi alla coda ER e sposta la posizione per avere l'albero completo
        //perchè voglio ER come middle _state

        cout<< "*********************************: pos: " << pos <<" reg queue size " << queue_temp_regions->size() << endl;
        while(pos < queue_temp_regions->size() /*&& queue_temp_regions->size()<2*/) {

            if ((*queue_temp_regions)[pos].size() != num_transactions)
                expand(&((*queue_temp_regions)[pos]), e.first, false, init_pos );
            else  regions->at(e.first)->push_back((*queue_temp_regions)[pos]);

            cout<< "POSIZIONEEEE**********************************: ";
            cout<< "POSIZIONEEEE**********************************: " << pos <<"reg size " << queue_temp_regions->size() << endl;
            pos++;

            //tolgo l'elemento espanso dalla coda
            // queue_temp_regions->pop_front();
        }
        (*queue_event_index)[e.first]=pos-1;

    }

    for(auto record: *regions){
        cout<< "REGION ER di " << record.first << endl;
        for(auto region: *record.second){
            Utilities::println(region);
        }
    }

    //creazione delle pre-regioni
    //create_pre_regions();

   /* vector<Region>::iterator it;
    for(it=queue_temp_regions->begin();it!=queue_temp_regions->end();it++) {
        cout << "elem ptr: " << &(*it) << endl;
    }*/


    set_middle_set_of_states(queue_event_index);

        cout<<"evento coda "<<endl;
       cout<< " " << &(  (*queue_temp_regions)[0] ) <<endl;
        cout<< " " << &(*queue_temp_regions)[0] <<endl;


    cout<<"\n debug middle"<<endl;
    for(auto e : *ts_map ){for(auto el_vec: *middle_set_of_states->at(e.first)){
            cout<<el_vec<<endl;
            Utilities::println(*el_vec);
        }

    }

    cout<<"debug number bad: " <<endl;
    for(auto e : *ts_map ){
        cout<<"evento " << e.first<<endl;
        for(auto n: *number_of_bad_events->at(e.first)){
            cout<<n<<endl;
        }
    }

    delete queue_event_index;
    return regions;
};


map<int, vector<Region*> *>* Region_generator::get_middle_set_of_states(){
    return middle_set_of_states;
}

void Region_generator::set_middle_set_of_states( map<int,int>* queue_event_index){

   /* //ER è compreso
    for(auto record:*queue_event_index){
        (*middle_set_of_states)[record.first] = new vector<Region*>();
        (*middle_set_of_states).at(record.first)->push_back(ER_set->at(record.first));
    }*/

    int init=0;
    int end=-1;

    for(auto record:*queue_event_index){
        cout<<"event: " <<record.first<<endl;
        if( (*middle_set_of_states)[record.first] == nullptr)
            (*middle_set_of_states)[record.first] = new vector<Region*>();

        init=end+1;
        end=(*queue_event_index)[record.first];
        cout<<"init: " <<init<<" end: " << end;
        for(int i=init;i<=end;i++){
          //  cout<<"debug: (queue[] set middle) " << &(*queue_temp_regions)[i];
            (*middle_set_of_states).at(record.first)->push_back( &(*queue_temp_regions)[i] );
        }

    }
}


void Region_generator::set_number_of_bad_events(int* event_type,int l,int event){
    //conta per ogni set di stati gli eventi bad
   // pair<int,Region*> *bad_events=new pair<int,Region*>;

    if(number_of_bad_events->find(event)==number_of_bad_events->end()) {
        (*number_of_bad_events)[event] = new vector<int>;
    }

    cout<<"\nSET BAD NUMBER per " <<event<<"********"<<endl;

    int counter=0;
    for(int i=0;i<l;i++) {
        if (event_type[i] != OK && event_type[i] != -1) {
            counter++;
        }
        else if (event_type[i] == -1) {//ho fatto il break per no cross
            counter=-1;
            break;
        }
    }

    //bad_events->first=counter;
   // bad_events->second=set;

   // cout<<"pair.second " << bad_events->second;
    cout<<"counter:"<<counter<<endl;

    number_of_bad_events->at(event)->push_back(counter);
}

map<int,vector< int >*>* Region_generator::get_number_of_bad_events(){
    return number_of_bad_events;
};



