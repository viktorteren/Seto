
#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <cassert>

#include "TS_parser.h"
#include "Label_splitting_module.h"
#include "Essential_region_search.h"
#include "Regions_generator.h"
#include "Minimal_and_irredundant_pn_creation_module.h"
#include "Pre_and_post_regions_generator.h"

typedef std::pair<int, int> Edge;

    using namespace std;
//using namespace boost;
    //using namespace pre_region_gen;
   // using namespace essential_regions;

//map: evento -> lista di coppie: (srcId, dstId)
    typedef vector<Edge> Lista_archi;
    typedef std::map<int, Lista_archi> My_Map;

/*typedef property<edge_name_t, int> event;
typedef adjacency_list<mapS, vecS, undirectedS,no_property,event> Graph;*/


//typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;


// declare a graph object
/*Graph g(0);
Vertex* vertex_array;*/


    int main() {
        bool first;
        TS_parser::parse();
        int pos = 0;
        vector<Region>* regions_vec;


        Region_generator *rg = new Region_generator();
        //Region_generator::Region_generator(num_stati,num_eventi);
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

        Pre_and_post_regions_generator *pprg = new Pre_and_post_regions_generator(regions_vec);
		map<int, vector<Region*> *> * pre_regions = pprg->get_pre_regions();
		map<int, vector<Region*> *> * post_regions = pprg->get_post_regions();


        //Inizio modulo: ricerca di set irridondanti di regioni
		Minimal_and_irredundant_pn_creation_module *pn_module = new Minimal_and_irredundant_pn_creation_module(pre_regions, post_regions);

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
