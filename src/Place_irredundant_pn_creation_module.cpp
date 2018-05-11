//
// Created by viktor on 18/04/18.
//

#include <climits>
#include "../include/Place_irredundant_pn_creation_module.h"
#include "../include/Essential_region_search.h"

Place_irredundant_pn_creation_module::Place_irredundant_pn_creation_module(map<int, set<Region*> *>* pre_reg, map<int, set<Region*> *>* post_reg){
	pre_regions = pre_reg;
	post_regions = post_reg;
	cost_map = new map<Region* , int>();
	not_essential_regions_map = new map<int, set<Region*> *>();
	not_essential_regions = new set<Region *>();
	ers = new Essential_regions_search(pre_regions);
	essential_regions = ers->search();
	search_not_essential_regions();
	irredundant_regions = new set<Region *>();
	if(not_essential_regions->size() > 0){
		cost_map_filling();
		uncovered_states = search_not_covered_states_per_event();
		if(uncovered_states->size() > 0){
			set<int> states_to_cover = *uncovered_states;
            delete uncovered_states;
			auto used_regions = new set<Region *>();
			computed_paths_cache = new set<set<Region *>>();
			cout << "--------------------------------------------------- MINIMUM COST SEARCH --------------------------------------------" << endl;
			int min = minimum_cost_search(states_to_cover, used_regions, INT_MAX, 0);
			cout << "min cost: " << min << endl;
			cout << "insieme di regioni irridondante: " << endl;
			for(auto region: *irredundant_regions){
				//cout << "[" << &(*region)  << "] ";
				println(*region);
			}
			delete used_regions;
		}
		else{
			cout << "ALL STATES ARE COVERED BY ESSENTIAL REGIONS" << endl;
		}

	}
	else{
		cout << "ALL REGIONS ARE ESSENTIAL" << endl;
	}
}

Place_irredundant_pn_creation_module::~Place_irredundant_pn_creation_module(){
	delete ers;
	delete essential_regions;
	delete not_essential_regions;
	delete cost_map;
	for(auto rec: *not_essential_regions_map){
		delete rec.second;
	}
	delete not_essential_regions_map;

	if(irredundant_regions_map!= nullptr){
    	for(auto rec: *irredundant_regions_map){
        	delete rec.second;
    	}
    	delete irredundant_regions_map;
	}
	delete irredundant_regions;
	delete computed_paths_cache;
}

map<int, set<Region*>*> * Place_irredundant_pn_creation_module::get_irredundant_regions(){
	if(irredundant_regions->size()!=0) {
        calculate_irredundant_regions_map();
        return irredundant_regions_map;
    }
    else {
        return nullptr;
    }


}
map<int, set<Region*>*> *Place_irredundant_pn_creation_module::get_essential_regions(){
	return ers->get_essential_regions_map();
}

void Place_irredundant_pn_creation_module::search_not_essential_regions() {
	for(auto record: *pre_regions){

		//cout << "nuovo evento: " << record.first << endl;
		for(auto region: *record.second){
			//cout << &(*region) << endl;
			//regione non essenziale
                if (essential_regions->find(&(*region)) == essential_regions->end()) {
                    if (not_essential_regions_map->find(record.first) == not_essential_regions_map->end()) {
                        (*not_essential_regions_map)[record.first] = new set<Region *>();
                    }
                    (*not_essential_regions_map)[record.first]->insert(region);
                    not_essential_regions->insert(region);
                }
		}
	}
	//per debug:
	/*
	for(auto record: *not_essential_regions_map){
		cout << "evento: " << record.first << endl;
		for(auto region: *record.second){
			cout << &(*region) << endl;
		}
	}*/
}

set<int> *Place_irredundant_pn_creation_module::search_not_covered_states_per_event() {
	cout << "--------------------------------------------------- SEARCHING FOR UNCOVERED STATES --------------------------------------------" << endl;
	int event;
	set<int> *event_states = nullptr;
	set<int> *essential_states = nullptr;
	//set<int> *uncovered_states = nullptr;
	set<int> *total_uncovered_states = nullptr;
	set<Region*> *regions;
	auto essential_regions_of_event = new vector<Region*>();
	bool first_iteration=true;

	//per ogni evento che ha regioni non essenziali:
	for(auto record: *not_essential_regions_map){
        if(!first_iteration) delete uncovered_states;
		first_iteration=false;
        set<int> *uncovered_states = nullptr;
		event = record.first;
		cout << "uncovered states for event " << event  << ":" << endl;
		//calcolo l'unione degli stati coperti dalle pre-regioni di quel evento
		regions = (*(pre_regions)->find(event)).second;
		event_states = regions_union(regions);
		//calcolo gli stati coperti da pre-regioni essenziali
		//scorro tutte le regioni dell'evento estraendo solo quelle essenziali
		//cout << "aggiunta regioni essenziali dell'evento " << event << endl;
		for(auto reg: *(pre_regions->find(event)->second)){
			if(essential_regions->find(reg) != essential_regions->end()){
				/*cout << "regione essenziale: ";
				print(*reg);*/
				essential_regions_of_event->push_back(reg);
			}
		}
		essential_states = regions_union(essential_regions_of_event);

		//calcolo gli stati non ancora coperti
		uncovered_states = region_difference(*event_states, *essential_states);
		println(*uncovered_states);
		//cout << "---------------" << endl;
		//cout << "evento: " << record.first << endl;
		/*cout << "tutti gli stati degli eventi: ";
		print(event_states);
		cout << "stati coperti da eventi essenziali: ";
		print(essential_states);*/
		/*cout << "stati non coperti da eventi essenziali: ";
		println(uncovered_states);
		cout << "---------------" << endl;*/

		if(total_uncovered_states == nullptr){
			total_uncovered_states = uncovered_states;
		}
		else{
			if(total_uncovered_states->empty()){
				total_uncovered_states = uncovered_states;
			}
			else{
//				total_uncovered_states = regions_union(total_uncovered_states, uncovered_states);
                auto tmp=regions_union(total_uncovered_states, uncovered_states);
                delete total_uncovered_states;
                total_uncovered_states=tmp;
                delete uncovered_states;
			}
		}

		//svuoto le variabili per ogni iterazione
		essential_regions_of_event->erase(essential_regions_of_event->begin(), essential_regions_of_event->end());
		delete event_states;
		delete essential_states;
	}
	delete essential_regions_of_event;
	delete uncovered_states;

	return total_uncovered_states;
}

int Place_irredundant_pn_creation_module::minimum_cost_search(set<int> states_to_cover, set<Region *> *used_regions, int last_best_cost, int father_cost){
	//per l'insieme degli stati non coperti devo trovare le possibili coperture irridondanti
	//trovare quali insiemi di regioni non essenziali

	//metto insieme gli stati non coperti senza fare la divisione per evento
	//e aggiungo regioni non essenziali che coprono più stati possibili tra quelli non coperti
	//mi fermo se supero il vecchio risultato migliore -> alla fine  l'insieme è sicuramente irridondante

	Region *candidate;
	int cover_of_candidate;
	int temp_cover;
	//coppio il contenuto del padre per non sovrascrivere i dati con l'insiieme delle regioni del figlio
	int cost_of_candidate;
	set<int> *new_states_to_cover;
	set<Region *> *new_states_used = used_regions;
	auto chosen_candidates = new set<Region *>();
	set<Region *> *temp_aggregation;
	int new_best_cost = last_best_cost; //uno dei sotto-rami potrebbe aver migliorato il risultato, di conseguenza devo aggiornare la variabile e non utilizzare il parametro in ingresso alla funzione
	//finchè ci sono candidati che aumentano la copertura
	while(true){
		cover_of_candidate = 0;
		//scelta del prossimo candidato
		for(auto region: *not_essential_regions){
			/*if(used_regions->size() == 0){
				cout << "	---candidate: ";
				println(*region);
			}*/
			//la regione nuova non può essere un vecchio candidato
			if(chosen_candidates->find(region) == chosen_candidates->end()){
				//controllo se l'insieme con il candidato è già stato calcolato o no [faccio prima questo controllo dato che lo ritengo più leggero dell'intersezione nella condizione successiva]
				temp_aggregation = new set<Region *>(*used_regions);
				temp_aggregation->insert(region);
				if(computed_paths_cache->find(*temp_aggregation) == computed_paths_cache->end()) {
					//devo vedere la dimensione dell'intersezione tra gli stati da coprire e la regione
					auto cover = regions_intersection(&states_to_cover, region);
					temp_cover = cover->size();
					delete cover;
					if (temp_cover > cover_of_candidate) {
						//cout << "candidato nuovo" << endl;
						cover_of_candidate = temp_cover;
						candidate = region;
					}
				}
				else{
					//cout << "elemento già presente nella cache" << endl;
				}
				//ogni volta che entro nell'if alloco un nuovo spazio di conseguenza devo deallocarlo
				delete temp_aggregation;
			}
		}
		//cout << "cover of candidate: " << cover_of_candidate << endl;

		//non posso migliorare la copertura
		if(cover_of_candidate == 0)
			break;
		else{
			//salvo il nuovo candidato
			chosen_candidates->insert(candidate);
		}
		cost_of_candidate = (*cost_map)[&(*candidate)];
		//cout << "cost of candidate: " << cost_of_candidate << endl;
		//cout << "candidate cover: " << cover_of_candidate << endl;
		//non devo fare una chiamata ricorsiva se il costo è troppo grande oppure se ho completato la copertura

		//non potrò trovare una soluzione migliore con il seguente candidato
		int current_cost = cost_of_candidate + father_cost;
		new_states_used->insert(candidate);

		/*cout << "prova con: ";
		for(auto r: new_states_used){
			print(*r);
			cout << "|||";
		}
		cout << endl;*/
		if(current_cost >= new_best_cost){
			//salvo il percorso nella cache per non ripeterlo
			computed_paths_cache->insert(*new_states_used);
			//break; -> non chiamare la chiamata ricorsiva ma non fare nemmeno break dato che i fratelli con copertura minore possono avere costo più basso
		}
		//ho completato la copertura ed è meglio di quella precedente
		else if(states_to_cover.size() - cover_of_candidate == 0){

			new_best_cost = current_cost;
			/*cout << "trovato nuovo minimo: " << new_best_cost << endl;
			for(auto r: new_states_used){
				println(*r);
			}
			cout << "aggiungendo ";
			println(*candidate);
			cout << "costo precedente era: " << father_cost << endl;*/

			//salvo il percorso nella cache per non ripeterlo
			computed_paths_cache->insert(*new_states_used);

			//dealloco lo spazio vecchio per allocarne uno nuovo
			delete irredundant_regions;
			irredundant_regions = new set<Region *>(*new_states_used);
		}
		//non ho completato la copertura e posso ancora trovare una soluzione migliore
		else{
			//essedo già stato scelto il candidato e sapendo che devo fare la chiamata ricorsiva devo calcolarmi il nuovo insieme di stati da coprire
			new_states_to_cover = region_difference(states_to_cover, *candidate);

			//chiamata ricorsiva per espandere ulteriormente la copertura con il candidato scelto
			int cost = minimum_cost_search(*new_states_to_cover, new_states_used, new_best_cost, current_cost);
			if(cost < new_best_cost){
				new_best_cost = cost;
				//non devo salvarmi il risultato migliore dato che è già stato salvato nella chiamata ricorsiva che ha fatto ritornare il costo minore
			}
			delete new_states_to_cover;
		}
		new_states_used = used_regions;
	}

	//salvo il percorso nella cache per non ripeterlo: ho calcolato tutti i sottorami di questo nodo per questo sono arrivato al return
	computed_paths_cache->insert(*used_regions);
	delete chosen_candidates;
	return new_best_cost;

}

//la mappa deve contenere solo i costi delle regioni non essenziali
void Place_irredundant_pn_creation_module::cost_map_filling(){
	cout << "--------------------------------------------------- COST MAP FILLING --------------------------------------------" << endl;

	for(auto record: *not_essential_regions_map){
		for(Region* reg: *record.second){
			//non è ancora stato calcolato il costo per la regione reg
			if(cost_map->find(&(*reg)) == cost_map->end()){
				(*cost_map)[&(*reg)] = region_cost(&(*reg));
				cout << "Trovato regione non essenziale :";
				print(*reg);
				cout << "[" << &(*reg)  << "] con il costo: " << (*cost_map)[&(*reg)]  << endl;
			}
		}
	}
}

unsigned long Place_irredundant_pn_creation_module::region_cost(Region *reg) {
	//funzione di costo per minimizzare sia il numero di posti che il numero di archi
	/*int cost = 1;
	for(auto record: *pre_regions){
		if(record.second->find(reg) != record.second->end()){
			cost++;
		}
	}
	for(auto record: *post_regions){
		if(record.second->find(reg) != record.second->end()){
			cost++;
		}
	}
	return cost;*/

	//funzione di costo per minimizzare solo il numero di posti
	return reg->size();
}

void Place_irredundant_pn_creation_module::calculate_irredundant_regions_map() {
	//algoritmo:
	//per ogni elemento di not_essential_regions_map se la regione è presente in irredundant_regions

    cout<<"CALCULATE IRREDUNDANT REGION MAP_______________"<<endl;
	irredundant_regions_map = new map<int, set<Region*>*>();
	for(auto record: *not_essential_regions_map){
		for(auto reg: *record.second){
			if(irredundant_regions->find(reg) != irredundant_regions->end()){
				if(irredundant_regions_map->find(record.first) == irredundant_regions_map->end()){
					(*irredundant_regions_map)[record.first] = new set<Region *>();
				}

                cout<<"REGION: " <<reg<< " AT " << record.first<<endl;
				(*irredundant_regions_map)[record.first]->insert(reg);
			}
		}
	}
	//debug
	/*cout << "prove irredundant regions:" << endl;
	for(auto rec: *irredundant_regions_map){
		cout << "set di regioni dell'evento: " << rec.first << endl;
		for(auto reg: rec.second){
			println(*reg);
		}
	}*/

}
