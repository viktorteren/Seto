
#include "Label_splitting_module.h"
#include "Regions_generator.h"
#include "Place_irredundant_pn_creation_module.h"
#include "Pre_and_post_regions_generator.h"

int main() {
    bool first;
    TS_parser::parse();
    int pos = 0;
    vector<Region>* regions_vec;


    Region_generator *rg = new Region_generator();
    map<int, vector<Region> *>* regions= rg->generate();

    Label_splitting_module *ls=new Label_splitting_module(regions,rg->get_ER_set());

    bool excitation_closure=ls->is_excitation_closed();
    if(!excitation_closure) {
        cout<<" not exitation closed " <<endl;
        regions_vec = ls->do_label_splitting(rg->get_middle_set_of_states(),rg->get_number_of_bad_events());
    }
    //else{ //TODO: test questo ramo se concatena giusto
    //creo il vettore dalla mappa di regioni per creare le preregioni!!
    regions_vec=new vector<Region>;
    for(auto record:*regions){
        regions_vec->insert(regions_vec->end(), record.second->begin(),record.second->end());
    }
    // }

    //todo: controllare prima di questo punto: il vettore di regioni regions_vec contiene duplicati con indirizzi diversi
	//todo: temporaneamente utilizzo la conversione di regions al posto di regions_vec: al completamento di label splittiing sarà da rimettere
	vector<Region> temp_regions = copy_map_to_vector(regions);
    Pre_and_post_regions_generator *pprg = new Pre_and_post_regions_generator(&temp_regions);
    map<int, set<Region*> *> * pre_regions = pprg->get_pre_regions();
    map<int, set<Region*> *> * post_regions = pprg->get_post_regions();


    //Inizio modulo: ricerca di set irridondanti di regioni
    Place_irredundant_pn_creation_module *pn_module = new Place_irredundant_pn_creation_module(pre_regions, post_regions);

    //cout << "fine ricerca " << endl;

    delete ls;
    //delete prg;
    delete rg; //dealloco tutto tranne il campo pre_regions

    //dealloco regions e tutti i suoi vettori
    for(auto record:*regions){
        delete record.second;
    }
    delete regions;


}
