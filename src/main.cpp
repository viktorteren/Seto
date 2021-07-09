/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Label_splitting_module.h"
#include "../include/Merging_Minimal_Preregions_module.h"
#include "../include/Regions_generator.h"
#include "../include/Place_irredundant_pn_creation_module.h"
#include "../pblib/pb2cnf.h"
#include <algorithm>
#include <Python.h>
#include <iomanip>
#include <include/k_FCPN_decomposition.h>
#include "../include/Merge.h"
#include "../include/FCPN_decomposition.h"
#include "../include/GreedyRemoval.h"

using namespace PBLib;
using namespace Minisat;

int main(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    string file;
    if (argc >= 3) {
        file = args[1];
        cout << file << endl;
        print_step_by_step = false;
        print_step_by_step_debug = false;
        decomposition = false;
        decomposition_debug = false;
        decomposition_output = false;
        benchmark_script = false;
        fcptnet = false;
        pn_synthesis = false;
        for(int i=2; i < argc; i++) {
            if(args[i] == "PN")
                pn_synthesis = true;
            else if (args[i] == "TS")
                ts_output = true;
            else if(args[i]=="FC")
                fcptnet = true;
            else if(args[i]=="KFC") {
                fcptnet = true;
                k_fcpn_decomposition = true;
            }
            /*else if(args[i]=="KFCB") {
                fcptnet = true;
                blind_fcpn = true;
            }
            else if(args[i]=="KFCL"){
                fcptnet = true;
                fcpn_with_levels = true;
                cerr << "KFCM does not work properly" << endl;
                exit(1);
            }
            else if(args[i]=="KFCM"){
                fcptnet = true;
                fcpn_modified = true;
            }*/
            else if(args[i] == "ECTS")
                ects_output = true;
            else if(args[i] == "M")
                decomposition = true;
            else if (args[i] == "S") {
                if (decomposition) {
                    cerr << "A flag for decomposition was previously chosen" << endl;
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
                decomposition_output = true;
            }
            else if(args[i] == "G") {
                decomposition_output = true;
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
            else if(args[i] == "NOMIN"){
                no_fcpn_min = true;
            }
            else{
                cerr << "INVALID FLAG " << args[i] << endl;
                exit(1);
            }
        }
        if(fcptnet && decomposition_output_sis){
            cerr << "SIS output not implemented for FCPNs, remove G flag"<< endl;
            exit(0);
        }
        if(pn_synthesis && decomposition){
            cerr << "PN synthesis cannot be done together with SM decomposition" << endl;
            exit(0);
        }
        if(pn_synthesis && fcptnet){
            cerr << "PN synthesis cannot be done together with FCPN decomposition" << endl;
            exit(0);
        }
    }
    else{
        cerr << "Wrong number of arguments." << endl;
        exit(1);
    }

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
        map<int, ER> *new_ER;
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
                        "DELETING OF NON MINIMAL REGIONS "
                        "-------------------------------------------"
                     << endl;
            }
            vector<Region>::iterator it;
            for (it = vector_regions->begin(); it < vector_regions->end(); ++it) {
                Region *region = &(*it);
                rg->remove_bigger_regions(*region, vector_regions);
            }


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
            pprg = new Pre_and_post_regions_generator(vector_regions);
            pre_regions = pprg->get_pre_regions();

            if (print_step_by_step && !info) {
                cout << "Preregions:" << endl;
                for (auto rec: *pre_regions) {
                    cout << "event: " << rec.first << endl;
                    for (auto reg: *rec.second) {
                        println(*reg);
                    }
                }
                cout << "" << endl;
            }


            t_pre_region_gen += (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;


            tStart_partial = clock();

            ls = new Label_splitting_module(pre_regions, rg->get_ER_set(), rg->get_middle_set_of_states());

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
            //NON so se commentare o no i seguenti 2 delete, con pulse non si hanno memory leak, con altri non si può rimuovere
            rg->delete_ER_set();
            delete rg;

            t_splitting = t_splitting + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

        } while (!excitation_closure);

        auto rg = new Region_generator(number_of_events);
        rg->generate();
        new_ER = rg->get_ER_set();

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
            print_ts_dot_file(file, aliases);
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

        if (decomposition || fcptnet) {
            auto regions_set = copy_map_to_set(pre_regions);
            aliases_region_pointer = new map<int, Region *>();
            aliases_region_pointer_inverted = new map<Region *, int>();
            if(decomposition) {
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
                    //faster aproximated solution
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

                cout << "=================[END PYTHON]=================" << endl;

                auto SMs = new set<SM *>(); //set of SMs, each SM is a set of regions
                read_SMs("final_FSMs.txt", SMs, *aliases_region_pointer);

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
                            cout << "check if can rermove the SM: " << endl;
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
                cout
                        << "=======[ GREEDY REMOVAL OF SMs CHECKING EC USING HEURISTIC WHICH REMOVES BIGGEST SMs FIRST ]======"
                        << endl;

                tStart_partial = clock();

                GreedyRemoval::minimize(SMs,pprg,new_ER,pre_regions);

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

                //if(decomposition_debug)
                cout << "=======================[ CREATION OF PRE/POST-REGIONS FOR EACH SM ]================" << endl;

                //map with key the pointer to SM
                auto map_of_SM_pre_regions = new map < SM *, map<int, Region *> * > ();
                auto map_of_SM_post_regions = new map < SM *, map<int, Region *> * > ();

                for (auto sm: *SMs) {
                    (*map_of_SM_pre_regions)[sm] = new map < int, Region * > ();
                    for (auto rec: *pprg->get_pre_regions()) {
                        for (auto reg: *rec.second) {
                            if (sm->find(reg) != sm->end()) {
                                (*(*map_of_SM_pre_regions)[sm])[rec.first] = reg;
                            }
                        }
                    }
                    (*map_of_SM_post_regions)[sm] = pprg->create_post_regions_for_SM((*map_of_SM_pre_regions)[sm]);
                }

                //if(decomposition_debug)
                cout << "=======================[ FINAL SMs REDUCTION MODULE / LABELS REMOVAL ]================"
                     << endl;
                tStart_partial = clock();


                auto *merge = new Merge(SMs,
                                        clauses,
                                        number_of_events,
                                        map_of_SM_pre_regions,
                                        map_of_SM_post_regions,
                                        file,
                                        pprg);


                auto t_labels_removal = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

                auto final_sum = getStatesSum(SMs);
                auto final_avg = getStatesAvg(SMs);
                auto final_var = getStatesVar(SMs);
                auto final_transitions_sum = getTransitionsSum(map_of_SM_pre_regions);
                auto final_transitions_avg = getTransitionsAvg(map_of_SM_pre_regions);
                auto final_transitions_var = getTransitionsVar(map_of_SM_pre_regions);
                auto maxPTSum = getMaxPTSum(map_of_SM_pre_regions);
                auto maxTransitions = getMaxTransitionsNumber(map_of_SM_pre_regions);
                auto maxAlphabet = getMaxAlphabet(map_of_SM_pre_regions, aliases);

                //if(decomposition_debug)
                if (decomposition_output) {
                    decomposition_output_sis ? cout
                            << "=======================[ CREATION OF A .g FILE FOR EACH SM / S-COMPONENT  ]================"
                            << endl : cout
                            << "=======================[ CREATION OF A .dot FILE FOR EACH SM / S-COMPONENT  ]================"
                            << endl;
                    if (decomposition) {
                        Merge::print_after_merge(SMs, map_of_SM_pre_regions, map_of_SM_post_regions, aliases, file);
                    }
                }

                if (decomposition_debug && !fcptnet) {
                    cout << "Final SMs" << endl;
                    for (auto SM: *SMs) {
                        cout << "SM:" << endl;
                        println(*SM);
                    }
                }

                int n = merge->dimacs_file.length();
                const char* dimacs = merge->dimacs_file.c_str();

                //===========FREE===========
                delete merge;
                //delete SMs_sum;
                for (auto SM: *SMs) {
                    delete SM;
                }
                delete SMs;

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
                printf("Total time: %.5fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);
                cout << "SIZE STATISTICS:" << endl;
                cout << "Number of SMs: " << SMs->size() << endl;
                //cout << "States sum after the decomposition: " << states_sum << endl;
                //cout << "States sum after the removal of redundant SMs: " << states_after_sms_removal << endl;
                cout << "States sum after final optimization: " << final_sum << endl;
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
                outfile << fixed
                        << get_file_name(file) << ","
                        << setprecision(4) << t_region_gen << ","
                        << setprecision(4) << t_decomposition << ","
                        << setprecision(4) << setw(4) << t_greedy << ","
                        << setprecision(4) << t_labels_removal << ","
                        << states_sum << "," << states_after_sms_removal << ","
                        << final_sum << "," << transitions_sum << ","
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

            if(fcptnet){
                tStart_partial = clock();
                double t_k_fcpn_decomposition;
                if(k_fcpn_decomposition){

                    auto k_fcpn_decomposition_module = new k_FCPN_decomposition(number_of_events, regions_set, file,
                                                                         pprg, aliases, new_ER);
                    t_k_fcpn_decomposition = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;
                    delete k_fcpn_decomposition_module;
                }
                else{
                    for(auto reg: *regions_set){
                        if(aliases_region_pointer_inverted->find(reg) == aliases_region_pointer_inverted->end()){
                            region_mapping(reg);
                        }
                    }
                    auto fcpn_decomposition_module = new FCPN_decomposition(number_of_events, regions_set, file,
                                                                     pprg, aliases, new_ER, 0);
                    t_k_fcpn_decomposition = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;
                    delete fcpn_decomposition_module;
                }
                printf("\nTime region gen: %.5fs\n", t_region_gen);
                printf("Time splitting: %.5fs\n", t_splitting);
                printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
                if(k_fcpn_decomposition)
                    printf("Time k-FCPN decomposition: %.5fs\n", t_k_fcpn_decomposition);
                else
                    printf("Time FCPN decomposition: %.5fs\n", t_k_fcpn_decomposition);
            }
            //FREE
            rg->basic_delete();
            rg->delete_ER_set();
            delete regions_set;
        }
        else if (pn_synthesis){
            tStart_partial = clock();
            // Start of module: search of the irredundant set of regions
            auto pn_module = new Place_irredundant_pn_creation_module(pre_regions, new_ER);
            t_irred = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;
            auto essential_regions = pn_module->get_essential_regions();
            map<int, set<Region *> *> *irredundant_regions =
                    pn_module->get_irredundant_regions();

            Merging_Minimal_Preregions_module *merging_module = nullptr;

            //cout << "pre-regioni essenziali" << endl;
            //print(*essential_regions);

            if (irredundant_regions != nullptr) {
                //cout << "pre-regioni irridondanti" << endl;
                //print(*irredundant_regions);
                merging_module = new Merging_Minimal_Preregions_module(essential_regions, irredundant_regions, new_ER);
                // print_PN(essential_regions,irredundant_regions);
            } else {
                merging_module = new Merging_Minimal_Preregions_module(essential_regions,
                                                                       nullptr, new_ER);
                // print_PN(essential_regions, nullptr);
            }

            auto merged_map = merging_module->get_merged_preregions_map();

            //cout << "not merged pre-regions: " << endl;
            //print(*merging_module->get_total_preregions_map());


            if (merged_map == nullptr) {
                merged_map = merging_module->get_total_preregions_map();
            }

            //cout << "merged pre-regions: " << endl;
            //print(*merging_module->get_merged_preregions_map());


            /*cout << "merged map nel main: " << endl;
            print(*merged_map);*/
            /*pprg->create_post_regions(merging_module->get_total_preregions_map());
            auto post_regions2 = pprg->get_post_regions();
            print_pn_dot_file(merging_module->get_total_preregions_map(), post_regions2, aliases, file);*/

            auto t_merge = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

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

            print_pn_dot_file(merged_map, post_regions, aliases, file);

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