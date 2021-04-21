/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/FCPN_decomposition.h"

using namespace PBLib;
using namespace Minisat;
using namespace Utilities;


FCPN_decomposition::FCPN_decomposition(int number_of_events,
                                        vector<Region> *regions,
                                        string file,
                                        Pre_and_post_regions_generator *pprg,
                                        map<int, int> *aliases,
                                        map<int, ER> *ER){
    auto clauses = new vector<vector<int32_t> *>();
    cout << "=========[FCPN DECOMPOSITION MODULE]===============" << endl;
    auto fcpn_set = new set<set<Region *>*>();
    auto not_used_regions = new set<Region *>();
    auto pre_regions_map = pprg->get_pre_regions();
    //create map (region, exiting events)
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

    auto results_to_avoid = new set<set<int>*>();
    auto reg_map = new map<Region*, int>();
    auto regions_vector = new vector<Region*>();
    for(int k=0;k<regions->size();k++){
        (*reg_map)[&(regions->at(k))] = k;
        regions_vector->push_back(&(regions->at(k)));
        not_used_regions->insert(&(regions->at(k)));
    }

    vector<int32_t> *clause;
    // todo: completare l'algoritmo completo
    /* Possible algorithm for the creation of one FCPN with SAT:
     * ALGORITHM STEPS:
     * 1) almeno una regione che copre lo stato: per ogni stato coperto da r1, r2, r3 creare una clausola (r1 v r2 v r3)
     * 2) vincolo FCPN -> date le regioni presenti in una PN queste non devono violare il vincolo
     *  ALGORITMO:
     *      per ogni ev
     *          per ogni r=pre(ev) -> posto/regione
     *              se r ha più di un evento in uscita
     *                  per ogni coppia (r, pre(ev))
     *                      se r != pre(ev)
     *                          crea clausola (!r v !pre(ev))
     *
     * 3) struttura PN completa dalla quale togliere:
     *      se ho una sequenza r1 -> a -> r2 avrò le clausole relative a r1 and r2 => a cioè (!r1 v !r2 v a)
     * 4) funzione da massimizzare: num regioni utilizzate nel risultato
     * todo: 5) minimizzare il numero di regioni totali dopo la prima esecuzione o forse anche fin da subito,
     * ancora meglio trovare il valore sat per masimizzare la copertura e poi con quel valore minimizzare il numero di regioni
     * ma se si combinano le 2 cose si trova il risultato migliore prima
     * todo: 6) solve the SAT problem decreasing the value of the region sum -> starting value is the sum of all regions
     * todo: 7) decoding risultato
     * todo: 8) vincolo EC
     *      si potrebbe usare il metodo is excitation closed, se ritorna falso con un risultato del sat solver
     *      aggiungere la clausola che neghi quel risultato
     *todo: 9) forse serve il vincolo sulla componibilità: se un posto ha n transizioni in uscita tutte devono essere presenti nella stessa PN: duplication avoidance
     */

    //TODO: all steps have to be implemented
    //encoding: [1, n] states range: n states
    //encoding: [n+1, n+k+1] regions range: k regions
    //encoding: [n+k+2, n+k+m+2] events range: m events

    //nuova codifica
    //encoding: [1, k] regions range: k regions
    //encoding: [k+1, k+m+1] events range: m events
    int m = number_of_events;
    int k = regions->size();
    bool excitation_closure = false;

    //STEP 1
    cout << "STEP 1a" << endl;
    auto state_regions_map = new map<int, set<Region*>*>();
    for(Region *reg: *regions_vector){
        for(auto st: *reg){
            if(state_regions_map->find(st) == state_regions_map->end()){
                (*state_regions_map)[st] = new set<Region*>();
            }
            state_regions_map->at(st)->insert(reg);
        }
    }
    do{
        for(auto clause: *clauses){
            delete clause;
        }
        clauses->clear();

        cout << "STEP 1b" << endl;
        for(auto rec: *state_regions_map){
            auto region_set = rec.second;
            clause = new vector<int32_t>();
            for(auto reg: *region_set){
                clause->push_back(reg_map->at(reg)+1);
            }
            clauses->push_back(clause);
            //print_clause(clause);
        }

        //STEP 2 todo: da testare bug probabilmente dovrei avere 5 e 11
        cout << "STEP 2" << endl;
        /*
         * ALGORITMO:
         *      per ogni ev
         *          per ogni r=pre(ev) -> posto/regione
         *              se r ha più di un evento in uscita
         *                  per ogni coppia (r, pre(ev))
         *                      se r != pre(ev)
         *                          crea clausola (!r v !pre(ev))
         */
        //for each ev
        for(auto rec: *pre_regions_map){
            auto ev = rec.first;
            auto set_of_regions = rec.second;
            for (auto r: *set_of_regions) {
                if ((*region_ex_event_map)[r]->size() > 1) {
                    for (auto r2: *set_of_regions) {
                        if (r != r2) {
                            clause = new vector<int32_t>();
                            clause->push_back(-(*reg_map)[r]-1);
                            clause->push_back(-(*reg_map)[r2]-1);
                            clauses->push_back(clause);
                            //print_clause(clause);
                        }
                    }
                }
            }
        }


        //STEP 3 TODO: da testare
        cout << "STEP 3" << endl;
        auto regions_connected_to_labels = merge_2_maps(pprg->get_pre_regions(),
                                                        pprg->get_post_regions());

        //conversion into clauses
        for (auto rec: *regions_connected_to_labels) {
            auto ev = rec.first;
            auto ev_encoding = k+2+ev;
            //(encoded_events_map)[ev_encoding] = make_pair(SM_counter, ev - 1);
            clause = new vector<int32_t>();
            clause->push_back(ev_encoding);
            for (auto reg: *rec.second) {
                int region_encoding = 1+reg_map->at(reg);
                clause->push_back(-region_encoding);
            }
            clauses->push_back(clause);
            //print_clause(clause);
        }
        for(auto rec: *regions_connected_to_labels){
            delete rec.second;
        }
        delete regions_connected_to_labels;

        //STEP 4
        cout << "STEP 4" << endl;
        vector<WeightedLit> literals_from_regions = {};
        literals_from_regions.reserve(k); //improves the speed
        for(int i=0;i<k;i++){
            if(not_used_regions->find((*regions_vector)[i]) != not_used_regions->end()){
                literals_from_regions.emplace_back(1+i, 1);
            }
            else{
                literals_from_regions.emplace_back(1+i, 0);
            }
        }

        //TODO: STEP 5
        cout << "STEP 5" << endl;
        vector<WeightedLit> sum_of_regions = {};
        sum_of_regions.reserve(k);
        for(int i=0;i<k;i++){
            sum_of_regions.emplace_back(1+i, 1);
        }

        //todo mettere il valore a 0 in seguito ???
        int current_value = 1;
        int min = 0;
        int max = k;

        PBConfig config = make_shared<PBConfigClass>();
        VectorClauseDatabase formula(config);
        PB2CNF pb2cnf(config);
        AuxVarManager auxvars(k+m+2);
        for (auto cl: *clauses) {
            formula.addClause(*cl);
        }

        Minisat::Solver solver;


        bool sat = true;
        string dimacs_file;
        bool exists_solution = false;

        auto last_solution = new set<int>();
        //iteration in the search of a correct assignment decreasing the total weight
        do {
            IncPBConstraint constraint(literals_from_regions, GEQ,
                                       current_value); //the sum have to be greater or equal to current_value
            pb2cnf.encodeIncInital(constraint, formula, auxvars);
            int num_clauses_formula = formula.getClauses().size();
            //cout << "formula 1" << endl;
            //formula.printFormula(cout);
            dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                            formula.getClauses(), results_to_avoid);
            sat = check_sat_formula_from_dimacs(solver, dimacs_file);
            if (sat) {
                exists_solution = true;
                if (decomposition_debug) {
                    cout << "SAT with value " << current_value << endl;
                    cout << "Model: ";
                }
                last_solution->clear();
                for (int i = 0; i < solver.nVars(); ++i) {
                    if (solver.model[i] != l_Undef) {
                        if (decomposition_debug) {
                            fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                    i + 1);
                        }
                        if(i < k) {
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
                //maxValueToCheck--;
                min = current_value;
            } else {
                if (decomposition_debug) {
                    //cout << "----------" << endl;
                    cout << "UNSAT with value " << current_value << endl;
                }
                max = current_value;
            }
            current_value = (min + max) / 2;
        } while ((max - min) > 1);


        int current_value2 = 0;
        //TODO qui bug: il primo risultato non può essere unsat
        for(auto val: *last_solution){
            if(val > 0){
                current_value2++;
            }
        }
        int min2 = 0;
        int max2 = current_value2;

        //TODO: this part doesn't works -> forse usare le clausole stanard, probabilmente auxvars in contraddizione tra le 2 parti
        //TODO: anche senza la parte aggiuntiva non viene il risultato sat, perchè?
        cout << "TRYING TO DECREASE THE NUMBER OF REGIONS" << endl;

        int num_clauses_formula = formula.getClauses().size();
        formula.clearDatabase();
        for (auto cl: *clauses) {
            formula.addClause(*cl);
        }
        //cout << "formula" << endl;
        //formula.printFormula(cout);

        Minisat::Solver solver2;

        do{
            IncPBConstraint constraint(literals_from_regions, BOTH,
                                        current_value, current_value); //the sum have to equal to current_value
            pb2cnf.encodeIncInital(constraint, formula, auxvars);
            num_clauses_formula = formula.getClauses().size();
            //cout << "formula size: " << formula.getClauses().size() << endl;
            //cout << "formula 2" << endl;
            //formula.printFormula(cout);
            IncPBConstraint constraint2(sum_of_regions, LEQ,
                                       current_value2); //the sum have to be lesser or equal to current_value2
            //AuxVarManager auxvars2(auxvars.getBiggestReturnedAuxVar()+1);
            pb2cnf.encodeIncInital(constraint2, formula, auxvars);
            //cout << "formula size: " << formula.getClauses().size() << endl;
            /*vector<int32_t> lits;
            for(int i=1;i<=k;i++){
                lits.push_back(i);
            }*/
            /*auto formula_clauses = formula.getClauses();
            cout << "formula  clauses size: " << formula_clauses.size() << endl;
            auto a_caso = pb2cnf.encodeAtMostK(lits, 7, formula_clauses, k+m+2);*/
            num_clauses_formula = formula.getClauses().size();
            //cout << "formula 3" << endl;
            //formula.printFormula(cout);
            //cout << "formula2 size: " << formula.getClauses().size() << endl;
            //cout << "formula  clauses2 size: " << formula_clauses.size() << endl;
            /*for(auto cl: formula_clauses){
                print_clause(&cl);
            }*/
            //num_clauses_formula =formula_clauses.size();
            int max_var = 0;
            /*for(auto cl: formula_clauses){
                for(auto var: cl){
                    if(var > max_var)
                        max_var = var;
                }
            }*/
            //todo: al posto di null pointer dopo rimettere le clausole già usate preceentemente
            dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                            formula.getClauses(), results_to_avoid);
            sat = check_sat_formula_from_dimacs(solver2, dimacs_file);
            if (sat) {
                exists_solution = true;
                if (decomposition_debug) {
                    cout << "(Decreasing) SAT with values " << current_value << ", " << current_value2 << endl;
                    cout << "Model: ";
                }
                last_solution->clear();
                for (int i = 0; i < solver2.nVars(); ++i) {
                    if (solver2.model[i] != l_Undef) {
                        if(decomposition_debug) {
                            fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                    i + 1);
                        }
                        if(i < k) {
                            if (solver2.model[i] == l_True) {
                                last_solution->insert(i + 1);
                            } else {
                                last_solution->insert(-i - 1);
                            }
                        }
                    }
                }
                if(decomposition_debug)
                    cout << endl;
                max2 = current_value2;
            } else {
                if (decomposition_debug) {
                    //cout << "----------" << endl;
                    cout << "(Decreasing) UNSAT with values " << current_value << ", " << current_value2 << endl;
                }
                min2 = current_value2;
            }
            current_value2 = (min2 + max2) / 2;
        } while((max2 - min2) > 1);



        //todo: solver model essendo unsat potrebbe essere una soluzione sbgliata
        if (exists_solution) {
            set<Region *> prova_PN;
            cout << "output model:" << endl;
            for(auto val: *last_solution){
                if(val > 0){
                    println(*regions_vector->at(val-1));
                    if(not_used_regions->find(regions_vector->at(val-1)) != not_used_regions->end())
                        not_used_regions->erase(regions_vector->at(val-1));
                }
            }
            auto temp_PN = new set<Region*>();
            for(auto r_index: *last_solution){
                if(r_index>0){
                    temp_PN->insert((*regions_vector)[r_index-1]);
                }
            }
            fcpn_set->insert(temp_PN);
            results_to_avoid->insert(last_solution);

            auto used_regions_map = get_map_of_used_regions(fcpn_set, pprg->get_pre_regions());
            excitation_closure = is_excitation_closed(used_regions_map, ER);
            for (auto rec: *used_regions_map) {
                delete rec.second;
            }
            delete used_regions_map;
            if (excitation_closure) {
                cout << "ec ok" << endl;
            }
            formula.clearDatabase();
        }
        else{
            cout << "no solution found" << endl;
            exit(0);
        }
        //TODO: perchè l'eliminazione di last solution crea loop se tale risultato non dovrebbe essere riutilizzato
        //delete last_solution;
    } while(!excitation_closure);



    string output_name = file;
    while (output_name[output_name.size() - 1] != '.') {
        output_name = output_name.substr(0, output_name.size() - 1);
    }
    output_name = output_name.substr(0, output_name.size() - 1);

    int counter = 0;

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
    //TODO: the regions of different fcpns have the wrong region names, have to bound them and solve it
    int pn_counter=0;
    for(auto pn: *fcpn_set){
        print_fcpn_dot_file(map_of_PN_pre_regions->at(pn), map_of_PN_post_regions->at(pn), aliases, file,pn_counter);
        pn_counter++;
    }

    if (decomposition_debug) {
        cout << "Final FCPNs" << endl;
        for (auto SM: *fcpn_set) {
            cout << "FCPN:" << endl;
            println(*SM);
        }
    }

    for(auto rec: *state_regions_map){
        delete rec.second;
    }
    delete state_regions_map;

    /*for(auto cl: *clauses){
        delete cl;
    }
    delete clauses;*/

    /*for(auto reg: *regions_vector){
        delete reg;
    }*/
    delete regions_vector;

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

    for(auto rec: *region_ex_event_map){
        delete rec.second;
    }
    delete region_ex_event_map;

    for(auto pn: *fcpn_set){
        delete pn;
    }
    delete fcpn_set;
    for(auto res: *results_to_avoid){
        delete res;
    }
    delete results_to_avoid;

    delete reg_map;
    delete not_used_regions;
    for(auto clause: *clauses){
        delete clause;
    }
    delete clauses;
}

FCPN_decomposition::~FCPN_decomposition()= default;