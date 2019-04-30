//#include <errno.h>
//#include <signal.h>
//#include "minisat/utils/System.h"
//#include "minisat/utils/ParseUtils.h"
//#include "minisat/utils/Options.h"
//#include "minisat/core/Dimacs.h"
#include "minisat/core/Solver.h"
#include "../include/Label_splitting_module.h"
#include "../include/Merging_Minimal_Preregions_module.h"
#include "../include/Regions_generator.h"
#include "../include/Pre_and_post_regions_generator.h"
#include "../include/Place_irredundant_pn_creation_module.h"

using namespace Minisat;

int main(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    string file;
    if (argc == 1) {
        // default input
        print_step_by_step=false;
        file = "../input/etff.g";
        //cout << "sono qui" << endl;
    } else if (argc == 2) {
        file = args[1];
        cout << file << endl;
    }
    else if (argc == 3) {
        file = args[1];
        cout << file << endl;
        if( args[2]=="S") {
            print_step_by_step = true;
            print_step_by_step_debug = false;
            decomposition = false;
        }
        else if( args[2]=="D") {
            print_step_by_step = true;
            print_step_by_step_debug = true;
            decomposition = false;
        }
        else if( args[2]=="M") {
            print_step_by_step = true; //todo: diventerà false alla fine ell'implementazione
            print_step_by_step_debug = false;
            decomposition = true;
        }
        else{
            print_step_by_step = false;
            print_step_by_step_debug = false;
            decomposition = false;
        }
    }
    else {
        cout << "Wrong number of input arguments" << endl;
        exit(0);
    }

    TS_parser::parse(file);

    if(print_step_by_step_debug) {
        cout << "TS" << endl;
        for (auto tr: *ts_map) {
            cout << "event " << tr.first << endl;
            for (auto r: tr.second) {
                cout << r->first << "->" << r->second << endl;
            }
        }
    }

    map<int,pair<int,Region*>*>* candidate_regions;
    map<int, set<Region *> *> *pre_regions;
    Label_splitting_module *ls;
    Pre_and_post_regions_generator *pprg=nullptr;
    map<int, ER> *new_ER;
    vector<Region> *vector_regions;
    map<int, vector<Region> *> *regions;


    double t_pre_region_gen=0.0;
    double t_region_gen=0.0;
    double t_splitting=0.0;
    int number_of_events;
    //int c=0;
    auto aliases= new map<int,int>();

    clock_t tStart = clock();

    auto tStart_partial = clock();

    int vec_size=-1;
    int num_split=0;

    bool excitation_closure=false;
    double dim_reg;
    num_events_after_splitting=static_cast<int>(ts_map->size());
    do {
        number_of_events = static_cast<int>(ts_map->size());
        //cout << "number_of_events: " << number_of_events << endl;
        //cout << "num_events: " << num_events  << endl;
        //cout << "contatore = " << contatore << " ts_map->size() = " << ts_map->size() << endl;
        auto rg = new Region_generator(number_of_events);
        regions = rg->generate();
        vector_regions = copy_map_to_vector(regions);

        if(print_step_by_step){
        cout << "Regions: " << endl;
        for (auto reg: *vector_regions) {
            println(reg);
        }
        cout << "" << endl;}

        // cout << "------------------------------------------------------------ "
        //       "DELETING OF NON MINIMAL REGIONS "
        //     "-------------------------------------------"
        //<< endl;
        vector<Region>::iterator it;
        for (it = vector_regions->begin(); it < vector_regions->end(); ++it) {
            Region *region = &(*it);
            rg->remove_bigger_regions(*region, vector_regions);
        }


        if(print_step_by_step){
            cout << "Minimal regions: " << endl;
            for (auto r: *vector_regions) {
                println(r);
        }
        cout << "" << endl;}

        /*cout<<"regioni"<<endl;
        for (auto rec: *regions) {
            cout << "event" << rec.first << endl;
            for (auto r: *rec.second) {
                cout << "reg: ";
                println(r);
            }
        }*/




        num_events_after_splitting = static_cast<int>(ts_map->size());


        t_region_gen = t_region_gen + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;


        tStart_partial = clock();
        delete pprg;
        pprg = new Pre_and_post_regions_generator(vector_regions);
        pre_regions = pprg->get_pre_regions();

        if(print_step_by_step) {
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

        ls = new Label_splitting_module(pre_regions, rg->get_ER_set(),rg->get_middle_set_of_states());

        set<int> *events = ls->is_excitation_closed();

        excitation_closure = events->empty();
        //cout << "EC*************" << excitation_closure << endl;

        set<int> *events_not_satify_EC = nullptr;

        if (!excitation_closure) {
            num_split++;
            candidate_regions = ls->candidate_search(rg->get_number_of_bad_events(), events);
            if(print_step_by_step){cout << "Splitting of the labels: ";}
            if(!pre_regions->empty())
                ls->split_ts_map(candidate_regions, aliases, rg->get_violations_event(), rg->get_violations_trans(),
                                 nullptr);
            else ls->split_ts_map(candidate_regions, aliases, rg->get_violations_event(), rg->get_violations_trans(),regions);

            map<int,pair<int,Region*>*>::iterator it2;
            for(it2=candidate_regions->begin();it2!=candidate_regions->end();++it2) {
               delete it2->second;
            }
            delete candidate_regions;

            new_ER = rg->get_ER_set();

            delete vector_regions;
            delete ls;

            for (auto el : *new_ER)
                delete el.second;
            delete new_ER;

            for(auto reg_vec: *regions) {
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

    if(print_step_by_step) {
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

    print_ts_dot_file(file,aliases);

    /*cout<<"ECTS:"<<endl;
    for(auto tr: *ts_map){
        cout<<"evento "<< tr.first<<endl;
        for(auto r: tr.second){
            cout<<r->first<< "->"<< r->second<<endl;
        }
    }
    cout << "" << endl;*/

    tStart_partial = clock();
    // Inizio modulo: ricerca di set irridondanti di regioni
    auto pn_module = new Place_irredundant_pn_creation_module(pre_regions, new_ER);
    auto t_irred = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

    if(decomposition) {
        cout << "============================[DECOMPOSITION]===================" << endl;
        //TODO:qui si può effettuare la decomposizione della TS
        /*
         * creo un'istanza del solver
         * trasformo le regioni essenziali+irridondanti(dovrei unire le 2 mappe) in clausole: una clausola per ogni regione con un solo
         * letterale e una clausola per ogni stato per vedere quali regioni si sovrappongono
         * chiamo il metodo addClause_ dell'istanza Solver per ogni regione (sia essenziale che non) e in più le clausole per i collegamenti???
         * salvo le regioni/stati da coprire
         * eseguo il solver
         * ricavo il risultato dal solver con una clausola nuova
         * set di set di regioni prende la nuova clausola ritrasformata in insieme di regioni //ogni set è una SM
         * ricreo le nuove clausole con la negazione della nuova aggiunta
         * termino quando la copertura è completa
         */
        Solver s;
        aliases_region_pointer = new map<int, Region*>();
        max_alias_decomp = 0;
        map<int, set<Region *> *> *merged_map = Utilities::merge_2_maps(pn_module->get_essential_regions(), pn_module->get_irredundant_regions());
        set<vec<Lit>>* clauses = nullptr;
        clauses = transform_regions_to_clauses(merged_map);

        //================== FREE ====================
        delete aliases_region_pointer;
        delete merged_map;
        delete aliases;

        // dealloco regions e tutti i suoi vettori
        for (auto record : *regions) {
            delete record.second;
        }
        delete regions;
        delete vector_regions;

        // cout << "fine ricerca " << endl;

        delete pn_module;
        delete pprg;

        for (auto el : *ts_map) {
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
    else{
        tStart_partial = clock();

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
        delete aliases;

        // dealloco regions e tutti i suoi vettori
        for (auto record : *regions) {
            delete record.second;
        }
        delete regions;
        delete vector_regions;

        // cout << "fine ricerca " << endl;

        delete pn_module;
        delete pprg;

        delete merging_module;

        for (const auto& el : *ts_map) {
            for (auto p : el.second) {
                delete p;
            }
        }
        delete ts_map;
        delete aliases_map_number_name;
        delete aliases_map_name_number;
        delete aliases_map_state_number_name;
        delete aliases_map_state_name_number;


        printf("Time total: %.5fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);
        printf("Time region gen: %.5fs\n", t_region_gen);
        printf("Time splitting: %.5fs\n", t_splitting);
        printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
        printf("Time essential+irredundant: %.5fs\n", t_irred);
        printf("Time merge: %.5fs\n", t_merge);
    }


}