#include "../include/Label_splitting_module.h"
#include "../include/Merging_Minimal_Preregions_module.h"
#include "../include/Regions_generator.h"
#include "../include/Pre_and_post_regions_generator.h"
#include "../include/Place_irredundant_pn_creation_module.h"
#include "../pblib/pb2cnf.h"
#include <algorithm>

using namespace PBLib;
using namespace Minisat;

int main(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    string file;
    if (argc == 1) {
        // default input
        print_step_by_step = false;
        file = "../test/input1.ts";
    } else if (argc == 2) {
        file = args[1];
        cout << file << endl;
    } else if (argc == 3) {
        file = args[1];
        cout << file << endl;
        if (args[2] == "S") {
            print_step_by_step = true;
            print_step_by_step_debug = false;
            decomposition = false;
            decomposition_debug = false;
        } else if (args[2] == "D") {
            print_step_by_step = true;
            print_step_by_step_debug = true;
            decomposition = false;
            decomposition_debug = false;
        } else if (args[2] == "M") {
            print_step_by_step = false;
            print_step_by_step_debug = false;
            decomposition = true;
            decomposition_debug = false;
        } else if (args[2] == "MD") {
            print_step_by_step = false;
            print_step_by_step_debug = false;
            decomposition = true;
            decomposition_debug = true;
        } else {
            print_step_by_step = false;
            print_step_by_step_debug = false;
            decomposition = false;
            decomposition_debug = false;
        }
    } else {
        cout << "Wrong number of input arguments" << endl;
        exit(0);
    }

    TS_parser::parse(file);

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
    auto aliases = new map<int, int>();

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

        if (print_step_by_step) {
            cout << "Regions: " << endl;
            for (auto reg: *vector_regions) {
                println(reg);
            }
            cout << "" << endl;
        }

        if(print_step_by_step_debug) {
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


        if (print_step_by_step) {
            cout << "Minimal regions: " << endl;
            for (auto r: *vector_regions) {
                println(r);
            }
            cout << "" << endl;
        }

        if(print_step_by_step_debug) {
            cout<<"regions"<<endl;
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

        if (print_step_by_step) {
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
        if(print_step_by_step_debug)
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

            new_ER = rg->get_ER_set();

            delete vector_regions;
            delete ls;

            for (auto el : *new_ER)
                delete el.second;
            delete new_ER;

            for (auto reg_vec: *regions) {
                delete reg_vec.second;
            }
            delete regions;
        } else {
            new_ER = rg->get_ER_set();
        }

        delete events;
        delete events_not_satify_EC;

        delete rg;

        t_splitting = t_splitting + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

    }//end prova do while
    while (!excitation_closure);

    if (print_step_by_step) {
        int cont = 0;
        double somma = 0;
        for (const auto &reg: *vector_regions) {
            cont++;
            somma += reg.size();
        }
        cout << "average: " << (somma / cont) << endl;

        cout << "number of regions: " << vector_regions->size() << endl;
        cout << "number of splits: " << num_split << endl;
    }
    delete ls;

    print_ts_dot_file(file, aliases);

    if(print_step_by_step_debug) {
        cout<<"ECTS:"<<endl;
        for(const auto& tr: *ts_map){
            cout<<"evento "<< tr.first<<endl;
            for(auto r: tr.second){
                cout<<r->first<< "->"<< r->second<<endl;
            }
        }
        cout << "" << endl;
    }

    tStart_partial = clock();

    if (decomposition) {
        cout << "============================[DECOMPOSITION]===================" << endl;
        auto regions_set = copy_map_to_set(pre_regions);
        int numRegions =  regions_set->size();
        cout << "Number of regions: " << numRegions << endl;
        int startingMinValueToCheck = 0;
        int minValueToCheck;
        auto regions_sorted = new vector<Region*>();
        for(auto reg: *regions_set){
            regions_sorted->push_back(reg);
        }

        //sorting of the regions in descending size order
        sort( regions_sorted->begin( ), regions_sorted->end( ), [ ](  Region *lhs,  Region *rhs )
        {
            return lhs->size() > rhs->size();
        });

        //Setting up the first value to overcom in the inequality
        unsigned long sum = 0;
        for(auto & i : *regions_sorted){
            sum += i->size();
            startingMinValueToCheck++;
            if(sum >= num_states)
                break;
        }

        //auto new_results_to_avoid = new set<set<int> *>();
        aliases_region_pointer = new map<int, Region *>();
        aliases_region_pointer_inverted = new map<Region *, int>();
        max_alias_decomp = 1;
        num_clauses = 0;
        set<int> used_regions;
        auto used_regions_map = new map<int,set< Region *>*>();
        for(auto rec: *pre_regions){
            (*used_regions_map)[rec.first] = new set<Region *>();
        }
        if(decomposition_debug)
            cout << "===============================[REDUCTION TO SAT OF THE OVERLAPS]=====================" << endl;
        overlaps_cache = new map<pair<Region *, Region *>, bool>();
        vector<vector<int32_t> *> *clauses = add_regions_clauses_to_solver(pre_regions);
        auto SMs = new set<set<Region *> *>(); //set of SMs, each SM is a set of regions
        //int last_uncovered_regions = uncovered_regions->size();

        excitation_closure = false;
        while(!excitation_closure) {
            minValueToCheck = startingMinValueToCheck;
            PBConfig config = make_shared<PBConfigClass>();
            VectorClauseDatabase formula(config);
            VectorClauseDatabase last_sat_formula(config);
            PB2CNF pb2cnf(config);
            AuxVarManager auxvars(numRegions + 1);


            //cout << "pre regions size " << all_pre_regions->size() <<endl;
            vector<WeightedLit> literals_from_regions = {};

            //modificare il ciclo per identificare se le regioni sono ostate già usate aumentando il peso per quelle non usate (magari aumentare il peso solo per quelle irridondanti non usate)
            for (int i = 1; i <= numRegions; i++) {
                if(used_regions.find(i) == used_regions.end()){
                    literals_from_regions.emplace_back(i, 2);
                    if(decomposition_debug)
                        cout << "Added the region number " << i << " with value " << 2 << endl;
                }
                else{
                    literals_from_regions.emplace_back(i, 1);
                    if(decomposition_debug)
                        cout << "Added the region number " << i << " with value " << 1 << endl;
                }
            }

            //speedup fix: at the firs iteration all regions are not used and have weight=2
            if(used_regions.empty()){
                minValueToCheck*=2;
            }

            IncPBConstraint constraint(literals_from_regions, GEQ,
                                       minValueToCheck); //imposto che la somma delle variabili deve essere maggiore o uguale a 1
            pb2cnf.encodeIncInital(constraint, formula, auxvars);

            /*cout << "Clauses inequality" << endl;
            formula.printFormula(cout);*/

            for (auto clause: *clauses) {
                formula.addClause(*clause);
            }
            /*cout << "clauses of the overlapping regions + clauses inequality" << endl;
            cout << "=======================" << endl;
            formula.printFormula(cout);
            cout << "=======================" << endl;*/

            Minisat::Solver solver;

            bool sat;
            vec<lbool> true_model;

            //iteration in the search of a correct assignment for a single SM
            do {
                last_sat_formula.clearDatabase();
                last_sat_formula.addClauses(formula.getClauses());
                constraint.encodeNewGeq(minValueToCheck, formula, auxvars);
                int num_clauses_formula = last_sat_formula.getClauses().size();
                string dimacs_file = convert_to_dimacs(file, auxvars.getBiggestReturnedAuxVar(), num_clauses_formula,
                                                       last_sat_formula.getClauses(), nullptr);
                sat = check_sat_formula_from_dimacs(solver, dimacs_file);
                if (sat) {
                    if (decomposition_debug) {
                        //cout << "----------" << endl;
                        cout << "SAT with value " << minValueToCheck << endl;
                        //cout << "formula: " << endl;
                        //formula.printFormula(cout);
                        cout << "Model: ";
                        for (int i = 0; i < solver.nVars(); i++) {
                            if (solver.model[i] != l_Undef) {
                                fprintf(stdout, "%s%s%d", (i == 0) ? "" : " ", (solver.model[i] == l_True) ? "" : "-",
                                        i + 1);
                            }
                        }
                        cout << endl;
                    }
                    true_model.clear(true);
                    for (auto val: solver.model) {
                        true_model.push(val);
                    }
                } else {
                    break;
                }
                minValueToCheck++;
            } while (sat);
            cout << "UNSAT with value " << minValueToCheck << endl;
            //cout << "formula: " << endl;
            //formula.printFormula(cout);


            FILE *res = stdout;

            set<Region *> *SM;
            SM = new set<Region *>();
            /*if(decomposition_debug)
                fprintf(res, "Last SAT with: ");*/
            for (int i = 0; i < solver.nVars(); i++) {
                if (true_model[i] != l_Undef) {
                    /*if(decomposition_debug)
                        fprintf(res, "%s%s%d", (i == 0) ? "" : " ", (true_model[i] == l_True) ? "" : "-", i + 1);*/
                    if (true_model[i] == l_True) {
                        if (i < numRegions) {
                            add_region_to_SM(SM, (*aliases_region_pointer)[i + 1]);
                        }
                    }
                }
            }
            /*if(decomposition_debug)
                fprintf(res, " 0\n");*/

            //update the used regions
            for (auto region: *SM) {
                used_regions.insert((*aliases_region_pointer_inverted)[region]);
            }

            //update the set of SMs
            SMs->insert(SM);

            //control excitation closure used regions

            //update of used regions map
            for(auto reg: used_regions){
                for(auto rec: *pre_regions){
                    if(rec.second->find((*aliases_region_pointer)[reg]) != rec.second->end()){
                        (*used_regions_map)[rec.first]->insert((*aliases_region_pointer)[reg]);
                    }
                }
            }

            excitation_closure = is_excitation_closed(used_regions_map, new_ER);

            //excitation_closure = events->empty();
            if(decomposition_debug) {
                if (excitation_closure) {
                    cout << "EXCITATION CLOSURE OK!!!" << endl;
                } else {
                    cout << "NO EXCITATION CLOSURE" << endl;
                }
            }
        }

        if(decomposition_debug) {
            //update of used regions map
            for (auto reg: used_regions) {
                for (auto rec: *pre_regions) {
                    if (rec.second->find((*aliases_region_pointer)[reg]) != rec.second->end()) {
                        (*used_regions_map)[rec.first]->insert((*aliases_region_pointer)[reg]);
                    }
                }
            }

            excitation_closure = is_excitation_closed(used_regions_map, new_ER);

            if (excitation_closure) {
                cout << "EXCITATION CLOSURE OK!!!" << endl;
            } else {
                cout << "NO EXCITATION CLOSURE" << endl;
            }
        }

        auto t_decomposition = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

        if(decomposition_debug)
            cout << "=======================[ GREEDY REMOVAL OF SMs CHECKING EC ]================" << endl;

        /*auto new_SMs = new set<set<Region *> *>();
        for(auto set: *SMs){
            new_SMs->insert(set);
        }*/
        //bool changed = true;

        //set<int> new_used_regions;
        /*for(auto SM: *new_SMs){
            for (auto region: *SM) {
                new_used_regions.insert((*aliases_region_pointer_inverted)[region]);
            }
        }*/

        set<set<Region *> *> SMs_to_remove;

        for(auto SM: *SMs) {
            if (decomposition_debug) {
                cout << "check if can rermove the SM: " << endl;
                println(*SM);
            }
            auto tmp_SMs = new set < set < Region * > * > ();
            //tmp_SM prende tutto tranne SM
            //std::copy_n ( SMs->begin(), SMs->size(), tmp_SMs->begin());
            for (auto set: *SMs) {
                if (set != SM)
                    tmp_SMs->insert(set);
            }
            set<int> new_used_regions_tmp;
            for (auto tmp_SM: *tmp_SMs) {
                for (auto region: *tmp_SM) {
                    new_used_regions_tmp.insert((*aliases_region_pointer_inverted)[region]);
                }
            }
            auto new_used_regions_map_tmp = new map<int, set < Region * > * > ();
            for (auto rec: *pre_regions) {
                (*new_used_regions_map_tmp)[rec.first] = new set < Region * > ();
            }

            for (auto reg: new_used_regions_tmp) {
                for (auto rec: *pre_regions) {
                    if (rec.second->find((*aliases_region_pointer)[reg]) != rec.second->end()) {
                        (*new_used_regions_map_tmp)[rec.first]->insert((*aliases_region_pointer)[reg]);
                    }
                }
            }

            excitation_closure = is_excitation_closed(new_used_regions_map_tmp, new_ER);
            for (auto rec: *new_used_regions_map_tmp) {
                delete rec.second;
            }
            delete new_used_regions_map_tmp;
            if (excitation_closure) {
                SMs_to_remove.insert(SM);
                if (decomposition_debug) {
                    cout << "ok without SM" << endl;
                    println(*SM);
                }
                //delete new_SMs;

                //new_SMs = tmp_SMs;
                //new_used_regions = new_used_regions_tmp;
                //removal of the SM
                //changed = true;
                //cout << "new SM set size:" << new_SMs->size() << endl;
                //serve per l'iteratore delle SM
                //break;
            }
            delete tmp_SMs;
        }

        if(decomposition_debug) {
            cout << "SM candidate for removal: " << endl;
            for(auto reg: SMs_to_remove){
                cout << "SM:" <<endl;
                println(*reg);
            }
        }

        //todo: this part will be modified with the search of max ind set of SMs to remove
        bool removed = true;
        while(removed){
            removed = false;
            for(auto SM: SMs_to_remove){
                auto tmp_SMs = new set < set < Region * > * > ();
                //tmp_SM prende tutto tranne SM
                for (auto set: *SMs) {
                    if (set != SM)
                        tmp_SMs->insert(set);
                }

                set<int> new_used_regions_tmp;
                for (auto tmp_SM: *tmp_SMs) {
                    for (auto region: *tmp_SM) {
                        new_used_regions_tmp.insert((*aliases_region_pointer_inverted)[region]);
                    }
                }
                auto new_used_regions_map_tmp = new map<int, set < Region * > * > ();
                for (auto rec: *pre_regions) {
                    (*new_used_regions_map_tmp)[rec.first] = new set < Region * > ();
                }

                for (auto reg: new_used_regions_tmp) {
                    for (auto rec: *pre_regions) {
                        if (rec.second->find((*aliases_region_pointer)[reg]) != rec.second->end()) {
                            (*new_used_regions_map_tmp)[rec.first]->insert((*aliases_region_pointer)[reg]);
                        }
                    }
                }

                excitation_closure = is_excitation_closed(new_used_regions_map_tmp, new_ER);
                for (auto rec: *new_used_regions_map_tmp) {
                    delete rec.second;
                }
                delete new_used_regions_map_tmp;
                if (excitation_closure) {
                    if (decomposition_debug) {
                        cout << "removed SM" << endl;
                        println(*SM);
                    }
                    SMs->erase(SM);
                    SMs_to_remove.erase(SM);
                    //removal of the SM
                    removed = true;
                    //cout << "new SM set size:" << new_SMs->size() << endl;
                    //serve per l'iteratore delle SM
                    break;
                }
                delete tmp_SMs;
            }
        }



        /*if(decomposition_debug)
            cout << "=======================[ MERGING MODULE ON RESULT OF DECOMPOSITION ]================" << endl;

        Merging_Minimal_Preregions_module *merging_module = nullptr;

        merging_module = new Merging_Minimal_Preregions_module(used_regions_map, nullptr, new_ER);

        //auto merged_map = merging_module->get_merged_preregions_map();*/





        if(decomposition_debug)
            cout << "=======================[ CREATION OF A .dot FILE FOR EACH SM / S-COMPONENT  ]================" << endl;
        //CREATION OF THE TRANSITIONS BETWEEN STATES OF THE SM
        //cout << "pre-regions" << endl;
        //print(*pprg->get_pre_regions());
        int counter = 0;
        for(auto SM: *SMs){
            counter++;
            if(decomposition_debug) {
                cout << "SM " << counter << endl;
                for(auto reg: *SM){
                    println(*reg);
                }
            }
            auto SM_pre_regions_map = new map<int, set<Region *> *>();
            for(auto rec: *pprg->get_pre_regions()){
                for(auto reg: *rec.second){
                    if(SM->find(reg)!= SM->end()){
                        //cout << "FOUND" << endl;
                        if(SM_pre_regions_map->find(rec.first) == SM_pre_regions_map->end()){
                            (*SM_pre_regions_map)[rec.first] = new set<Region *>;
                        }
                        (*SM_pre_regions_map)[rec.first]->insert(reg);
                    }
                }
            }

            auto post_regions_SM = pprg->create_post_regions_for_SM(SM_pre_regions_map);
            string SM_name = file;
            //generic cut for .ts file path and other extensions
            int lower = 0;
            for (int i = static_cast<int>(SM_name.size() - 1); i > 0; i--) {
                if (SM_name[i] == '.') {
                    lower = i;
                    break;
                }
            }
            SM_name = SM_name.substr(0, lower);
            SM_name += "_SM_"+to_string(counter)+".g";
            print_pn_dot_file(SM_pre_regions_map, post_regions_SM, aliases, SM_name);
            for(auto rec: *post_regions_SM){
                delete rec.second;
            }
            delete post_regions_SM;
            for(auto rec: *SM_pre_regions_map){
                delete rec.second;
            }
            delete SM_pre_regions_map;
        }

        //===========FREE===========
        for(auto rec: *used_regions_map){
            delete rec.second;
        }
        delete used_regions_map;
        for(auto SM: *SMs){
            delete SM;
        }
        delete SMs;
        //delete rg;
        delete clauses;
        delete regions_set;
        delete regions_sorted;

        printf("\nTime region gen: %.5fs\n", t_region_gen);
        printf("Time splitting: %.5fs\n", t_splitting);
        printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
        printf("Time decomposition: %.5fs\n", t_decomposition);
        printf("Total time: %.5fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);

    } else {
        tStart_partial = clock();
        // Start of module: search of the irredundant set of regions
        auto pn_module = new Place_irredundant_pn_creation_module(pre_regions, new_ER);
        auto t_irred = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;
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

        //todo: aggiungere gli alias invertiti nell'output con l'estensione .g
        print_pn_dot_file(merged_map, post_regions, aliases, file);

        // cout << "fine ricerca " << endl;

        //===============FREE ============
        delete pn_module;
        delete merging_module;

        printf("\nTime region gen: %.5fs\n", t_region_gen);
        printf("Time splitting: %.5fs\n", t_splitting);
        printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
        printf("Time essential+irredundant: %.5fs\n", t_irred);
        printf("Time merge: %.5fs\n", t_merge);
        printf("Total time: %.5fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);
    }

    // ============== COMMON BRANCH FREE ============

    delete aliases;

    // dealloco regions e tutti i suoi vettori
    for (auto record : *regions) {
        delete record.second;
    }
    delete regions;
    delete vector_regions;


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