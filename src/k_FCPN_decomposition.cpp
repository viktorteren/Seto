/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/k_FCPN_decomposition.h"

using namespace PBLib;
using namespace Minisat;
using namespace Utilities;


k_FCPN_decomposition::k_FCPN_decomposition(int number_of_ev,
                                       set<Region *> *regions,
                                       const string& file,
                                       Pre_and_post_regions_generator *pprg,
                                       map<int, int> *aliases,
                                       map<int, ER> *ER){
    cout << "=========[k-FCPN DECOMPOSITION MODULE]===============" << endl;
    number_of_regions = regions->size();
    number_of_events = number_of_ev;
    if(decomposition_debug){
        cout << "number of events: " << number_of_events << endl;
        cout << "number of regions: " << number_of_regions << endl;
    }
    /* Possible algorithm for the creation of one FCPN with SAT:
     * ALGORITHM STEPS:
     * do
     *      1) at least one region which covers each state: for each covered state by r1, r2, r3 create a clause (r1 v r2 v r3)
     *      2) FCPN constraint -> given the regions of a PN these cannot violate the constraint
     *      ALGORITHM:
     *          for aech ev
     *              for each r=pre(ev) -> place/region
     *                  if r have more than one exiting event
     *                      for each couple (r, pre(ev))
     *                          if r != pre(ev)
     *                              create clause (!r v !pre(ev))
     *      3) complete PN structure:
     *          given a sequence r1 -> a -> r2 we have the clause with the bound (r1 and r2 => a) that is (!r1 v !r2 v a)
     *      4) maximization function: number of new regions used in the result -> max covering
     *      5) minimize the total number of regions after having achieved the first result (todo: o forse anche fin da subito,
     *          ancora meglio trovare il valore sat per masimizzare la copertura e poi con quel valore minimizzare il numero di regioni
     *          ma se si combinano le 2 cose si trova il risultato migliore prima)
     *      6) solve the SAT problem decreasing the value of the region sum -> starting value is the sum of all regions
     *      7) decode result
     * 8) while !EC: previous results clauses are added as results to avoid in future
     */

    //EC encoding for simultaneous k-FCPN decomposition check
    /*
     * 1) each event need to have at lest one occurrence in the total amount of FCPNs
     *      for each event e calculate er(e) and pre(e)
     *          calculate all possible minimal sets of regions which satisfy er(e)
     *          transform these sets from DNF to CNF -> explosion of number of clauses
     *          if I have n FCPNs I will have k possible instances of the event
     *                  1b) expand CNF clause for each of n FCPN: if I have (r1 v r2) and 2 FCPNs it will become (r11 v r12 v r21 v r22)
     * 2) FCPN constraint for each FCPN
     * 3) complete PN structure:
     *          given a sequence r1 -> a -> r2 we have the clause with the bound (r1 and r2 => a) that is (!r1 v !r2 v a), for each FCON
     * 4) at least one occurrence of each event in all FCPNs
     * 5) solve SAT
     * todo: 6) decode results
     *
     */

    //TODO: still bugged: SAT with 3 FCPNs whene exists a solution with 2

    auto er_satisfiable_sets = new map<int, set<set<Region *>*>*>();
    vector<set<Region *>*> *vector_of_candidates;
    auto checked_sets = new set<set<Region*>*>();

    //per trovare tutte le combinazioni utili bisogna creare una coda, all'inizio ne fanno parte tutte le regioni singole,
    // ogni volta che una regione viene analizzata questa viene rimossa se completamente dalla coda se da sola soddisfa ec ed inoltre
    // viene aggiunta all'insieme delle soluzioni per tale evento, se da sola non vale, si prendono tutte le combinazioni di tale evento con i successori della coda
    // se queste combinazioni soddisfano vengono salvate mentre se non è cosi si aggiungono a loro volta nella coda

    auto pre_regions_map = pprg->get_pre_regions();
    for(auto rec: *pre_regions_map){
        auto ev = rec.first;
        //cout << "EVENT " << ev << endl;
        vector_of_candidates = new vector<set<Region *>*>();
        for(auto reg: *rec.second){
            auto temp_set = new set<Region *>();
            temp_set->insert(reg);
            vector_of_candidates->push_back(temp_set);
            /*cout << "adding to vector initial: " << endl;
            for(auto r: *temp_set){
                println(*r);
            }*/
        }
        bool exit = false;
        while(!exit){
            auto not_ok_sets = new vector<set<Region *>*>();
            for(auto temp_set: *vector_of_candidates){
                if(checked_sets->find(temp_set)==checked_sets->end()) {
                    if (check_ER_intersection(ev, temp_set, ER)) {
                        if (er_satisfiable_sets->find(ev) == er_satisfiable_sets->end()) {
                            (*er_satisfiable_sets)[ev] = new set<set<Region *> *>();
                        }
                        (*er_satisfiable_sets)[ev]->insert(temp_set);
                        /*vector_of_candidates->erase(vector_of_candidates->begin());
                        if(vector_of_candidates->empty())
                            exit = true;*/
                    } else {
                        auto temp_set_copy = new set<Region *>();
                        for (auto reg: *temp_set) {
                            temp_set_copy->insert(reg);
                        }
                        not_ok_sets->push_back(temp_set_copy);
                        /*vector_of_candidates->pop_back();
                        int vector_length = vector_of_candidates->size();
                        for(int i = 1; i< vector_length-1;i++){
                            vector_of_candidates->push_back(regions_set_union(temp_set, vector_of_candidates->at(i)));
                        }
                        vector_of_candidates->erase(vector_of_candidates->begin());*/
                    }
                    checked_sets->insert(temp_set);
                }
                else{
                    cout << "insieme già visto" << endl;
                }
            }
            vector_of_candidates->clear();
            for(int i=0; i< not_ok_sets->size();i++){
                for(int k=i+1; k< not_ok_sets->size();k++){
                    if(!not_ok_sets->at(i)->empty() && !not_ok_sets->at(k)->empty()) {
                        auto reg_union = regions_set_union(not_ok_sets->at(i), not_ok_sets->at(k));
                        if(checked_sets->find(reg_union)==checked_sets->end()) {
                            /*cout << "adding to vector: " << endl;
                            for (auto r: *reg_union) {
                                println(*r);
                            }*/
                            vector_of_candidates->push_back(reg_union);
                        }
                        else{
                            cout << "insieme unione già visto" << endl;
                        }
                    }
                }
            }
            if(vector_of_candidates->empty())
                exit = true;
            for(auto temp: *not_ok_sets){
                delete temp;
            }
            delete not_ok_sets;
        }
        delete vector_of_candidates;
    }

    number_of_FCPNs = 1;
    bool solution_found = false;


    //EC clause sets creation (transformation from DNF to CNF): this initial set will be further modified depending on
    // the number of FCPNs but the generic form is calculated only once
    //this maps binds events to sets of regions which satisfies EC for this specific event
    auto cnf_ec_map = new map<int, set<set<Region *>*>*>();

    for(auto rec: *er_satisfiable_sets){
        auto ev = rec.first;
        if(cnf_ec_map->find(ev)==cnf_ec_map->end()){
            (*cnf_ec_map)[ev]=new set<set<Region*>*>();
        }
        auto current_set = (*cnf_ec_map)[ev];
        if(rec.second->size()> 1) {
            //todo: qui si genereranno memory leak dovuti al mescolamento tra le 2 mappe
            for (auto reg_set: *rec.second) {
                current_set->insert(reg_set);
            }
            while (current_set->size() > 1) {
                for (auto set1: *current_set) {
                    for (auto set2: *current_set) {
                        if (set1 != set2) {
                            auto set3 = regions_set_union(set1, set2);
                            current_set->erase(set1);
                            current_set->erase(set2);
                            current_set->insert(set3);
                        }
                    }
                }
            }
        }
        //single and clause which have to be only  split
        else{
            for (auto reg_set: *rec.second) {
                for(auto reg: *reg_set){
                    auto new_set = new set<Region*>();
                    new_set->insert(reg);
                    current_set->insert(new_set);
                }
            }
        }
    }

    cout << "ER SATISFIABLE SETS" <<endl;
    for(auto rec: *er_satisfiable_sets){
        cout << "EVENT: " << rec.first << endl;
        auto set_of_sets_of_regions = rec.second;
        for(auto reg_set: *rec.second){
            for(auto reg: *reg_set){
                println(*reg);
            }
            cout << endl;
        }
    }

    cout << "CNF EC MAP" << endl;
    for(auto rec: *cnf_ec_map){
        cout << "EVENT: " << rec.first << endl;
        for(auto reg: *rec.second){
            println(*reg);
        }
    }

    auto clauses = new vector<vector<int32_t> *>();
    vector<int32_t> *clause;

    //encoding: [1, k*number_of_FCPNs+1] regions range: k regions
    //encoding: [k*number_of_FCPNs+2, k*number_of_FCPNs+m*number_of_FCPNs+2] events range: m events

    reg_map = new map<Region*, int>();
    inverse_reg_map = new map<int, Region*>();
    //auto regions_vector = new vector<Region*>();
    int temp = 0;
    for(auto reg: *regions){
        (*reg_map)[reg] = temp;
        (*inverse_reg_map)[temp] = reg;
        //regions_vector->push_back(reg);
        temp++;
    }

    //preparation for STEP 2 -> inverse map respect to pre_regions map i.e. map of outgoing events from a region
    auto region_ex_event_map = new map<Region *, set<int>*>();
    for(auto rec: *pprg->get_pre_regions()){
        auto ev = rec.first;
        for(auto reg: *rec.second){
            if(region_ex_event_map->find(reg) == region_ex_event_map->end()){
                (*region_ex_event_map)[reg] = new set<int>();
            }
            (*region_ex_event_map)[reg]->insert(ev);
        }
    }

    vector<vector<int32_t> *> *updatable_clauses;

    while(!solution_found){
        updatable_clauses = new vector<vector<int32_t> *>();
        cout << "STEP 1a" << endl;
        //STEP 1a: EC clauses creation
        for(auto rec: *cnf_ec_map){
            //reg_set have only one set of regions inside inside
            for(auto reg_set: *rec.second){
                clause = new vector<int32_t>();
                for(auto reg: *reg_set){
                    for(int i=1;i<=number_of_FCPNs;i++){
                        /*if(decomposition_debug){
                            cout << "encoding FCPN " << i << " and region ";
                            println(*reg);
                        }*/
                        auto encoded_value = encoded_region(reg,i);
                        clause->push_back(encoded_value);
                        /*auto decoded_value = decoded_region(encoded_value);
                        if(decomposition_debug){
                            cout << "decoded FCPN " << decoded_value.first << " and region ";
                            println(*decoded_value.second);
                        }*/
                    }
                }
                updatable_clauses->push_back(clause);
                print_clause(clause);
            }
        }

        //STEP 2
        cout << "STEP 2" << endl;
        /*
         * ALGORITMO:
         *      per ogni ev
         *          per ogni r=pre(ev) -> posto/regione
         *              se r ha più di un evento in uscita
         *                  per ogni coppia (r, pre(ev))
         *                      se r != pre(ev)
         *                          per ogni FCPN k
         *                              crea clausola (!r_k v !pre(ev_k))
         */

        for(auto rec: *pre_regions_map){
            //auto ev = rec.first;
            auto set_of_regions = rec.second;
            for (auto r: *set_of_regions) {
                if ((*region_ex_event_map)[r]->size() > 1) {
                    for (auto r2: *set_of_regions) {
                        if (r != r2) {
                            for(int i=1;i<=number_of_FCPNs;i++){
                                clause = new vector<int32_t>();
                                if(decomposition_debug){
                                    cout << "encoding FCPN " << i << " and region ";
                                    println(*r);
                                    cout << "encoding FCPN " << i << " and region ";
                                    println(*r2);
                                }
                                clause->push_back(-encoded_region(r,i));
                                clause->push_back(-encoded_region(r2,i));
                                updatable_clauses->push_back(clause);
                                print_clause(clause);
                                if(decomposition_debug){
                                    auto decoded_value1 = decoded_region(-encoded_region(r,i));
                                    auto decoded_value2 = decoded_region(-encoded_region(r2,i));
                                    cout << "decoded FCPN " << decoded_value1.first << " and region ";
                                    println(*decoded_value1.second);
                                    cout << "decoded FCPN " << decoded_value2.first << " and region ";
                                    println(*decoded_value2.second);
                                }
                            }
                        }
                    }
                }
            }
        }

        //STEP 3
        cout << "STEP 3" << endl;
        auto regions_connected_to_labels = merge_2_maps(pprg->get_pre_regions(),
                                                        pprg->get_post_regions());

        //conversion into clauses
        for (auto rec: *regions_connected_to_labels) {
            auto ev = rec.first;
            for(int i=1;i<=number_of_FCPNs;i++){
                auto ev_encoding = encoded_event(ev,i);
                clause = new vector<int32_t>();
                clause->push_back(ev_encoding);
                if(decomposition_debug){
                    cout << "adding event " << ev << " of FCPN " << i << " encoded as " << ev_encoding << endl;
                    auto ev_decoding = decoded_event(ev_encoding);
                    cout << "decoded event " << ev_decoding.second << " of FCPN " << ev_decoding.first << endl;
                }
                for (auto reg: *rec.second) {
                    int region_encoding = encoded_region(reg, i);
                    if(decomposition_debug){
                        cout << "encoding FCPN " << i << " and region ";
                        println(*reg);
                    }
                    clause->push_back(-region_encoding);
                }
                updatable_clauses->push_back(clause);
                print_clause(clause);
            }
        }
        for(auto rec: *regions_connected_to_labels){
            delete rec.second;
        }
        delete regions_connected_to_labels;

        //STEP 4
        cout << "STEP 4" << endl;
        for(int i=0;i<num_events;i++){
            clause = new vector<int32_t>();
            for(int k=1;k<=number_of_FCPNs;k++){
                int encoded_ev = encoded_event(i, k);
                clause->push_back(encoded_ev);
            }
            updatable_clauses->push_back(clause);
        }


        //STEP 5
        cout << "STEP 5" << endl;
        PBConfig config = make_shared<PBConfigClass>();
        VectorClauseDatabase formula(config);
        PB2CNF pb2cnf(config);
        int first_aux_var = number_of_FCPNs*number_of_regions+number_of_FCPNs*number_of_events-2*number_of_FCPNs+3;
        if(decomposition_debug)
            cout << "first aux var: " << first_aux_var << endl;
        AuxVarManager auxvars(first_aux_var);
        for (auto cl: *clauses) {
            formula.addClause(*cl);
        }
        for (auto cl: *updatable_clauses) {
            formula.addClause(*cl);
        }
        Minisat::Solver solver;

        int num_clauses_formula = formula.getClauses().size();
        string dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                        formula.getClauses());
        bool sat = check_sat_formula_from_dimacs(solver, dimacs_file);
        if (sat) {
            solution_found = true;
            if (decomposition_debug) {
                cout << "SAT with " << number_of_FCPNs << " FCPNs" << endl;
                cout << "Model: ";
            }
            //last_solution->clear();
            for (int i = 0; i < solver.nVars(); ++i) {
                if (solver.model[i] != l_Undef) {
                    if (decomposition_debug) {
                        fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                i + 1);
                    }
                    /*if(i < number_of_regions) {
                        if (solver.model[i] == l_True) {
                            last_solution->insert(i + 1);
                        } else {
                            last_solution->insert(-i - 1);
                        }
                    }*/
                }
            }
            if(decomposition_debug)
                cout << endl;
        } else {
            if (decomposition_debug) {
                //cout << "----------" << endl;
                cout << "UNSAT with " << number_of_FCPNs << " FCPNs" << endl;
                /*if(exists_solution) {
                    cout << "Model: ";
                    for (int i = 0; i < solver.nVars(); ++i) {
                        if (solver.model[i] != l_Undef) {
                            fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                    i + 1);
                        }
                    }
                    cout << endl;
                }*/
            }
        }

        /*if(!solution_found){
            if(decomposition_debug)
                cout << "no solution with " << number_of_FCPNs << " FCPNs" << endl;
            number_of_FCPNs++;
        }*/
        if(solution_found){
            //STEP 6 TRY for debug
            auto temp_FCPN = new set<Region *>();
            for (int i = 0; i < number_of_regions; ++i) {
                if (solver.model[i] != l_Undef) {
                    if(solver.model[i] == l_True){
                        auto dec = decoded_region(i+1);
                        //println(*dec.second);
                        temp_FCPN->insert(dec.second);
                    }
                }
            }

            auto fcpn_set = new set<set<Region *>*>();
            fcpn_set->insert(temp_FCPN);
            //auto pre_regions_map = pprg->get_pre_regions();
            //create map (region, exiting events)

            string output_name = file;
            while (output_name[output_name.size() - 1] != '.') {
                output_name = output_name.substr(0, output_name.size() - 1);
            }
            output_name = output_name.substr(0, output_name.size() - 1);
            output_name += "_TEMP";

            cout << "=======================[ CREATION OF PRE/POST-REGIONS FOR EACH PN ]================" << endl;

            //map with key the pointer to SM
            auto map_of_PN_pre_regions = new map < SM *, map<int, set<Region *> *>*> ();
            auto map_of_PN_post_regions = new map < SM *, map<int, set<Region *> *>*> ();

            for (auto pn: *fcpn_set) {
                (*map_of_PN_pre_regions)[pn] = new map<int, set<Region *> *> ();
                for (auto rec: *pprg->get_pre_regions()) {
                    for (auto reg: *rec.second) {
                        if (pn->find(reg) != pn->end()) {
                            if((*map_of_PN_pre_regions)[pn]->find(rec.first) == (*map_of_PN_pre_regions)[pn]->end()){
                                (*(*map_of_PN_pre_regions)[pn])[rec.first] = new set<Region *>();
                            }
                            (*(*map_of_PN_pre_regions)[pn])[rec.first]->insert(reg);
                        }
                    }
                }

                (*map_of_PN_post_regions)[pn] = pprg->create_post_regions_for_FCPN((*map_of_PN_pre_regions)[pn]);
            }
            int pn_counter=0;
            auto regions_mapping = get_regions_map(pprg->get_pre_regions());
            for(auto pn: *fcpn_set){
                print_fcpn_dot_file(regions_mapping,map_of_PN_pre_regions->at(pn), map_of_PN_post_regions->at(pn), aliases, file,pn_counter);
                pn_counter++;
            }

            delete regions_mapping;

            if (decomposition_debug) {
                cout << "Final FCPNs" << endl;
                for (auto SM: *fcpn_set) {
                    cout << "FCPN:" << endl;
                    println(*SM);
                }
            }

            for(auto rec: *map_of_PN_pre_regions){
                for(auto subset: *rec.second){
                    delete subset.second;
                }
                delete rec.second;
            }
            delete map_of_PN_pre_regions;
            for(auto rec: *map_of_PN_post_regions){
                for(auto subset: *rec.second){
                    delete subset.second;
                }
                delete rec.second;
            }
            delete map_of_PN_post_regions;

            for(auto pn: *fcpn_set){
                delete pn;
            }
            delete fcpn_set;
        }
        //exit(1);
        number_of_FCPNs++;
        for(auto cl: *updatable_clauses){
            delete cl;
        }
        delete updatable_clauses;
    }

    //STEP 6
    cout << "STEP 6" << endl;

    for(auto rec: *er_satisfiable_sets){
        delete rec.second;
    }
    delete er_satisfiable_sets;

    for(auto val: *checked_sets){
        delete val;
    }
    delete checked_sets;

    exit(1);


    for(auto rec: *region_ex_event_map){
        delete rec.second;
    }
    delete region_ex_event_map;



    for(auto cl: *clauses){
        delete cl;
    }
    delete clauses;
}

k_FCPN_decomposition::~k_FCPN_decomposition() {
    delete reg_map;
    delete inverse_reg_map;
};

//suppose number_of_FCPNs = n
//encoding: [1, n*(k-1)+1=n*k-n+1] regions range: k regions
//reg_map decoded values: [0,k-1]
//FCPN_number: [1, n]
int k_FCPN_decomposition::encoded_region(Region *reg, int current_FCPN_number) const{
    if(current_FCPN_number < 1) {
        cerr << "FCPN number have to be greater than 0" << endl;
        exit(1);
    }
    return 1+reg_map->at(reg)*current_FCPN_number;
}

//returns the number of FCPN and the pointer to the region
pair<int, Region *> k_FCPN_decomposition::decoded_region(int encoded_value) const{
    if(encoded_value < 0)
        encoded_value*=-1;
    int fcpn_counter = 0;
    int temp_value = encoded_value-1;
    while(temp_value>=number_of_regions){
        temp_value -= number_of_regions;
        fcpn_counter++;
    }
    return make_pair(fcpn_counter+1, inverse_reg_map->at(temp_value));
}

//suppose number_of_FCPNs = n
//number of regions k
//event values: [0,m-1] having m different events
//FCPN_number: [1, n]
//encoding for event i in FCPN j: n*k-k+2+i*j
//events range: [n*k-n+2, n*(k-1)+2+(m-1)*n = n*k-n+2+n*m-n = n*k+n*m-2*n+2]
int k_FCPN_decomposition::encoded_event(int event, int current_FCPN_number) const {
    return number_of_FCPNs*number_of_regions-number_of_FCPNs+2+event*current_FCPN_number;
}

//returns the number of FCPN and the event number
pair<int, int> k_FCPN_decomposition::decoded_event(int encoded_value) const{
    if(encoded_value < 0)
        encoded_value*=-1;
    encoded_value -= number_of_FCPNs*number_of_regions-number_of_FCPNs+2;
    int fcpn_counter = 0;
    int temp_value = encoded_value-1;
    while(temp_value>number_of_events){
        temp_value -= number_of_events;
        fcpn_counter++;
    }
    return make_pair(fcpn_counter+1, temp_value+1);
}