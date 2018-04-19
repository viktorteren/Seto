
#include <fstream>

#include <vector>

#include <algorithm>
#include <iostream>
#include <cassert>

#include "TS_parser.h"
#include "Label_splitting_module.h"
#include "Essential_region_search.h"
#include "Minimal_pre_region_generator.h"
#include "Irredundant_sets_creation_module.h"

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
        Minimal_pre_region_generator *mg = new Minimal_pre_region_generator();
        //Minimal_pre_region_generator::Minimal_pre_region_generator(num_stati,num_eventi);
        map<int, vector<Region*> *>* pre_regions= mg->generate();

        Label_splitting_module *ls=new Label_splitting_module(pre_regions,mg->get_ER_set());

        bool excitation_closure=ls->is_excitation_closed();
        if(!excitation_closure)
            ls->do_label_splitting(mg->get_middle_set_of_states());

        cout<< "Exitation closed: " << excitation_closure;

        //Inizio modulo: ricerca di set irridondanti di regioni
		Irredundant_sets_creation_module *is_module = new Irredundant_sets_creation_module(pre_regions);

        //cout << "fine ricerca " << endl;

        delete ls;
        delete mg; //dealloco tutto tranne il campo pre_regions

        //dealloco pre_regions e tutti i suoi vettori
        for(auto record:*pre_regions){
            delete record.second;
        }
        delete pre_regions;


    }
