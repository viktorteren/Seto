/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include <include/GreedyRemoval.h>
#include "../include/FCPN_decomposition.h"

using namespace PBLib;
using namespace Minisat;
using namespace Utilities;


FCPN_decomposition::FCPN_decomposition(int number_of_events,
                                        set<Region *> *regions,
                                        const string& file,
                                        Pre_and_post_regions_generator *pprg,
                                        map<int, int> *aliases,
                                        map<int, ER> *ER){
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
     *      5) maximization function: number of new regions used in the result -> max covering
     *      6) solve the SAT problem decreasing the value of the region sum -> starting value is the sum of all regions
     *      7) decode result
     * 8) while !EC: previous results clauses are added as results to avoid in future
     * 9) greey FCPN removal
     */

    cout << "=========[FCPN DECOMPOSITION MODULE]===============" << endl;
    auto pre_regions_map = pprg->get_pre_regions();
    auto clauses = new vector<vector<int32_t> *>();
    auto fcpn_set = new set<set<Region *>*>();
    auto not_used_regions = new set<Region *>();
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

    auto results_to_avoid = new vector<set<int>>();
    auto reg_map = new map<Region*, int>();
    auto regions_vector = new vector<Region*>();
    int temp = 0;
    for(auto reg: *regions){
        (*reg_map)[reg] = temp;
        regions_vector->push_back(reg);
        not_used_regions->insert(reg);
        temp++;
    }

    vector<int32_t> *clause;

    //encoding: [1, k] regions range: k regions
    //encoding: [k+1, k+m+1] events range: m events
    int m = number_of_events;
    int k = regions->size();
    bool excitation_closure = false;

    //STEP 1
    //cout << "STEP 1a" << endl;
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
        for(auto cl: *clauses){
            delete cl;
        }
        clauses->clear();

        //cout << "STEP 1b" << endl;
        for(auto rec: *state_regions_map){
            auto region_set = rec.second;
            clause = new vector<int32_t>();
            for(auto reg: *region_set){
                clause->push_back(reg_map->at(reg)+1);
            }
            clauses->push_back(clause);
            //print_clause(clause);
        }

        //STEP 2
        //cout << "STEP 2" << endl;
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
            //auto ev = rec.first;
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


        //STEP 3
        //cout << "STEP 3" << endl;
        auto regions_connected_to_labels = merge_2_maps(pprg->get_pre_regions(),
                                                        pprg->get_post_regions());

        //conversion into clauses
        for (auto rec: *regions_connected_to_labels) {
            auto ev = rec.first;
            auto ev_encoding = k+2+ev;
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
        /*
        auto events_connected_to_regions = new map<Region *, set<int>*>();
        for(auto rec: *pprg->get_pre_regions()){
            auto ev = rec.first;
            for(auto reg: *rec.second){
                if(events_connected_to_regions->find(reg)==events_connected_to_regions->end()){
                    (*events_connected_to_regions)[reg] =new set<int>();
                }
                (*events_connected_to_regions)[reg]->insert(ev);
            }
        }
        for(auto rec: *pprg->get_post_regions()){
            auto ev = rec.first;
            for(auto reg: *rec.second){
                if(events_connected_to_regions->find(reg)==events_connected_to_regions->end()){
                    (*events_connected_to_regions)[reg] =new set<int>();
                }
                (*events_connected_to_regions)[reg]->insert(ev);
            }
        }
        for(auto rec: *events_connected_to_regions){
            auto reg = rec.first;
            for(auto ev: *rec.second){
                clause = new vector<int32_t>();
                auto ev_encoding = k+2+ev;
                int region_encoding = 1+reg_map->at(reg);
                clause->push_back(-region_encoding);
                clause->push_back(ev_encoding);
                clauses->push_back(clause);
            }
        }

        for(auto rec: *events_connected_to_regions){
            delete rec.second;
        }
        delete events_connected_to_regions;
         */

        //STEP 5
        //cout << "STEP 5" << endl;
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
                    cout << "SAT with value " << current_value << ": representing the number of new covered regions" << endl;
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
                min = current_value;
            } else {
                if (decomposition_debug) {
                    //cout << "----------" << endl;
                    cout << "UNSAT with value " << current_value << endl;
                    if(exists_solution) {
                        cout << "Model: ";
                        for (int i = 0; i < solver.nVars(); ++i) {
                            if (solver.model[i] != l_Undef) {
                                fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                        i + 1);
                            }
                        }
                        cout << endl;
                    }
                }
                max = current_value;
            }
            current_value = (min + max) / 2;
        } while ((max - min) > 1);

        if(!no_fcpn_min) {
            //STEP 6
            if (decomposition_debug)
                cout << "TRYING TO DECREASE THE NUMBER OF REGIONS" << endl;


            int current_value2 = 0;
            for (auto val: *last_solution) {
                if (val > 0) {
                    current_value2++;
                }
            }
            current_value2--;
            int min2 = 0;
            int max2 = current_value2;

            vector<WeightedLit> sum_of_regions = {};
            sum_of_regions.reserve(k);
            for (int i = 0; i < k; i++) {
                sum_of_regions.emplace_back(1 + i, 1);
            }


            int num_clauses_formula;
            //cout << "formula" << endl;
            //formula.printFormula(cout);

            //Minisat::Solver solver2;
            Minisat::Solver *solver2;
            //auto solver2 = new Minisat::Solver();


            //the sum have to be equal to current_value

            PBConfig config2 = make_shared<PBConfigClass>();
            VectorClauseDatabase formula2(config2);
            AuxVarManager auxvars2(k + m + 2);
            PBConstraint constraint3(literals_from_regions, BOTH,
                                     current_value, current_value);

            do {
                solver2 = new Minisat::Solver();
                auxvars2.resetAuxVarsTo(k + m + 2);
                formula2.clearDatabase();
                for (auto cl: *clauses) {
                    formula2.addClause(*cl);
                }
                pb2cnf.encode(constraint3, formula2, auxvars2);

                if (decomposition_debug)
                    cout << "values: " << current_value << ", " << current_value2 << endl;
                PBConstraint constraint2(sum_of_regions, LEQ,
                                         current_value2); //the sum have to be lesser or equal to current_value2
                pb2cnf.encode(constraint2, formula2, auxvars2);

                num_clauses_formula = formula2.getClauses().size();

                if (decomposition_debug)
                    cout << "Formula size: " << formula2.getClauses().size() << endl;

                dimacs_file = convert_to_dimacs(file, /*std::max(*/auxvars2.getBiggestReturnedAuxVar()/*, max_number)*/,
                                                num_clauses_formula,
                                                formula2.getClauses(), results_to_avoid);
                sat = check_sat_formula_from_dimacs(*solver2, dimacs_file);

                if (sat) {
                    if (decomposition_debug) {
                        cout << "(Decreasing) SAT with values " << current_value << ", " << current_value2 << endl;
                        cout << "Model: ";
                    }
                    last_solution->clear();
                    for (int i = 0; i < solver2->nVars(); ++i) {
                        if (solver2->model[i] != l_Undef) {
                            if (decomposition_debug) {
                                fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver2->model[i] == l_True) ? "" : "-",
                                        i + 1);
                            }
                            if (i < k) {
                                if (solver2->model[i] == l_True) {
                                    last_solution->insert(i + 1);
                                } else {
                                    last_solution->insert(-i - 1);
                                }
                            }
                        }
                    }
                    if (decomposition_debug)
                        cout << endl;
                    max2 = current_value2;
                } else {
                    if (decomposition_debug) {
                        cout << "(Decreasing) UNSAT with values " << current_value << ", " << current_value2 << endl;
                    }
                    min2 = current_value2;
                }
                current_value2 = (min2 + max2) / 2;
                delete solver2;
            } while ((max2 - min2) > 1);
        }


        if (exists_solution) {
            set<Region *> prova_PN;
            if(decomposition_debug)
                cout << "output model:" << endl;
            for(auto val: *last_solution){
                if(val > 0){
                    if(decomposition_debug) {
                        cout << val << ": ";
                        println(*regions_vector->at(val - 1));
                    }
                    if(val <= k)
                        if(not_used_regions->find(regions_vector->at(val-1)) != not_used_regions->end())
                            not_used_regions->erase(regions_vector->at(val-1));
                }
            }
            //STEP 7
            auto temp_PN = new set<Region*>();
            for(auto r_index: *last_solution){
                if(r_index>0){
                    temp_PN->insert((*regions_vector)[r_index-1]);
                }
            }
            fcpn_set->insert(temp_PN);
            /*auto last_solution_copy = new set<int>();
            for(auto val: *last_solution){
                last_solution_copy->insert(val);
            }*/
            results_to_avoid->push_back(*last_solution);

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
        delete last_solution;
    } while(!excitation_closure);

    cout << "PNs before greedy: " <<fcpn_set->size() << endl;


    //STEP 9
    GreedyRemoval::minimize(fcpn_set,pprg,ER,pre_regions_map);


    string output_name = file;
    while (output_name[output_name.size() - 1] != '.') {
        output_name = output_name.substr(0, output_name.size() - 1);
    }
    output_name = output_name.substr(0, output_name.size() - 1);

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

    if (decomposition_debug) {
        cout << "Final FCPNs" << endl;
        for (auto SM: *fcpn_set) {
            cout << "FCPN:" << endl;
            println(*SM);
        }
    }

    if(decomposition_debug){
        set<int> used_regions;
        for(auto pn: *fcpn_set){
            for(auto reg: *pn){
                used_regions.insert(regions_mapping->at(reg));
            }
        }
        cout << "Used regions: ";
        for(auto reg: used_regions){
            cout << "r" << reg << " ";
        }
        cout << endl;
    }

    if(pn_counter == 1){
        cout << "1 FCPN" << endl;
    }
    else {
        cout << pn_counter << " FCPNs" << endl;
    }

    delete regions_mapping;

    for(auto rec: *state_regions_map){
        delete rec.second;
    }
    delete state_regions_map;

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
    /*for(auto res: *results_to_avoid){
        delete res;
    }*/
    delete results_to_avoid;

    delete reg_map;
    delete not_used_regions;
    for(auto cl: *clauses){
        delete cl;
    }
    delete clauses;
}

FCPN_decomposition::~FCPN_decomposition()= default;