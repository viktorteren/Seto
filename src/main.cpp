/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/main.h"

using namespace PBLib;
using namespace Minisat;


int main(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    string file = parseArguments(argc,args);

    TS_parser::parse(file);

    cout << "===========[PARSING DONE]===========" << endl;

    if(!ts_output) {
        if (print_step_by_step_debug) {
            cout << "TS" << endl;
            for (const auto &tr: *ts_map) {
                cout << "event " << tr.first << endl;
                for (auto r: tr.second) {
                    cout << r->first << "->" << r->second << endl;
                }
            }
        }

        map<int, pair<int, Region *> *> *candidate_regions;
        map<int, set<Region *> *> *pre_regions;
        Label_splitting_module *ls;
        Pre_and_post_regions_generator *pprg = nullptr;
        map<int, ES> *new_ES;
        vector<Region> *vector_regions;
        map<int, vector<Region> *> *regions;

        double t_pre_region_gen = 0.0;
        double t_region_gen = 0.0;
        double t_splitting = 0.0;
        int number_of_events;
        auto aliases = new map<int, int>(); //map <new_label, old_label>

        clock_t tStart = clock();

        auto tStart_partial = clock();

        int num_split = 0;

        bool excitation_closure;
        //double dim_reg;
        num_events_after_splitting = static_cast<int>(ts_map->size());
        do {
            number_of_events = static_cast<int>(ts_map->size());
            auto rg = new Region_generator(number_of_events);
            regions = rg->generate();
            vector_regions = copy_map_to_vector(regions);

            if (print_step_by_step && !info) {
                cout << "Regions: " << endl;
                for (auto reg: *vector_regions) {
                    println(reg);
                }
                cout << "" << endl;
            }

            if (print_step_by_step_debug) {
                cout << "------------------------------------------------------------ "
                        "DELETING NON MINIMAL REGIONS "
                        "-------------------------------------------"
                     << endl;
            }
            //cout << "vector regions size: " << vector_regions->size() << endl;
            vector<Region>::iterator it;
            set<Region> checked_regions;
            for (it = vector_regions->begin(); it < vector_regions->end(); ++it) {
                Region *region = &(*it);
                if(checked_regions.find(*region) == checked_regions.end()){
                    //cout << "region: ";
                    //println(*region);
                    if(Region_generator::remove_bigger_regions(*region, vector_regions)){
                        it = vector_regions->begin();
                    }
                    checked_regions.insert(*region);
                    //cout << "CONT: " << cont << endl;
                }
            }
            //cout << "vector regions size after: " << vector_regions->size() << endl;


            if (print_step_by_step && !info) {
                cout << "Minimal regions: " << endl;
                for (auto r: *vector_regions) {
                    println(r);
                }
                cout << "" << endl;
            }

            if (print_step_by_step_debug) {
                cout << "regions" << endl;
                for (auto rec: *regions) {
                    cout << "event" << rec.first << endl;
                    for (auto r: *rec.second) {
                        cout << "reg: ";
                        println(r);
                    }
                }
            }

            num_events_after_splitting = static_cast<int>(ts_map->size());


            t_region_gen = t_region_gen + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;


            tStart_partial = clock();
            delete pprg;
            //cout << "new pre-regions creation" << endl;
            pprg = new Pre_and_post_regions_generator(vector_regions);
            pre_regions = pprg->get_pre_regions();

            /*
            if (decomposition_debug) {
                cout << "Pre-regions:" << endl;
                for (auto rec: *pre_regions) {
                    cout << "event: " << rec.first << endl;
                    for (auto reg: *rec.second) {
                        println(*reg);
                    }
                }
                cout << "" << endl;
            }*/


            t_pre_region_gen += (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;


            tStart_partial = clock();

            ls = new Label_splitting_module(pre_regions, rg->get_ES_set(), rg->get_middle_set_of_states());

            set<int> *events = ls->is_excitation_closed();

            excitation_closure = events->empty();
            if (print_step_by_step_debug)
                cout << "EC*************" << excitation_closure << endl;

            set<int> *events_not_satify_EC = nullptr;

            if (!excitation_closure) {
                num_split++;
                candidate_regions = ls->candidate_search(rg->get_number_of_bad_events(), events);
                if (print_step_by_step) { cout << "Splitting of the labels: "; }
                if (!pre_regions->empty())
                    ls->split_ts_map(candidate_regions, aliases, rg->get_violations_event(), rg->get_violations_trans(),
                                     nullptr);
                else
                    ls->split_ts_map(candidate_regions, aliases, rg->get_violations_event(), rg->get_violations_trans(),
                                     regions);

                map<int, pair<int, Region *> *>::iterator it2;
                for (it2 = candidate_regions->begin(); it2 != candidate_regions->end(); ++it2) {
                    delete it2->second;
                }
                delete candidate_regions;

                delete vector_regions;
                delete ls;
            }

            delete events;
            delete events_not_satify_EC;
            rg->basic_delete();
            rg->delete_ER_set();
            delete rg;

            t_splitting = t_splitting + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

        } while (!excitation_closure);

        auto rg = new Region_generator(number_of_events);
        rg->generate();
        new_ES = rg->get_ES_set();



        if (print_step_by_step) {
            int cont = 0;
            double sum = 0;
            for (const auto &reg: *vector_regions) {
                cont++;
                sum += reg.size();
            }
            cout << "average: " << (sum / cont) << endl;
            cout << "number of regions: " << vector_regions->size() << endl;
            cout << "number of splits: " << num_split << endl;
        }
        delete ls;

        if(ects_output){
            if(aut_output){
                print_ts_aut_file(file, aliases);
            }
            else{
                print_ts_dot_file(file, aliases);
            }
            exit(0);
        }

        if (print_step_by_step_debug) {
            cout << "ECTS:" << endl;
            for (const auto &tr: *ts_map) {
                cout << "event " << tr.first << endl;
                for (auto r: tr.second) {
                    cout << r->first << "->" << r->second << endl;
                }
            }
            cout << endl;
        }

        if(info){
            printf("\nTime region gen: %.5fs\n", t_region_gen);
            printf("Time splitting: %.5fs\n", t_splitting);
        }

        double t_irred;
        tStart_partial = clock();

        if (decomposition || fcptnet || acpn) {
            auto regions_set = copy_map_to_set(pre_regions);
            aliases_region_pointer = new map<int, Region *>();
            aliases_region_pointer_inverted = new map<Region *, int>();
            set<SM *> *SMs;
            if((!bdd_usage && decomposition) || (fcptnet && greedy_exact)) {
                SMs = new set<SM *>(); //set of SMs, each SM is a set of regions
            }
            if(decomposition && !bdd_usage) {

                cout << "============================[DECOMPOSITION]===================" << endl;

                int numRegions = regions_set->size();
                cout << "Number of regions: " << numRegions << endl;
                if(decomposition_debug){
                    for(auto reg: *regions_set){
                        println(*reg);
                    }
                }

                max_alias_decomp = 1;
                num_clauses = 0;

                if (decomposition_debug)
                    cout << "===============================[REDUCTION TO SAT OF THE OVERLAPS]====================="
                         << endl;
                overlaps_cache = new map<pair<Region *, Region *>, bool>();
                vector<vector<int32_t> *> *clauses = add_regions_clauses_to_solver(pre_regions);
                create_dimacs_graph(numRegions, clauses);

                cout << "==================[START PYTHON]============= " << endl;

                #ifdef USE_PYTHON
                string python_code;
                string python_source;
                //compute all possible minimal independent sets
                if (python_all) {
                    if (benchmark_script) {
                        python_source = "src/AllMISSolver.py";
                    } else {
                        python_source = "../src/AllMISSolver.py";
                    }
                }
                    //faster approximated solution
                else {
                    if (benchmark_script) {
                        python_source = "src/MIS-Solver.py";
                    } else {
                        python_source = "../src/MIS-Solver.py";
                    }
                }
                std::ifstream t(python_source);
                std::stringstream buffer;
                buffer << t.rdbuf();
                python_code = buffer.str();

                Py_Initialize();

                PyRun_SimpleString(python_code.c_str());
                Py_Finalize();
                #endif

                cout << "=================[END PYTHON]=================" << endl;

                read_SMs("final_FSMs.txt", SMs, *aliases_region_pointer);
                if(SMs->empty()){
                    cerr << "Python code was not performed or it generated an empty set of SMs." << endl;
                    exit(1);
                }

                if (python_all) {
                    cout << "==============REMOVAL OF SETs WHICH ARE NOT SMs====" << endl;
                    vector<set<Region *> *> SMs_to_remove;
                    set<int> states_of_TS;
                    for (const auto &tr: *ts_map) {
                        for (auto r: tr.second) {
                            states_of_TS.insert(r->first);
                            states_of_TS.insert(r->second);
                        }
                    }
                    for (auto SM: *SMs) {
                        /*if (decomposition_debug) {
                            cout << "check if can remove the SM: " << endl;
                            println(*SM);
                        }*/

                        set<int> new_used_states_tmp;

                        for (auto region: *SM) {
                            for (auto s: *region) {
                                new_used_states_tmp.insert(s);
                            }
                        }

                        if (new_used_states_tmp.size() < states_of_TS.size()) {
                            SMs_to_remove.push_back(SM);
                            if (decomposition_debug) {
                                cout << "not an SM:" << endl;
                                println(*SM);
                            }
                        }
                    }

                    while (!SMs_to_remove.empty()) {
                        if (decomposition_debug)
                            cout << "removing a NOT SM set" << endl;
                        auto SM = SMs_to_remove.begin();
                        SMs->erase(SMs->find(*SM));
                        SMs_to_remove.erase(SMs_to_remove.begin());
                    }
                }

                if (decomposition_debug) {
                    cout << "Initial SMs" << endl;
                    for (auto SM: *SMs) {
                        cout << "SM:" << endl;
                        println(*SM);
                        //print_SM_on_file(*SM, "DEBUG.txt");
                    }
                }

                auto t_decomposition = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

                auto states_sum = getStatesSum(SMs);
                auto temp_map_of_SM_pre_regions = new map<SM *, map<int, Region *> *>();
                for (auto sm: *SMs) {
                    (*temp_map_of_SM_pre_regions)[sm] = new map<int, Region *>();
                    for (auto rec: *pprg->get_pre_regions()) {
                        for (auto reg: *rec.second) {
                            if (sm->find(reg) != sm->end()) {
                                (*(*temp_map_of_SM_pre_regions)[sm])[rec.first] = reg;
                            }
                        }
                    }
                }
                auto transitions_sum = getTransitionsSum(temp_map_of_SM_pre_regions);
                for (auto map: *temp_map_of_SM_pre_regions) {
                    delete map.second;
                }
                delete temp_map_of_SM_pre_regions;

                //if(decomposition_debug)
                if(!fcptnet) {


                    tStart_partial = clock();

                    if(greedy_exact)
                        if(SMs->size() < 20)
                            GreedyRemoval::minimize_sat_SM_exact(SMs, new_ES, pre_regions);
                        else{
                            cerr << "Remove GE flag: 20 or more SM were generated in the first computation step" << endl;
                            cerr << "Exact algorithm cannot be performed O(2^n)" << endl;
                            exit(1);
                        }
                    else{
                        if(mixed_strategy && SMs->size() < 20){
                            GreedyRemoval::minimize_sat_SM_exact(SMs, new_ES, pre_regions);
                        }
                        else {
                            cout
                                    << "=======[ GREEDY REMOVAL OF SMs CHECKING EC USING HEURISTIC WHICH REMOVES BIGGEST SMs FIRST ]======"
                                    << endl;
                            GreedyRemoval::minimize(SMs, pprg, new_ES, pre_regions);
                        }
                    }


                    auto t_greedy = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

                    auto states_after_sms_removal = getStatesSum(SMs);
                    temp_map_of_SM_pre_regions = new map < SM *, map < int, Region * > * > ();
                    for (auto sm: *SMs) {
                        (*temp_map_of_SM_pre_regions)[sm] = new map < int, Region * > ();
                        for (auto rec: *pprg->get_pre_regions()) {
                            for (auto reg: *rec.second) {
                                if (sm->find(reg) != sm->end()) {
                                    (*(*temp_map_of_SM_pre_regions)[sm])[rec.first] = reg;
                                }
                            }
                        }
                    }
                    auto transitions_after_sms_removal = getTransitionsSum(temp_map_of_SM_pre_regions);
                    for (auto map: *temp_map_of_SM_pre_regions) {
                        delete map.second;
                    }
                    delete temp_map_of_SM_pre_regions;

                    if (decomposition_debug) {
                        cout << "Remaining SMs:" << endl;
                        for (auto sm: *SMs) {
                            cout << endl;
                            println(*sm);
                        }
                    }

                    /*
                    for (auto SM: *SMs) {
                        //cout << "SM:" << endl;
                        //println(*SM);
                        print_SM_on_file(*SM, "DEBUG.txt");
                    }*/

                    //if(decomposition_debug)
                    cout << "=======================[ CREATION OF PRE/POST-REGIONS FOR EACH SM ]================"
                         << endl;

                    //map with key the pointer to SM
                    auto map_of_SM_pre_regions = new map < SM *, map<int, Region *>
                    * > ();
                    auto map_of_SM_post_regions = new map < SM *, map<int, Region *>
                    * > ();

                    for (auto sm: *SMs) {
                        (*map_of_SM_pre_regions)[sm] = new map < int, Region * > ();
                        for (auto rec: *pprg->get_pre_regions()) {
                            for (auto reg: *rec.second) {
                                if (sm->find(reg) != sm->end()) {
                                    (*(*map_of_SM_pre_regions)[sm])[rec.first] = reg;
                                }
                            }
                        }
                        (*map_of_SM_post_regions)[sm] = Pre_and_post_regions_generator::create_post_regions_for_SM(
                                (*map_of_SM_pre_regions)[sm]);
                    }

                    Merge *merge;

                    if (!no_merge) {
                        cout << "=======================[ FINAL SMs REDUCTION MODULE / LABELS REMOVAL ]================"
                             << endl;
                        tStart_partial = clock();

                        merge = new Merge(SMs,
                                          number_of_events,
                                          map_of_SM_pre_regions,
                                          map_of_SM_post_regions,
                                          file,
                                          nullptr);

                    }

                    auto t_labels_removal = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

                    if (composition)
                        SM_composition::compose(SMs, map_of_SM_pre_regions, map_of_SM_post_regions, aliases, file);

                    auto final_sum = getStatesSum(SMs);
                    auto final_avg = getStatesAvg(SMs);
                    auto final_var = getStatesVar(SMs);
                    auto final_transitions_sum = getTransitionsSum(map_of_SM_pre_regions);
                    auto final_transitions_avg = getTransitionsAvg(map_of_SM_pre_regions);
                    auto final_transitions_var = getTransitionsVar(map_of_SM_pre_regions);
                    auto maxPTSum = getMaxPTSum(map_of_SM_pre_regions);
                    auto maxTransitions = getMaxTransitionsNumber(map_of_SM_pre_regions);
                    maxAlphabet = getMaxAlphabet(map_of_SM_pre_regions, aliases);

                    //if(decomposition_debug)
                    if (output) {
                        decomposition_output_sis ? cout
                                << "=======================[ CREATION OF A .g FILE FOR EACH SM / S-COMPONENT  ]================"
                                << endl : cout
                                << "=======================[ CREATION OF A .dot FILE FOR EACH SM / S-COMPONENT  ]================"
                                << endl;
                        if (decomposition) {
                            Merge::print_after_merge(SMs, map_of_SM_pre_regions, map_of_SM_post_regions, aliases, file);
                        }
                    }

                    if (decomposition_debug && (!fcptnet && !acpn)) {
                        cout << "Final SMs" << endl;
                        for (auto SM: *SMs) {
                            cout << "SM:" << endl;
                            println(*SM);
                        }
                    }

                    int n = merge->dimacs_file.length();
                    const char *dimacs = merge->dimacs_file.c_str();

                    //===========FREE===========
                    if (!no_merge)
                        delete merge;
                    //delete SMs_sum;
                    if(!fcptnet) {
                        for (auto SM: *SMs) {
                            delete SM;
                        }
                        delete SMs;
                    }

                    for (auto vec: *clauses) {
                        delete vec;
                    }

                    delete clauses;

                    for (auto map: *map_of_SM_pre_regions) {
                        delete map.second;
                    }
                    delete map_of_SM_pre_regions;
                    for (auto map: *map_of_SM_post_regions) {
                        delete map.second;
                    }
                    delete map_of_SM_post_regions;

                    // declaring character array
                    char dimacs_file_array[n + 1];

                    // copying the contents of the
                    // string to char array
                    strcpy(dimacs_file_array, dimacs);
                    remove(dimacs_file_array);

                    printf("\nTime region gen: %.5fs\n", t_region_gen);
                    printf("Time splitting: %.5fs\n", t_splitting);
                    printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
                    printf("Time decomposition: %.5fs\n", t_decomposition);
                    printf("Time greedy SM removal: %.5fs\n", t_greedy);
                    printf("Time labels removal / final SMs minimization: %.5fs\n", t_labels_removal);
                    printf("Total decomposition time : %.5fs\n", t_decomposition+t_greedy+t_labels_removal);
                    printf("Total time: %.5fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);
                    cout << "SIZE STATISTICS:" << endl;
                    cout << "Number of SMs: " << SMs->size() << endl;
                    cout << "Number of places after the initial decomposition: " << states_sum << endl;
                    cout << "Number of places after the removal of redundant SMs: " << states_after_sms_removal << endl;
                    cout << "Number of places after the final optimization: " << final_sum << endl;
                    cout << "Avg. states per SM: " << final_avg << endl;
                    cout << "Var. states per SM: " << final_var << endl;
                    cout << "Avg. alphabet size per SM: " << final_transitions_avg << endl;
                    cout << "Var. alphabet size per SM: " << final_transitions_var << endl;
                    //cout << "Transitions sum after the decomposition: " << transitions_sum << endl;
                    //cout << "Transitions sum after the removal of redundant SMs: " << transitions_after_sms_removal << endl;
                    cout << "Transitions sum after final optimization: " << final_transitions_sum << endl;
                    cout << "Max size P+T: " << maxPTSum << endl;
                    cout << "Max number of places of an SM: " << (maxPTSum - maxTransitions) << endl;
                    cout << "Max alphabet of an SM: " << maxAlphabet << endl;
                    std::ofstream outfile;
                    outfile.open("stats.csv", std::ios_base::app);
                    if(greedy_exact){
                        outfile << "SM_exact_removal,";
                    }
                    else if(mixed_strategy){
                        outfile << "SM_mixed_strategy,";
                    }
                    else {
                        outfile << "SM,";
                    }
                    outfile << fixed
                            << get_file_name(file) << ","
                            << setprecision(2) << t_region_gen << ","
                            << setprecision(2) << t_decomposition << ","
                            << setprecision(2) << setw(4) << t_greedy << ","
                            << setprecision(2) << t_labels_removal << ","
                            << setprecision(2) << t_decomposition+t_greedy+t_labels_removal << ","
                            << states_sum << ","
                            << states_after_sms_removal << ","
                            << final_sum << ","
                            << transitions_sum << ","
                            << transitions_after_sms_removal << ","
                            << final_transitions_sum << ","
                            << SMs->size() << ","
                            << maxPTSum << ","
                            << maxAlphabet << ","
                            << maxTransitions << ","
                            << setprecision(2) << final_avg << ","
                            << setprecision(2) << final_var << ","
                            << setprecision(2) << final_transitions_avg << ","
                            << setprecision(2) << final_transitions_var
                            << endl;
                }
            }

            if(fcptnet || acpn || (decomposition & bdd_usage)){
                if(!bdd_usage && decomposition && !fcptnet){
                    delete SMs;
                }
                tStart_partial = clock();
                double t_fcpn_decomposition;
                double t_runtime;
                for (auto reg: *regions_set) {
                    if (aliases_region_pointer_inverted->find(reg) == aliases_region_pointer_inverted->end()) {
                        region_mapping(reg);
                    }
                }
                set<set<Region *> *> *final_pn_set=nullptr;
                if(bdd_usage){
                    int max_num_pre_regions = 0;
                    for(auto rec: *pre_regions){
                        int num_pre_regions_event = rec.second->size();
                        if(num_pre_regions_event > max_num_pre_regions)
                            max_num_pre_regions = num_pre_regions_event;
                    }

                    cout << "MAX NUMBER OF EVENT PRE-REGIONS: " << max_num_pre_regions << endl;
                    if(max_num_pre_regions <= 10) {
                        auto be = new BDD_encoder(pre_regions, new_ES);

                        //search a solution with n FCPNs increasing n until the result becomes SAT
                        final_pn_set = PN_decomposition::search_k(number_of_events, regions_set, file,
                                                                  pprg, new_ES, aliases, be);
                        delete be;
                    }
                    else{
                        final_pn_set = PN_decomposition::search(number_of_events, *regions_set, file,
                                                                pprg, new_ES, aliases, SMs);
                    }
                }
                else{
                    final_pn_set = PN_decomposition::search(number_of_events, *regions_set, file,
                                                            pprg, new_ES, aliases, SMs);
                }

                if (decomposition_debug) {
                    if(fcptnet)
                        cout << "Final FCPNs" << endl;
                    else if(acpn)
                        cout << "Final ACPNs" << endl;
                    else if(decomposition && bdd_usage)
                        cout << "Final SMs" << endl;
                    for (auto PN: *final_pn_set) {
                        if(fcptnet)
                            cout << "FCPN:" << endl;
                        else if(acpn)
                            cout << "ACPN:" << endl;
                        else if(decomposition && bdd_usage)
                            cout << "SM:" << endl;
                        println(*PN);
                    }
                }


                int pn_counter = final_pn_set->size();

                if (pn_counter == 1) {
                    if(fcptnet) {
                        cout << "1 FCPN" << endl;
                    }else if(acpn) {
                        cout << "1 ACPN" << endl;
                    }else
                        cout << "1 SM" << endl;
                } else {
                    if(fcptnet) {
                        cout << pn_counter << " FCPNs" << endl;
                    }else if(acpn) {
                        cout << pn_counter << " ACPNs" << endl;
                    }else
                        cout << pn_counter << " SMs" << endl;
                }
                int num_places = 0;
                for (auto FCPN: *final_pn_set) {
                    num_places += FCPN->size();
                }

                cout << "Total number of places: " << num_places << endl;



                t_runtime = (double) (clock() - tStart) / CLOCKS_PER_SEC;
                t_fcpn_decomposition = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;
                std::ofstream outfile;
                outfile.open("stats.csv", std::ios_base::app);
                if(bdd_usage) {
                    if (decomposition)
                        outfile << "SM_BDD";
                    else
                        outfile << "FCPN_BDD";
                }
                else if(acpn)
                    outfile << "ACPN";
                else
                    outfile << "FCPN";
                if(optimal)
                    outfile << "_OPTIMAL";
                if(safe_components_SM)
                    outfile << "_SS";
                if(safe_components)
                    outfile << "_SAFE(TRIVIAL)";
                if(no_reset)
                    outfile << "_NORESET";
                if(decomposition_debug)
                    outfile << "_DEBUG,";
                else
                    outfile << ",";
                outfile << fixed
                        << get_file_name(file) << ","
                        << setprecision(2) << t_runtime << ","
                        << setprecision(2) << t_region_gen << ","
                        << setprecision(2) << t_fcpn_decomposition << ","
                        << num_places << ","
                        << final_pn_set->size() << ","
                        << places_after_initial_decomp << ","
                        << places_after_greedy << ","
                        << maxAlphabet << ","
                        << avgAlphabet
                        << endl;
                //todo: remove if solving memory leak, why the internal code is a problem for isend?
                //if(!bdd_usage) {
                for (auto FCPN: *final_pn_set) {
                    for (auto reg: *FCPN) {
                        if (reg != nullptr) {
                            if (regions_set->find(reg) == regions_set->end()) {
                                delete reg;
                            }
                        }
                    }
                    delete FCPN;
                }
                delete final_pn_set;
                //}

                cout << "MAX alphabet: " << maxAlphabet << endl;
                cout << "AVG alphabet: " << avgAlphabet << endl;
                printf("\nTime region gen: %.5fs\n", t_region_gen);
                printf("Time splitting: %.5fs\n", t_splitting);
                printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
                if(bdd_usage && !decomposition)
                    printf("Time k-FCPN decomposition: %.5fs\n", t_fcpn_decomposition);
                else if(bdd_usage && decomposition)
                    printf("Time k-SM decomposition: %.5fs\n", t_fcpn_decomposition);
                else if (fcptnet)
                    printf("Time FCPN decomposition: %.5fs\n", t_fcpn_decomposition);
                else
                    printf("Time ACPN decomposition: %.5fs\n", t_fcpn_decomposition);
                printf("Total time: %.5fs\n", t_runtime);
            }
            //FREE
            rg->basic_delete();
            rg->delete_ER_set();
            delete regions_set;
        }
        else if (pn_synthesis || conformance_checking){
            tStart_partial = clock();
            // Start of module: search of the irredundant set of regions
            auto pn_module = new Place_irredundant_pn_creation_module(pre_regions, new_ES);
            t_irred = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;
            auto essential_regions = pn_module->get_essential_regions();
            map<int, set<Region *> *> *irredundant_regions =
                    pn_module->get_irredundant_regions();

            Merging_Minimal_Preregions_module *merging_module = nullptr;
            map<int, set<Region *>*> *merged_map; //map of pre-regions for an event
            if(!no_merge) {
                //cout << "pre-regioni essenziali" << endl;
                //print(*essential_regions);

                if (irredundant_regions != nullptr) {
                    //cout << "pre-regioni irridondanti" << endl;
                    //print(*irredundant_regions);
                    merging_module = new Merging_Minimal_Preregions_module(essential_regions, irredundant_regions,
                                                                           new_ES);
                    // print_PN(essential_regions,irredundant_regions);
                } else {
                    merging_module = new Merging_Minimal_Preregions_module(essential_regions,
                                                                           nullptr, new_ES);
                    // print_PN(essential_regions, nullptr);
                }

                merged_map = merging_module->get_merged_preregions_map();


                //cout << "not merged pre-regions: " << endl;
                //print(*merging_module->get_total_preregions_map());


                if (merged_map == nullptr) {
                    merged_map = merging_module->get_total_preregions_map();
                }
            }
            else{
                merged_map = pre_regions;
            }


            //cout << "merged pre-regions: " << endl;
            //print(*merging_module->get_merged_preregions_map());


            /*cout << "merged map nel main: " << endl;
            print(*merged_map);*/
            /*pprg->create_post_regions(merging_module->get_total_preregions_map());
            auto post_regions2 = pprg->get_post_regions();
            print_pn_dot_file(merging_module->get_total_preregions_map(), post_regions2, aliases, file);*/

            auto t_merge = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

            auto regions_set = new set<Region *>();

            for (auto rec: *merged_map) {
                for (auto reg: *rec.second) {
                    regions_set->insert(reg);
                }
            }

            int number_of_places = regions_set->size();

            cout << "Number of places: " << number_of_places << endl;


            if(output) {
                pprg->create_post_regions(merged_map);
                // pprg->create_post_regions(pprg->get_pre_regions());

                auto post_regions = pprg->get_post_regions();

                // restore_default_labels(merged_map, pprg->get_events_alias());
                // print_pn_dot_file( pprg->get_pre_regions(), post_regions,
                // pprg->get_events_alias(), file);
                /*cout << "print aliases" << endl;
                for(auto rec: *aliases){
                    cout << rec.first << " : " << rec.second << endl;
                }*/

                /*cout << "aliases: " << endl;
                for(auto al: *aliases){
                    cout<<al.first<<"->"<<al.second<<endl;
                }*/

                //cout<<"print finale PN"<<endl;
                //cout<<"post regions"<<endl;
                //print(*post_regions);
                //cout<<"pre regions merged map"<<endl;
                //print(*merged_map);



                if(conformance_checking){
                    //print_for_conformance_checking(merged_map, post_regions, aliases, file);
                    if(output){
                        int pn_counter = 0;
                        for (Region *region: *regions_set) {
                            print_cc_component_dot_file(region, merged_map, post_regions, aliases, file, pn_counter);
                            pn_counter++;
                        }
                    }
                }
                else {
                    print_pn_dot_file(merged_map, post_regions, aliases, file);
                }
            }

            delete regions_set;

            // cout << "fine ricerca " << endl;

            //===============FREE ============
            delete pn_module;
            delete merging_module;

            rg->basic_delete();

            printf("\nTime region gen: %.5fs\n", t_region_gen);
            printf("Time splitting: %.5fs\n", t_splitting);
            printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
            printf("Time essential+irredundant: %.5fs\n", t_irred);
            printf("Time merge: %.5fs\n", t_merge);
            printf("Total time: %.5fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);
        }

        // ============== COMMON BRANCH FREE ============

        delete aliases;

        delete vector_regions;

        delete rg;
        delete pprg;

        for (const auto &el : *ts_map) {
            for (auto p : el.second) {
                delete p;
            }
        }

        delete ts_map;
        delete aliases_map_number_name;
        delete aliases_map_name_number;
        delete aliases_map_state_number_name;
        delete aliases_map_state_name_number;
    }
}

string parseArguments(int argc, vector<string> args) {
    string file;
    if (argc >= 3) {
        file = args[1];
        cout << file << endl;
        print_step_by_step = false;
        print_step_by_step_debug = false;
        decomposition = false;
        decomposition_debug = false;
        output = false;
        benchmark_script = false;
        no_fcpn_min = true;
        fcptnet = false;
        acpn = false;
        pn_synthesis = false;
        composition = false;
        greedy_exact = false;
        dot_output = false;
        bdd_usage = false;
        ignore_correctness = false;
        check_structure = false;
        no_bounds = false;
        mixed_strategy = false;
        only_safeness_check = false;
        safe_components = false;
        safe_components_SM = false;
        optimal = false;
        no_reset = false;
        count_SMs = false;
        region_counter = false;
        unsafe_path = false;
        no_timeout = false;
        counter_optimized = false;
        conformance_checking = false;
        parallel = false;
        python_available = false;
        #ifdef USE_PYTHON
            python_available = true;
        #endif
        for(int i=2; i < argc; i++) {
            if(args[i] == "PN")
                pn_synthesis = true;
            else if (args[i] == "TS")
                ts_output = true;
            else if(args[i]=="FC")
                fcptnet = true;
            else if(args[i]=="AC")
                acpn = true;
            else if(args[i]=="CC") {
                conformance_checking = true;
            }
            else if(args[i] == "NOMERGE"){
                no_merge = true;
            }
            else if(args[i] == "COMPOSE"){
                composition = true;
            }
            else if(args[i] == "DOT"){
                dot_output = true;
            }
            else if(args[i] == "GE"){
                greedy_exact = true;
            }
            else if(args[i] == "B"){
                fcptnet = true;
                decomposition = true;
            }
            else if(args[i] == "BDD"){
                bdd_usage = true;
            }
            else if(args[i] == "CHECK"){
                check_structure = true;
            }
            else if(args[i] == "NOBOUNDS"){
                no_bounds = true;
            }
            else if(args[i] == "SC"){
                only_safeness_check = true;
            }
            else if(args[i] == "NOTIMEOUT"){
                no_timeout = true;
            }
            else if(args[i] == "SAFE"){
                safe_components = true;
            }
            else if(args[i] == "SS"){
                safe_components_SM = true;
            }
            else if(args[i] == "NORESET"){
                no_reset = true;
            }
            else if(args[i] == "CS"){
                count_SMs = true;
            }
            else if(args[i] == "ECTS")
                ects_output = true;
            else if(args[i] == "SM")
                decomposition = true;
            else if (args[i] == "S") {
                if (decomposition) {
                    cerr << "A flag for decomposition was previously chosen." << endl;
                    exit(1);
                }
                print_step_by_step = true;
            }
            else if (args[i] == "D") {
                //print_step_by_step = true;
                //print_step_by_step_debug = true;
                decomposition_debug = true;
            }
                /*if (args[i] == "L"){
                    log_file = true;
                }*/
            else if(args[i] == "O"){
                output = true;
            }
            else if(args[i] == "G") {
                output = true;
                decomposition_output_sis = true;
                benchmark_script = true;
            }
            else if(args[i] == "-ALL"){
                python_all = true;
            }
            else if(args[i] == "--INFO"){
                info = true;
                print_step_by_step = true;
            }
                /*
                else if(args[i] == "MIN"){
                    no_fcpn_min = false;
                }*/
            else if(args[i] == "AUT"){
                aut_output = true;
            }
            else if(args[i] == "I"){
                ignore_correctness = true;
            }
            else if(args[i] == "MS"){
                mixed_strategy = true;
            }
            else if(args[i] == "OPTIMAL"){
                optimal = true;
            }
            else if(args[i] == "COUNTER"){
                region_counter = true;
            }
            else if(args[i] == "COUNTER_OPTIMIZED"){
                region_counter = true;
                counter_optimized = true;
            }
            else if(args[i] == "UNSAFE_PATH"){
                unsafe_path = true;
            }
            else{
                cerr << "INVALID FLAG " << args[i] << endl;
                exit(1);
            }
        }
        if(fcptnet && decomposition_output_sis){
            cerr << "SIS output not implemented for FCPNs, remove G flag."<< endl;
            exit(0);
        }
        if(optimal && !bdd_usage){
            cerr << "OPTIAL flag can be used only with BDD flag."<< endl;
            exit(0);
        }
        if(region_counter && !safe_components){
            cerr << "COUNTER flag can be used only with SAFE flag."<< endl;
            exit(0);
        }
        if(no_bounds && !bdd_usage){
            cerr << "NOBOUNDS flag can be used only with BDD flag." << endl;
            exit(0);
        }
        if(decomposition && !no_bounds && only_safeness_check){
            cerr << "SC cannot be used without NOBOUNDS flag if SM decomposition is performed." << endl;
            exit(0);
        }
        if(decomposition && only_safeness_check && !bdd_usage){
            cerr << "SC cannot be used without BDD flag if SM decomposition is performed." << endl;
            exit(0);
        }
        if(acpn && decomposition_output_sis){
            cerr << "SIS output not implemented for ACPNs, remove G flag."<< endl;
            exit(0);
        }
        if(pn_synthesis && decomposition){
            cerr << "PN synthesis cannot be done together with SM decomposition." << endl;
            exit(0);
        }
        if(mixed_strategy && bdd_usage){
            cerr << "Mixed strategy cannot be performed with BDD flag" << endl;
            exit(0);
        }
        if(mixed_strategy && greedy_exact){
            cerr << "Mixed strategy cannot be performed with GE flag" << endl;
            exit(0);
        }
        if(decomposition && no_merge && !bdd_usage){
            cerr << "SM decomposition cannot be performed with NOMERGE flag if BDD flag is not used." << endl; //todo
            exit(0);
        }
        if(pn_synthesis && fcptnet){
            cerr << "PN synthesis cannot be done together with FCPN decomposition." << endl;
            exit(0);
        }
        if(pn_synthesis && acpn){
            cerr << "PN synthesis cannot be done together with ACPN decomposition." << endl;
            exit(0);
        }
        if(bdd_usage && !no_fcpn_min){
            cerr << "BDD usage cannot be performed with PN minimization (yet)." << endl;
            exit(0);
        }
        if(decomposition && !bdd_usage && check_structure){
            cerr << "Structure check not yet implemented on SM decomposition without BDD usage" << endl; //TODO
            exit(1);
        }
        if(aut_output){
            if(!ts_output && !ects_output){
                cerr << "AUT output flag is compatible only with TS and ECTS flags." << endl;
                exit(0);
            }
        }
        if(composition){
            if((!fcptnet && !decomposition && !acpn)){
                cerr << "Composition works only with FCPN/SM decomposition (excluded k-FCPN decomposition)." << endl;
                exit(0);
            }
        }
        if(fcptnet && acpn){
            cerr << "FCPN decomposition and ACPN decomposition cannot be performed on the same run." << endl;
            exit(0);
        }
        if(check_structure && decomposition){
            cerr << "CHECK flag cannot be used with SM decomposition (including SM decomposition with BDD." << endl;
            exit(0);
        }
        if(check_structure && pn_synthesis){
            cerr << "CHECK flag cannot be used with PN synthesis." << endl;
            exit(0);
        }
        if(safe_components_SM && decomposition && !fcptnet && !acpn){
            cerr << "SS flag cannot be used on SMs." << endl;
            exit(0);
        }
        if(safe_components_SM && bdd_usage){
            cerr << "SS flag cannot be combined with BDD flag." << endl;
            exit(0);
        }
        if(safe_components && !fcptnet){
            cerr << "Safeness check can be done only on FCPNs." << endl;
            exit(0);
        }
        if(safe_components && safe_components_SM){
            cerr << "SS and SAFE flags cannot be used at the same time."<< endl;
            exit(0);
        }
        if(decomposition && !python_available && !bdd_usage){
            cerr << "<Python.h> was not found: SM decomposition without usage of BDDs cannot be performed."  << endl;
            exit(0);
        }
    }
    else{
        cerr << "Wrong number of arguments." << endl;
        exit(1);
    }
    return file;
}