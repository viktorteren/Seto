/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/k_FCPN_decomposition_blind.h"

using namespace PBLib;
using namespace Minisat;
using namespace Utilities;


k_FCPN_decomposition_blind::k_FCPN_decomposition_blind(int number_of_ev,
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

    //EC encoding for simultaneous k-FCPN decomposition check
    /*
     * 1) FCPN constraint for each FCPN
     * 2) complete PN structure:
     *          given a sequence r1 -> a -> r2 we have the clause with the bound (r1 and r2 => a) that is (!r1 v !r2 v a),
     *          for each FCPN
     * 3) at least one occurrence of each event in all FCPNs (not each FCPN!!!)
     * 4) at least one region which covers each state: for each covered state by r1, r2, r3 create a clause (r1 v r2 v r3) for each FCPN
     * do
     *      solve SAT
     *      if(SAT)
     *          if(!EC)
     *              add constraint
     *      if(UNSAT)
     *          expand number of FCPNs
     * while(!EC)
     * 5) minimize the resultant FCPNs: min(number of regions) knowing the number of FCPNs -> solve SAT with binary search usage
     * 6) decode results
     */

    auto er_satisfiable_sets = new map<int, set<set<Region *>*>*>();
    vector<set<Region *>*> *vector_of_candidates;
    set<set<Region*>> *checked_sets;
    auto fcpn_set = new set<set<Region *>*>();
    auto pre_regions_map = pprg->get_pre_regions();

    //per trovare tutte le combinazioni utili bisogna creare una coda, all'inizio ne fanno parte tutte le regioni singole,
    // ogni volta che una regione viene analizzata questa viene rimossa completamente dalla coda se da sola soddisfa ec ed inoltre
    // viene aggiunta all'insieme delle soluzioni per tale evento, se da sola non vale, si prendono tutte le combinazioni di tale evento con i successori della coda
    // se queste combinazioni soddisfano vengono salvate mentre se non è cosi si aggiungono a loro volta nella coda



    number_of_FCPNs = 1;
    bool solution_found = false;


    auto clauses = new vector<vector<int32_t> *>();
    vector<int32_t> *clause;
    set<int32_t> *lit_set;

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

    //preparation for STEP 1 -> inverse map respect to pre_regions map i.e. map of outgoing events from a region
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

    auto regions_in_conflict = new set<pair<Region *, Region *>>();
    for(auto rec: *pre_regions_map) {
        //auto ev = rec.first;
        auto set_of_regions = rec.second;
        for (auto r: *set_of_regions) {
            if ((*region_ex_event_map)[r]->size() > 1) {
                for (auto r2: *set_of_regions) {
                    if (r != r2) {
                        regions_in_conflict->insert(make_pair(r,r2));
                    }
                }
            }
        }
    }

    //cout << "regions in conflict size: " << regions_in_conflict->size() << endl;

    //preparation for STEP 4
    auto state_regions_map = new map<int, set<Region*>*>();
    for(Region *reg: *regions) {
        for (auto st: *reg) {
            if (state_regions_map->find(st) == state_regions_map->end()) {
                (*state_regions_map)[st] = new set<Region *>();
            }
            state_regions_map->at(st)->insert(reg);
        }
    }

    set<set<int32_t> *> *updatable_clauses = nullptr;

    int regions_in_solution = 0;

    while(!solution_found){
        if(updatable_clauses != nullptr){
            for(auto cl: *updatable_clauses){
                delete cl;
            }
            delete updatable_clauses;
        }
        updatable_clauses = new set<set<int32_t> *>();

        //STEP 1
        if(decomposition_debug)
            cout << "STEP 1" << endl;
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

        for(auto pair: *regions_in_conflict){
            auto r = pair.first;
            auto r2 = pair.second;
            for(int i=1;i<=number_of_FCPNs;++i){
                //clause = new vector<int32_t>();
                lit_set = new set<int32_t>();
                /*if(decomposition_debug){
                    cout << "encoding FCPN " << i << " and region ";
                    println(*r);
                    cout << "encoding FCPN " << i << " and region ";
                    println(*r2);
                }*/
                //clause->push_back(-encoded_region(r,i));
                //clause->push_back(-encoded_region(r2,i));
                lit_set->insert(-encoded_region(r,i));
                lit_set->insert(-encoded_region(r2,i));
                bool found = false;
                for(auto clau: *updatable_clauses){
                    if(*clau == *lit_set){
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    updatable_clauses->insert(lit_set);
                    //print_clause(lit_set);
                }
                else{
                    //cout << "clausola già presente" << endl;
                    delete lit_set;
                }
                //delete clause;
                /*if(decomposition_debug){
                    auto decoded_value1 = decoded_region(-encoded_region(r,i));
                    auto decoded_value2 = decoded_region(-encoded_region(r2,i));
                    cout << "decoded FCPN " << decoded_value1.first << " and region ";
                    println(*decoded_value1.second);
                    cout << "decoded FCPN " << decoded_value2.first << " and region ";
                    println(*decoded_value2.second);
                }*/
            }
        }

        //STEP 2
        if(decomposition_debug)
            cout << "STEP 2" << endl;
        auto regions_connected_to_labels = merge_2_maps(pprg->get_pre_regions(),
                                                        pprg->get_post_regions());

        //conversion into clauses
        for (auto rec: *regions_connected_to_labels) {
            auto ev = rec.first;
            for(int i=1;i<=number_of_FCPNs;++i){
                auto ev_encoding = encoded_event(ev,i);
                //clause = new vector<int32_t>();
                //clause->push_back(ev_encoding);
                lit_set = new set<int32_t>();
                lit_set->insert(ev_encoding);
                /*if(decomposition_debug){
                    cout << "adding event " << ev << " of FCPN " << i << " encoded as " << ev_encoding << endl;
                    auto ev_decoding = decoded_event(ev_encoding);
                    cout << "decoded event " << ev_decoding.second << " of FCPN " << ev_decoding.first << endl;
                }*/
                for (auto reg: *rec.second) {
                    int region_encoding = encoded_region(reg, i);
                    /*if(decomposition_debug){
                        cout << "encoding FCPN " << i << " and region ";
                        println(*reg);
                    }*/
                    //clause->push_back(-region_encoding);
                    lit_set->insert(-region_encoding);
                }
                updatable_clauses->insert(lit_set);
                //print_clause(lit_set);
                //delete clause;
            }
        }
        for(auto rec: *regions_connected_to_labels){
            delete rec.second;
        }
        delete regions_connected_to_labels;

        //STEP 3
        if(decomposition_debug)
            cout << "STEP 3" << endl;
        for(int i=0;i<num_events;++i){
            //clause = new vector<int32_t>();
            lit_set = new set<int32_t>();
            for(int k=1;k<=number_of_FCPNs;++k){
                int encoded_ev = encoded_event(i, k);
                //clause->push_back(encoded_ev);
                lit_set->insert(encoded_ev);
            }
            updatable_clauses->insert(lit_set);
            //delete clause;
        }

        //STEP 4
        if(decomposition_debug)
            cout << "STEP 4" << endl;
        for(auto rec: *state_regions_map){
            for(int i=1;i<=number_of_FCPNs;++i) {
                auto region_set = rec.second;
                clause = new vector<int32_t>();
                for (auto reg: *region_set) {
                    clause->push_back(encoded_region(reg,i));
                }
                clauses->push_back(clause);
            }
            //print_clause(clause);
        }

        bool excitation_closure = false;
        auto results_to_avoid = new set<set<int>*>();
        bool unsat = false;
        do{
            //cout  << "in cicle" << endl;
            if(decomposition_debug)
                cout << "STEP 6" << endl;
            PBConfig config = make_shared<PBConfigClass>();
            VectorClauseDatabase formula(config);
            PB2CNF pb2cnf(config);
            int first_aux_var = number_of_FCPNs*number_of_regions+number_of_FCPNs*number_of_events-2*number_of_FCPNs+3;
            /*if(decomposition_debug)
                cout << "first aux var: " << first_aux_var << endl;*/
            AuxVarManager auxvars(first_aux_var);
            for (auto cl: *clauses) {
                formula.addClause(*cl);
            }
            auto new_vector = new set<vector<int32_t>*>();
            for(auto cl_set: *updatable_clauses){
                new_vector->insert(new vector<int32_t>(cl_set->begin(), cl_set->end()));
            }
            for (auto cl: *new_vector) {
                formula.addClause(*cl);
            }
            for(auto elem: *new_vector){
                delete elem;
            }
            delete new_vector;
            Minisat::Solver solver;

            int num_clauses_formula = formula.getClauses().size();
            string dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                                   formula.getClauses(), results_to_avoid);
            bool sat = check_sat_formula_from_dimacs(solver, dimacs_file);
            if (sat) {
                auto last_solution = new set<int>();
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
                        if(i < number_of_regions*number_of_FCPNs) {
                            if (solver.model[i] == l_True) {
                                last_solution->insert(i + 1);
                            } else {
                                last_solution->insert(-i - 1);
                            }
                        }
                    }
                }
                if(decomposition_debug)
                    cout << endl;
                fcpn_set->clear();
                //creation of temporal FCPNs
                for(int k=1;k<=number_of_FCPNs;++k) {
                    auto temp_FCPN = new set<Region *>();
                    for (int i = 0; i < number_of_regions; ++i) {
                        int encoded_value = encoded_region(inverse_reg_map->at(i), k);
                        if (last_solution->find(encoded_value) != last_solution->end()) {
                            //cout << "encoded value found " << encoded_value << endl;
                            //cout << "to decode: " << encoded_value << endl;
                            auto dec = decoded_region(encoded_value);
                            //println(*dec.second);
                            temp_FCPN->insert(dec.second);
                        }
                    }
                    fcpn_set->insert(temp_FCPN);
                }
                auto used_regions_map = get_map_of_used_regions(fcpn_set, pprg->get_pre_regions());
                excitation_closure = is_excitation_closed(used_regions_map, ER);
                if(!excitation_closure){
                    for(auto FCPN: *fcpn_set){
                        delete FCPN;
                    }
                    results_to_avoid->insert(last_solution);
                    /*cout << "to avoid ";
                    print(*last_solution);
                    cout << endl;*/
                }
                else{
                    cout << "solution found" << endl;
                    solution_found = true;
                    regions_in_solution = 0;
                    //set<Region *> *temp_FCPN;
                    for(int k=1;k<=number_of_FCPNs;++k) {
                        //temp_FCPN = new set<Region *>();
                        for (int i = 0; i < number_of_regions; ++i) {
                            int encoded_value = encoded_region(inverse_reg_map->at(i), k);
                            if (solver.model[encoded_value-1] != l_Undef) {
                                if (solver.model[encoded_value-1] == l_True) {
                                    //cout << "to decode: " << encoded_value << endl;
                                    //auto dec = decoded_region(encoded_value);
                                    //println(*dec.second);
                                    //temp_FCPN->insert(dec.second);
                                    regions_in_solution++;
                                    /*if(decomposition_debug) {
                                        cout << "k = " << k << endl;
                                        cout << "decoded PN: " << dec.first << endl;
                                    }*/
                                    /*if(k != dec.first){
                                        cerr << "wrong FCPN decoding" << endl;
                                        exit(1);
                                    }*/
                                }
                            }
                        }
                        //fcpn_set->insert(temp_FCPN);
                    }
                }
            } else {
                if (decomposition_debug) {
                    //cout << "----------" << endl;
                    cout << "UNSAT with " << number_of_FCPNs << " FCPNs" << endl;
                }
                unsat = true;
            }
        }while(!excitation_closure && !unsat);






        /*if(!solution_found){
            if(decomposition_debug)
                cout << "no solution with " << number_of_FCPNs << " FCPNs" << endl;
            number_of_FCPNs++;
        }*/
        number_of_FCPNs++;
        cout << "try FCPNs: " << number_of_FCPNs << endl;

        /*for(auto cl: *updatable_clauses){
            delete cl;
        }
        delete updatable_clauses;*/
    }
    number_of_FCPNs--;

    delete regions_in_conflict;

    //STEP 7
    if(decomposition_debug)
        cout << "STEP 7" << endl;
    int max = regions_in_solution;
    int min = 1;
    int current_max_regions = max; //max - 1 could produce better results but with also max I am sure that the firs result is SAT

    //sum of regions encoding
    vector<WeightedLit> sum_of_regions = {};
    sum_of_regions.reserve(number_of_FCPNs*number_of_regions);
    for(int k=1;k<=number_of_FCPNs*number_of_regions;++k){
        sum_of_regions.emplace_back(k,1);
    }

    Minisat::Solver *solver2;
    PBConfig config2 = make_shared<PBConfigClass>();
    VectorClauseDatabase formula2(config2);
    PB2CNF pb2cnf(config2);
    int first_aux_var = number_of_FCPNs*number_of_regions+number_of_FCPNs*number_of_events-2*number_of_FCPNs+3;
    AuxVarManager auxvars2(first_aux_var);

    auto last_solution = new set<int>();

    do {
        solver2 = new Minisat::Solver();
        auxvars2.resetAuxVarsTo(first_aux_var);
        formula2.clearDatabase();
        for (auto cl: *clauses) {
            formula2.addClause(*cl);
        }
        auto new_vector = new set<vector<int32_t>*>();
        for(auto cl_set: *updatable_clauses){
            new_vector->insert(new vector<int32_t>(cl_set->begin(), cl_set->end()));
        }
        for (auto cl: *new_vector) {
            formula2.addClause(*cl);
        }
        for(auto elem: *new_vector){
            delete elem;
        }
        delete new_vector;
        PBConstraint constraint2(sum_of_regions, LEQ,
                                 current_max_regions); //the sum have to be lesser or equal to current_value2
        pb2cnf.encode(constraint2, formula2, auxvars2);

        int num_clauses_formula = formula2.getClauses().size();

        if(decomposition_debug)
            cout<< "Formula size: " << formula2.getClauses().size() << endl;

        string dimacs_file = convert_to_dimacs(file, auxvars2.getBiggestReturnedAuxVar(), num_clauses_formula,
                                               formula2.getClauses());
        bool sat = check_sat_formula_from_dimacs(*solver2, dimacs_file);

        if (sat) {
            max = current_max_regions;
            if (decomposition_debug) {
                cout << "SAT with " << number_of_FCPNs << " FCPNs and " << current_max_regions << " regions" << endl;
                cout << "Model: ";
            }
            last_solution->clear();
            for (int i = 0; i < solver2->nVars(); ++i) {
                if (solver2->model[i] != l_Undef) {
                    if (decomposition_debug) {
                        fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver2->model[i] == l_True) ? "" : "-",
                                i + 1);
                    }
                    if(i < number_of_regions*number_of_FCPNs) {
                        if (solver2->model[i] == l_True) {
                            last_solution->insert(i + 1);
                        } else {
                            last_solution->insert(-i - 1);
                        }
                    }
                }
            }
            if(decomposition_debug)
                cout << endl;
        } else {
            min = current_max_regions;
            if (decomposition_debug) {
                //cout << "----------" << endl;
                cout << "UNSAT with " << number_of_FCPNs << " FCPNs and " << current_max_regions << " regions" << endl;
            }
        }
        //cout << "min: " << min << endl;
        //cout << "max: " << max << endl;
        current_max_regions = (min + max) / 2;
        delete solver2;
    } while((max - min) > 1);

    //STEP 8
    if(decomposition_debug)
        cout << "STEP 8" << endl;
    set<Region *> *temp_FCPN;
    for(int k=1;k<=number_of_FCPNs;++k) {
        temp_FCPN = new set<Region *>();
        for (int i = 0; i < number_of_regions; ++i) {
            int encoded_value = encoded_region(inverse_reg_map->at(i), k);
            if (last_solution->find(encoded_value) != last_solution->end()) {
                //cout << "encoded value found " << encoded_value << endl;
                //cout << "to decode: " << encoded_value << endl;
                auto dec = decoded_region(encoded_value);
                //println(*dec.second);
                temp_FCPN->insert(dec.second);
            }
        }
        fcpn_set->insert(temp_FCPN);
    }
    delete last_solution;

    const string& output_name = file;
    /*while (output_name[output_name.size() - 1] != '.') {
        output_name = output_name.substr(0, output_name.size() - 1);
    }
    output_name = output_name.substr(0, output_name.size() - 1);
    output_name += "_TEMP.g";*/

    if(decomposition_debug)
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
        print_fcpn_dot_file(regions_mapping,map_of_PN_pre_regions->at(pn), map_of_PN_post_regions->at(pn), aliases, output_name,pn_counter);
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

    cout << "Number of FCPNs: " << fcpn_set->size() << endl;

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


    for(auto rec: *er_satisfiable_sets){
        delete rec.second;
    }
    delete er_satisfiable_sets;

    for(auto rec: *state_regions_map){
        delete rec.second;
    }
    delete state_regions_map;

    for(auto rec: *region_ex_event_map){
        delete rec.second;
    }
    delete region_ex_event_map;

    for(auto cl: *clauses){
        delete cl;
    }
    delete clauses;
    for(auto cl: *updatable_clauses){
        delete cl;
    }
    delete updatable_clauses;

    //delete set_of_initial_regions;
}

k_FCPN_decomposition_blind::~k_FCPN_decomposition_blind() {
    delete reg_map;
    delete inverse_reg_map;
}

//number of regions: k
//number fo FCPNS: n
//current_FCPN: [1, n]
//current_region: [0, k-1]
//encoding: k*(current_FCPN-1)+current_region+1
//encoding range: [1, k*(n-1)+k+1]
int k_FCPN_decomposition_blind::encoded_region(Region *reg, int current_FCPN_number) const{
    if(current_FCPN_number < 1) {
        cerr << "FCPN number have to be greater than 0" << endl;
        exit(1);
    }
    return (number_of_regions*(current_FCPN_number-1))+reg_map->at(reg) + 1;
    //return (1+reg_map->at(reg))*current_FCPN_number;
}

//returns the number of FCPN and the pointer to the region
pair<int, Region *> k_FCPN_decomposition_blind::decoded_region(int encoded_value) const{
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
int k_FCPN_decomposition_blind::encoded_event(int event, int current_FCPN_number) const {
    return number_of_FCPNs*number_of_regions-number_of_FCPNs+2+event*current_FCPN_number;
}

//returns the number of FCPN and the event number
pair<int, int> k_FCPN_decomposition_blind::decoded_event(int encoded_value) const{
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