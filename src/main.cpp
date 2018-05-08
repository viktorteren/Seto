
#include "../include/Label_splitting_module.h"
#include "../include/Regions_generator.h"
#include "../include/Place_irredundant_pn_creation_module.h"
#include "../include/Pre_and_post_regions_generator.h"

int main(int argc, char** argv) {
	bool first;
	vector <string> args (argv, argv + argc);
    if(argc == 1){
    	//default input
		TS_parser::parse("../test/input.dot");
    }
    else if (argc == 2){
        //cout << args[1] << endl;
		TS_parser::parse(args[1]);
    }
    else{
    	cout << "Too many input arguments" << endl;
    	exit(0);
    }

    int pos = 0;
    vector<Region>* candidate_regions;

    auto rg = new Region_generator();
    map<int, vector<Region> *>* regions= rg->generate();

    vector<Region>* vector_regions = copy_map_to_vector(regions);

    auto ls=new Label_splitting_module(regions,rg->get_ER_set());

    Pre_and_post_regions_generator *pprg;

    bool excitation_closure;

    vector<int>* events_not_satify_EC=ls->is_excitation_closed();

	excitation_closure= events_not_satify_EC->empty();
    cout<<"EC*************"<< excitation_closure <<endl;

    if(!excitation_closure) {
        cout<<" not exitation closed " <<endl;
        candidate_regions = ls->do_label_splitting(rg->get_middle_set_of_states(),rg->get_number_of_bad_events(),events_not_satify_EC);
        cout<<"___________label splitting ok"<<endl;
        //chiamo il pre_region genertor passandogli anche le regioni nuove
       // pprg = new Pre_and_post_regions_generator(vector_regions,candidate_regions);
        pprg = new Pre_and_post_regions_generator(vector_regions, candidate_regions);
	    delete candidate_regions;
    }
    else{
        pprg = new Pre_and_post_regions_generator(vector_regions);
    }
    delete events_not_satify_EC;

    map<int, set<Region*> *> * pre_regions = pprg->get_pre_regions();
    map<int, set<Region*> *> * post_regions = pprg->get_post_regions();

    delete ls;
    delete rg;

    //Inizio modulo: ricerca di set irridondanti di regioni
    auto pn_module = new Place_irredundant_pn_creation_module(pre_regions, post_regions);


    auto essential_regions=pn_module->get_essential_regions();
    map<int,set<Region*>>* irredundant_regions= nullptr;

    if(pn_module->get_irredundant_regions()!= nullptr) {
        irredundant_regions = pn_module->get_irredundant_regions();
        print_PN(essential_regions,irredundant_regions);
    }
    else {
        print_PN(essential_regions, nullptr);

    }

    //dealloco regions e tutti i suoi vettori
    for(auto record:*regions){
        delete record.second;
    }
    delete regions;

    delete vector_regions;

    //cout << "fine ricerca " << endl;

    delete pn_module;
    delete pprg;

}
