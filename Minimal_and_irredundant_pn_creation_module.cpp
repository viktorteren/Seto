//
// Created by viktor on 18/04/18.
//

#include "Minimal_and_irredundant_pn_creation_module.h"
#include "Essential_region_search.h"

Minimal_and_irredundant_pn_creation_module::Minimal_and_irredundant_pn_creation_module(map<int, vector<Region*> *>* pre_reg, map<int, vector<Region*> *>* post_reg){
	pre_regions = pre_reg;
	post_regions = post_reg;
	not_essential_regions = new map<int, vector<Region*> *>();
	auto *ers = new Essential_regions_search(pre_regions);
	essential_regions = ers->search();
	search_events_with_not_essential_regions();
	//cost_map_filling();
	search_not_covered_states_per_event();
	//minimal_cost_search();
}

void Minimal_and_irredundant_pn_creation_module::search_events_with_not_essential_regions() {
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

void Minimal_and_irredundant_pn_creation_module::search_not_covered_states_per_event() {
	cout << "searching for not covered states ... " << endl;
	int event;
	//per ogni evento
	set<int> event_states;
	set<int> essential_states;
	set<int> uncovered_states;
	vector<Region*> *regions;
	auto essential_regions_of_event = new vector<Region*>();
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
		cout << "---------------" << endl;
		cout << "tutti gli stati degli eventi: ";
		print(event_states);
		cout << "stati coperti da eventi essenziali: ";
		print(essential_states);
		cout << "stati non coperti da eventi essenziali: ";
		print(uncovered_states);
		cout << "---------------" << endl;

		//per l'insieme degli stati non coperti devo trovare le possibili coperture irridondanti
		//todo: 1.1. trovare quali insiemi di regioni non essenziali (dell'evento in questione) coprono tali stati -> creare una parte dell'equazione
		//prendo una regione alla volta
		//calcolo gli stati che rimangono da coprire, torno al punto precedente finchè non ho coperto tutti gli stati
		//controllo se l'insieme che ho ricavato è irridondante


		//svuoto le variabili per ogni iterazione
		essential_regions_of_event->erase(essential_regions_of_event->begin(), essential_regions_of_event->end());
	}


}

void Minimal_and_irredundant_pn_creation_module::minimal_cost_search(){
	//QUI: dovrei eseguire l'ordinamento degli elementi della covering_map secondo l'euristica scelta:
	// per esempio la somma dei costi delle regioni dell'insieme di copertura nella cost_map in ordine crescente
	// userò sort() avendo creato il mio comparator

	//finchè non ho coperto tutto e non ho percorso tutti i rami
	//euristica:
	//1.2 scelgo una regione o un insiemee di regioni che coprono completamennte un evento [calcolato nel punto 1.1](tenendo salvate le altre scelte)
	//faccio il punto 1.2 fino ad una copertura completa o finchè non supero il costo totale dell'ultimo miglior risultato trovato
	//	CONTROLLANDO CHE AD OGNI AGGIUNTA LA COPERTURA NON DIVENTI RIDONDANTE !!!
	//1.3 valuto se il nuovo risultato è meglio del precedente (considerando i costi relativi sia alle pre-regioni che alle post-regioni)

	//alla fine troverò un insieme di regioni da aggiungere a quelle essenziali per completare la creazione della pn
}

//la mappa deve contenere solo i costi delle regioni non essenziali
void Minimal_and_irredundant_pn_creation_module::cost_map_filling(){
	for(auto record: *not_essential_regions){
		for(auto reg: *(record.second)){
			//non è ancora stato calcolato il costo per la regione reg
			if(cost_map->find(&(*reg)) == cost_map->end()){
				(*cost_map)[&(*reg)] = region_cost(*reg);
			}
		}
	}
}

int Minimal_and_irredundant_pn_creation_module::region_cost(const Region &reg) {

}


