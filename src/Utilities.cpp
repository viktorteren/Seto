//
// Created by ciuchino on 18/04/18.
//

#include "../include/Utilities.h"

namespace Utilities {
    //Region = set<int> ->ritorna un insieme di stati
    set<int> *regions_union(vector<Region *> *vec){
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region: *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int> *regions_union(set<Region *> *vec){
        //cout << "region union" << endl;
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region: *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                //cout << "Stato: " << *it << endl;
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int> *regions_union(Region *first, Region *second){
        auto all_states = new Region();
        int size;
        Region::iterator it;
        it = first->begin();
        size = static_cast<int>(first->size());
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }
        it = second->begin();
        size = static_cast<int>(second->size());
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }

        return all_states;
    }

    map<int, set<int> *> *do_regions_intersection(map<int, vector<Region> *> *regions){

        auto pre_regions_intersection = new map<int, set<int> *>;

        std::vector<Region>::iterator it;
        bool state_in_intersecton=true;

        //per ogni evento
        for (auto item: *regions) {
            (*pre_regions_intersection)[item.first]=new Region();
            for (auto state: (*item.second)[0]) {
                state_in_intersecton=true;
                for(auto set:*item.second){
                    if(set.find(state)==set.end()) {//non l'ho trovato
                        state_in_intersecton = false;
                        break;
                    }
                }
                if(state_in_intersecton){
                    pre_regions_intersection->at(item.first)->insert(state);
                }
            }
        }
        cout << "intersezione****************" << endl;
        for(auto el:*pre_regions_intersection){
            cout<<"event "<<el.first<<endl;
            println(*el.second);
        }

        return pre_regions_intersection;
    }


    set<int>* regions_intersection(set<Region*>* regions){

        auto pre_regions_intersection= new set<int>();
        bool state_in_intersecton;

        for (auto state: **regions->begin()) {
            state_in_intersecton=true;
            for(auto s: *regions){
                if(s->find(state)==s->end()) {//non l'ho trovato
                    state_in_intersecton = false;
                    break;
                }
            }
            if(state_in_intersecton){
                pre_regions_intersection->insert(state);
            }
        }

        return pre_regions_intersection;
    }

    set<int> *regions_intersection(Region *first, Region *second){
        auto intersection = new set<int>();
        for (auto state: *first) {
            if (second->find(state) != second->end()) {//trovo lo stato (appartiene a entrambe)
                intersection->insert(state);
            }
        }
        return intersection;
    }

    void print(Region &region){
        Region::iterator it;
        int pos=0;
        int size=region.size();
        //for (auto state : region) {
        for(it=region.begin();it!=region.end();++it){
            auto state=*it;
            pos++;
            cout << state;
            if(pos!=size){
                cout<<",";
            }
        }
    }

    void println(Region &region){
        print(region);
        cout << endl;
    }

    void print_place(int pos,Region &region){
        cout<<"r"<<pos<<": { ";
        print(region);
        cout<<" } ";
    }

    void print_transactions(){
       cout<<"Transazioni: "<<endl;
        for(int i=0;i<num_transactions;i++){
            cout<<"t" <<i;
            if(i!=num_transactions-1)
                cout<<",";
        }

    }


    void print_PN(map<int, set<Region*>> * essential_regions,map<int, set<Region*>> * irredundant_regions) {
        int pos=0;


        //todo:ci sono regioni doppie perchè ho mappa -> non ci sono più regioni doppie con il set ma devo tenere traccia del numero di regione con la regione stessa
		set<Region *> *unrepeated_regions = new set<Region *>();
        cout<<"Posti: "<< endl;
		for(auto record:*essential_regions){
			for(auto reg: record.second) {
				unrepeated_regions->insert(reg);
			}
		}

		for(auto reg: *unrepeated_regions) {
			pos++;
			print_place(pos,*reg);
		}
        cout<<endl;

		delete unrepeated_regions;

        cout<< "Posti dovuti a regioni non essenziali: " << endl;

		set<Region *> *unrepeated_irredundant_regions = new set<Region *>();

        if(irredundant_regions!= nullptr) {
			for(auto record:*irredundant_regions){
				for(auto reg: record.second) {
					unrepeated_irredundant_regions->insert(reg);
				}
			}
			for (auto reg: *unrepeated_irredundant_regions) {
				pos++;
				print_place(pos, *reg);
			}
        }

        cout << endl;

        delete unrepeated_irredundant_regions;

        print_transactions();

        //todo:finire PN con pre e post regioni (pre_essential_region & post_essential_region && pre/post_irredundant)

	    cout << endl;
    }

    set<int> *region_difference(set<int> &first, set<int> &second) {
        auto s = new set<int>();
        for (auto state: first) {
            if (second.find(state) == second.end()) {
                s->insert(state);
            }
        }

        return s;
    }

    vector<Region> *copy_map_to_vector(map<int, vector<Region> *> *map) {
        auto input = new set<Region>();
        for (auto record: *map) {
            for (const auto &region: *record.second) {
                input->insert(region);
            }
        }
        vector<Region> *vec = new vector<Region>(input->size());
        std::copy(input->begin(), input->end(), vec->begin());
        delete input;
        return vec;
    }

    set<Region*> *copy_map_to_set(map<int, set<Region*> *> *map) {
        auto input = new set<Region*>();
        for (auto record: *map) {
            for (auto region: *record.second) {
                input->insert(region);
            }
        }

        return input;
    }


    bool is_bigger_than(Region *region, set<int> *region2) {

        if (region->size() > region2->size()) {
            for (auto elem: *region2) {
                //nella regione non trovo un elem
                if (region->find(elem) == region->end()) {
                    cout << "****FALSE ";
                    print(*region),
                            cout << " is not bigger than ";
                    println(*region2);
                    return false;
                }
            }
        } else if (region->size() <= region2->size())
            return false;

        //nella regione trovo tutti gli stati della reg2
        cout << "****TRUE";
        print(*region),
                cout << " is bigger than ";
        println(*region2);
        return true;
    }

    bool is_bigger_than_or_equal_to(Region* region ,set<int>* intersection){

        if(region->size() >= intersection->size()){
            cout<<"TRUE**************"<<endl;
            return true;
        }

        for(auto elem: *intersection){
            //nella regione non trovo un elem delll'intersez
            if( region->find(elem) == region->end()){
                cout<<"FALSE**************"<<endl;
                return false;
            }
        }

        //nella regione trovo tutti gli stati dell'intersezione
        cout<<"TRUE**************"<<endl;
        return true;
    }


    bool are_equals(Region *region1, Region *region2) {


        if (region1->size() != region2->size())
            return false;

        for (auto elem: *region1) {
            if (region2->find(elem) == region2->end()) {
                return false;
            }
        }

        return true;

    }

    bool contains(set<Region *> *set, Region *region) {

        for(auto elem:*set){
            if(are_equals(elem, region) ){
                return true;
            }
        }

        return false;
    }

    void print_pn_dot_file(map<int,set<Region*>*>* net, string file_name){
    	//todo: aggiungere il carattere speciale per i token -> utilizzando uno stile speciale per gli stati iniziali
        auto initial_reg = initial_regions(net);
    	string output_name = file_name;
    	string in_dot_name;
    	string output = "";
    	//todo: creo il collegamento tra le regioni ed un intero identificativo
        map<Region*, int>*regions_mapping;
        regions_mapping = get_regions_map(net);

    	while(output_name[output_name.size()-1] != '.'){
    		output_name = output_name.substr(0, output_name.size()-1);
    	}
	    output_name = output_name.substr(0, output_name.size()-1);
    	int lower = 0;
    	for(int i= output_name.size()-1; i > 0; i--){
    		if(output_name[i] == '/'){
    			lower = i;
    			break;
    		}
    	}
    	in_dot_name = output_name.substr(lower+1, output_name.size());
    	cout << "out name: " << in_dot_name << endl;

    	output_name = output_name + "_PN.dot";
    	cout << "file: " << output_name << endl;

    	//todo: finire la creazione del file in output
    	ofstream fout(output_name);
	    fout << "digraph ";
	    fout << in_dot_name+"_PN";
	    fout << "{\n";
	    fout << "subgraph initial_place {\n"
	            "\tnode [shape=doublecircle,fixedsize=true, fixedsize = 2, color = black, fillcolor = black, style = filled];";
		//todo: qui aggiunta regioni iniziali

	    fout << "\n}\n";
		fout << "subgraph place {     \n"
		        "\tnode [shape=circle,fixedsize=true, fixedsize = 2];";
		//todo: qui regioni non iniziali
	    //dovrò usare regions_difference: tutte - iniziali -> devo avere l'insieme di tutte le regioni iniziali
	    fout << "\n}\n";
	    fout << "subgraph transitions {\n"
	            "\tnode [shape=rect,height=0.2,width=2, forcelabels = false];";
	    for(auto record: *net){
	    	fout << "\tt"+to_string(record.first)+";\n";
	    }
	    fout << "}\n";
	    //todo: qui tutti gli archi tra posti e transazioni
	    for(auto record: *net){
	    	//ogni regione dovrebbe avere un alias, andrebbe bene avere la mappa tra evento e il numero di regione e non la regione stessa
	    }
	    fout << "\n}";
    	fout.close();
	    delete initial_reg;
	    delete regions_mapping;
    }


    map<Region *, int>* get_regions_map(map<int,set<Region*>*>* net){
    	auto regions_map = new map<Region *, int>();
    	int counter = 0;
    	for(auto record: *net){
    		for(auto reg: *record.second){
    			if(regions_map->find(reg) == regions_map->end()){
				    (*regions_map)[reg] = int(counter);
				    counter++;
    			}
    		}
    	}
    	//x debug
	    /*for(auto record: *regions_map){
    		cout << record.second << ": ";
    		println(*record.first);
    	}*/
    	return regions_map;
    }

    set<Region *>* initial_regions(map<int,set<Region*>*>* reg_map){
        auto init_reg = new set<Region *>();
        for(auto rec: *reg_map){
            for(auto reg: *rec.second){
                if(reg->find(initial_state) != reg->end()){
                    init_reg->insert(reg);
                }
            }
        }
        //per DEBUG:
		/*cout << "Initial regions:" << endl;
		for(auto reg: *init_reg){
        	println(*reg);
        }*/

        return init_reg;
    }

}

