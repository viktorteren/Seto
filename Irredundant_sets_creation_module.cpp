//
// Created by viktor on 18/04/18.
//

#include "Irredundant_sets_creation_module.h"
#include "Essential_region_search.h"

Irredundant_sets_creation_module::Irredundant_sets_creation_module(map<int, vector<Region*> *>* pre_reg){
	pre_regions = pre_reg;
	not_essential_regions = new map<int, vector<Region*> *>();
	Essential_regions_search *ers = new Essential_regions_search(pre_regions);
	essential_regions = ers->search();
	search_events_with_not_essential_regions();
	search_not_covered_states_per_event();

}

void Irredundant_sets_creation_module::search_events_with_not_essential_regions() {
	for(auto record: *pre_regions){

		//cout << "nuovo evento: " << record.first << endl;
		for(auto region: *record.second){
			//cout << &(*region) << endl;
			//regione non essenziale
			if(essential_regions.find(&(*region)) == essential_regions.end()){
				if (not_essential_regions->find(record.first) == not_essential_regions->end()) {
					(*not_essential_regions)[record.first] = new vector<Region*>();
				}
				(*not_essential_regions)[record.first]->push_back(region);
			}
		}
	}
	//per debug:
	/*cout << "prova" << endl;
	for(auto record: *not_essential_regions){
		cout << "evento: " << record.first << endl;
		for(auto region: *record.second){
			cout << &(*region) << endl;
		}
	}*/
}

void Irredundant_sets_creation_module::search_not_covered_states_per_event() {
	cout << "searching for not covered states ... " << endl;
	int event;
	//per ogni evento
	set<int> event_states;
	set<int> essential_states;
	set<int> uncovered_states;
	vector<Region*> *regions;
	vector<Region*> *essential_regions_of_event = new vector<Region*>();
	//per ogni evento che ha regioni non essenziali:
	for(auto record: *not_essential_regions){
		event = record.first;
		//calcolo l'unione degli stati coperti dalle pre-regioni di quel evento
		regions = (*(pre_regions)->find(event)).second;
		event_states = regions_union(regions);

		//calcolo gli stati coperti da pre-regioni essenziali
		//scorro tutte le regioni dell'evento estraendo solo quelle essenziali
		cout << "aggiunta regioni essenziali dell'evento " << event << endl;
		for(auto reg: *(pre_regions->find(event)->second)){
			if(essential_regions.find(reg) != essential_regions.end()){
				cout << "regione essenziale: ";
				print(*reg);
				essential_regions_of_event->push_back(reg);
			}
		}
		essential_states = regions_union(essential_regions_of_event);


		//calcolo gli stati non ancora coperti

		uncovered_states = region_difference(event_states, essential_states);
		/*cout << "---------------" << endl;
		cout << "tutti gli eventi: ";
		print(event_states);
		cout << "stati essenziali: ";
		print(essential_states);
		cout << "differenza: ";
		print(uncovered_states);
		cout << "---------------" << endl;*/

		//per l'insieme degli stati non coperti devo trovare una copertura
		//1. trovare quali regioni non essenziali coprono tali stati -> creare una parte dell'equazione


		//svuoto le variabili per ogni iterazione
		essential_regions_of_event->erase(essential_regions_of_event->begin(), essential_regions_of_event->end());
	}

	//2. trovare le coperture irridondanti (moltiplicando tra loro le parti dell'equazioni trovate nel ciclo for precedente): la funzione teorica da 1
	//3. trovare la copertura che

}


