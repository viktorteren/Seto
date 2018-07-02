
#include "../include/Label_splitting_module.h"
#include "../include/Merging_Minimal_Preregions_module.h"
#include "../include/Place_irredundant_pn_creation_module.h"
#include "../include/Pre_and_post_regions_generator.h"
#include "../include/Regions_generator.h"
#include <ctime>

int main(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    string file;
    if (argc == 1) {
        // default input
        file = "../test/input14.ts";
    } else if (argc == 2) {
        file = args[1];
    } else {
        cout << "Too many input arguments" << endl;
        exit(0);
    }
    TS_parser::parse(file);

    map<int,pair<int,Region*>*>* candidate_regions;
    map<int, set<Region *> *> *pre_regions;
    Label_splitting_module *ls;
    map<int, ER> *new_ER;
    vector<Region> *vector_regions;
    map<int, vector<Region> *> *regions;


    double t_pre_region_gen=0.0;
    double t_region_gen=0.0;
    double t_splitting=0.0;
    int number_of_events;
    //int c=0;
    auto aliases= new map<int,int>();
    int contatore;

    clock_t tStart = clock();

    auto tStart_partial = clock();

    int vec_size=-1;

    bool excitation_closure=false;
    double dim_reg;
    do {
        number_of_events = static_cast<int>(ts_map->size());
        //cout << "number_of_events: " << number_of_events << endl;
        //cout << "num_events: " << num_events  << endl;
        //cout << "contatore = " << contatore << " ts_map->size() = " << ts_map->size() << endl;
        auto rg = new Region_generator(number_of_events);
        regions = rg->generate();
        cout << "DEBUG: regioni dopo generate " << endl;
        for (auto rec: *regions) {
            cout << "evento: " << rec.first << endl;
            for (auto reg: *rec.second) {
                println(reg);
            }
        }
        vector_regions = copy_map_to_vector(regions);

        // cout << "------------------------------------------------------------ "
        //       "DELETING OF NON MINIMAL REGIONS "
        //     "-------------------------------------------"
        //<< endl;
        vector<Region>::iterator it;
        for (it = vector_regions->begin(); it < vector_regions->end(); ++it) {
            Region *region = &(*it);
            rg->remove_bigger_regions(*region, vector_regions);
        }

        /*cout << "region dopo l'eliminazioni delle regioni più grandi" << endl;
        for (auto r: *vector_regions) {
            cout << "reg: ";
            println(r);
        }

        cout<<"regioni"<<endl;
        for (auto rec: *regions) {
            cout << "event" << rec.first << endl;
            for (auto r: *rec.second) {
                cout << "reg: ";
                println(r);
            }
        }*/

        int cont = 0;
        double somma = 0;
        for (const auto &reg: *vector_regions) {
            cont++;
            somma += reg.size();
        }

        cout << "media: " << (somma / cont) << endl;

        cout << "numero regioni: " << vector_regions->size() << endl;
        num_events_after_splitting = static_cast<int>(vector_regions->size());

        //messo per non andare in loop ma non sarei exit closure----da togliere
        /*if(vec_size==vector_regions->size() && dim_reg==(somma / cont)) break;
        vec_size=vector_regions->size();
        dim_reg=(somma / cont);*/


        t_region_gen = t_region_gen + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

        tStart_partial = clock();

        ls = new Label_splitting_module(regions, rg->get_ER_set(),rg->get_middle_set_of_states());

        set<int> *events = ls->is_excitation_closed();

        excitation_closure = events->empty();
        //cout << "EC*************" << excitation_closure << endl;

        set<int> *events_not_satify_EC = nullptr;

        if (!excitation_closure) {
            //cout << " not exitation closed " << endl;
            candidate_regions = ls->do_label_splitting( rg->get_number_of_bad_events(), events);
            //cout << "___________label splitting ok" << endl;
            ls->split_ts_map_2(candidate_regions, aliases, rg->get_violations_event(),rg->get_violations_trans());

            /*events_not_satify_EC = new set<int>();
            auto pairs = rg->get_trees_init();

            for (auto el : *events) {
                events_not_satify_EC->insert(pairs->at(el));
            }*/

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
            //break;
            //pprg = new Pre_and_post_regions_generator(vector_regions);
            new_ER = rg->get_ER_set();
        }

        delete events;
        delete events_not_satify_EC;

        delete rg;
        //number_of_events++;
        //cout << "ho finito" << endl;

        t_splitting = t_splitting + (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

    }//end prova do while
    while (!excitation_closure);
    cout << "uscito dal ciclo while" << endl;


    /*cout<<"ts map debug:"<<endl;
    for(auto tr: *ts_map){
        cout<<"evento "<< tr.first<<endl;
        for(auto r: tr.second){
            cout<<r->first<< "->"<< r->second<<endl;
        }
    }*/


    tStart_partial = clock();
    auto pprg = new Pre_and_post_regions_generator(vector_regions);
    pre_regions = pprg->get_pre_regions();

    t_pre_region_gen = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

    tStart_partial = clock();
    // Inizio modulo: ricerca di set irridondanti di regioni
    auto pn_module = new Place_irredundant_pn_creation_module(pre_regions, new_ER);
    auto t_irred = (double) (clock() - tStart_partial) / CLOCKS_PER_SEC;

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


        merging_module = new Merging_Minimal_Preregions_module(
                essential_regions, irredundant_regions, new_ER);
        // print_PN(essential_regions,irredundant_regions);
    } else {
        merging_module = new Merging_Minimal_Preregions_module(essential_regions,
                                                               nullptr, new_ER);
        // print_PN(essential_regions, nullptr);
    }

    auto merged_map = merging_module->get_merged_preregions_map();

    cout << "not merged pre-regions: " << endl;
    print(*merging_module->get_total_preregions_map());


    if (merged_map == nullptr) {
        merged_map = merging_module->get_total_preregions_map();
    }

    //cout << "merged pre-regions: " << endl;
    //print(*merging_module->get_merged_preregions_map());



    /*cout << "merged map nel main: " << endl;
    print(*merged_map);*/

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
    delete ls;
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

    for (auto el : *ts_map) {
        for (auto p : el.second) {
            delete p;
        }
    }
    delete ts_map;

    printf("Time total: %.5fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);
    printf("Time region gen: %.5fs\n", t_region_gen);
    printf("Time splitting: %.5fs\n", t_splitting);
    printf("Time pre region gen: %.5fs\n", t_pre_region_gen);
    printf("Time essential+irredundant: %.5fs\n", t_irred);
    printf("Time merge: %.5fs\n", t_merge);
}