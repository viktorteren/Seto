//
// Created by viktor on 18/04/18.
//

#include "Minimal_and_irredundant_pn_creation_module.h"
#include "Essential_region_search.h"

Minimal_and_irredundant_pn_creation_module::Minimal_and_irredundant_pn_creation_module(map<int, set<Region*> *>* pre_reg, map<int, set<Region*> *>* post_reg){
	pre_regions = pre_reg;
	post_regions = post_reg;
	cost_map = new map<Region* , int>();
	covering_map = new map<set<int>, set<set<Region *> >>();
	not_essential_regions_map = new map<int, set<Region*> *>();
	not_essential_regions = new set<Region *>();
	auto *ers = new Essential_regions_search(pre_regions);
	essential_regions = ers->search();
	search_not_essential_regions();
	cost_map_filling();
	uncovered_states = search_not_covered_states_per_event();
	minimal_cost_search();
}

void Minimal_and_irredundant_pn_creation_module::search_not_essential_regions() {
	for(auto record: *pre_regions){

		//cout << "nuovo evento: " << record.first << endl;
		for(auto region: *record.second){
			//cout << &(*region) << endl;
			//regione non essenziale
			if(essential_regions.find(&(*region)) == essential_regions.end()){
				if (not_essential_regions_map->find(record.first) == not_essential_regions_map->end()) {
					(*not_essential_regions_map)[record.first] = new set<Region*>();
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

set<int>& Minimal_and_irredundant_pn_creation_module::search_not_covered_states_per_event() {
	//todo: si potrebbe migliorare questo metodo utilizzando essential_regions al posto di essential_regions_map facendo un solo calcolo senza cicli for
	cout << "--------------------------------------------------- SEARCHING FOR UNCOVERED STATES --------------------------------------------" << endl;
	int event;
	set<int> event_states;
	set<int> essential_states;
	set<int> uncovered_states;
	set<int> total_uncovered_states;
	set<Region*> *regions;
	auto essential_regions_of_event = new vector<Region*>();
	//per ogni evento che ha regioni non essenziali:
	for(auto record: *not_essential_regions_map){
		event = record.first;
		//calcolo l'unione degli stati coperti dalle pre-regioni di quel evento
		regions = (*(pre_regions)->find(event)).second;
		event_states = regions_union(regions);

		//calcolo gli stati coperti da pre-regioni essenziali
		//scorro tutte le regioni dell'evento estraendo solo quelle essenziali
		//cout << "aggiunta regioni essenziali dell'evento " << event << endl;
		for(auto reg: *(pre_regions->find(event)->second)){
			if(essential_regions.find(reg) != essential_regions.end()){
				/*cout << "regione essenziale: ";
				print(*reg);*/
				essential_regions_of_event->push_back(reg);
			}
		}
		essential_states = regions_union(essential_regions_of_event);


		//calcolo gli stati non ancora coperti

		uncovered_states = region_difference(event_states, essential_states);
		//cout << "---------------" << endl;
		//cout << "evento: " << record.first << endl;
		/*cout << "tutti gli stati degli eventi: ";
		print(event_states);
		cout << "stati coperti da eventi essenziali: ";
		print(essential_states);*/
		/*cout << "stati non coperti da eventi essenziali: ";
		println(uncovered_states);
		cout << "---------------" << endl;*/

		if(total_uncovered_states.empty()){
			total_uncovered_states = uncovered_states;
		}
		else{
			total_uncovered_states = regions_union(&total_uncovered_states, &uncovered_states);
		}

		//svuoto le variabili per ogni iterazione
		essential_regions_of_event->erase(essential_regions_of_event->begin(), essential_regions_of_event->end());
	}
	cout << "total uncovered states: ";
	println(total_uncovered_states);
	return total_uncovered_states;
}

void Minimal_and_irredundant_pn_creation_module::minimal_cost_search(){
	//per l'insieme degli stati non coperti devo trovare le possibili coperture irridondanti
	//todo: 1.1. trovare quali insiemi di regioni non essenziali (dell'evento in questione) coprono tali stati -> creare una parte dell'equazione
	//prendo una regione alla volta
	//calcolo gli stati che rimangono da coprire, torno al punto precedente finchè non ho coperto tutti gli stati
	//controllo se l'insieme che ho ricavato è irridondante

	//metodo simile ma alternativo:
	//metto insieme gli stati non coperti senza fare la divisione per evento
	//e aggiungo regioni che coprono più stati possibili -> potrei avere un miglioramento delle prestazioni
	//il resto è analogo: mi fermo se supero il vecchio risultato migliore o l'insieme non è più irridondante
	//alla fine è come se avessi l'equazione del punto 1.1 ma minimizzata parzialmente

	//trovare tutte le coperture possibili è troppo difficile:
	//prendo una regione non essenzialee alla volta e vedo quanti stati non coperti copre:
	//scelgo quella che copre più stati procedo con l'esecuzione
	//mi serve una funzione che dato un insieme di stati e una regione ritorni il numero di stati coperti da quella regione: cardinalità dell'intersezione
	//devo salvarmi tutte le regioni irridondanti insieme senza mappe
	set<int> states_to_cover = uncovered_states;
	Region *candidate;
	int cover_of_candidate = 0;
	int temp_cover;
	for(auto region: *not_essential_regions){
		//devo vedere la dimensione dell'intersezione tra gli stati da coprire e la regione
		temp_cover = regions_intersection(&states_to_cover, region).size();
		if(temp_cover > cover_of_candidate){
			cover_of_candidate = temp_cover;
			candidate = region;
		}
	}

	//ho trovato il candidato, devo assegnarlo
	//se scelgo le regioni per numero di stati coperti decrescente è possibile avere alla fine di un ramo di scelte uno stato ridondante??
	//suppongo di no oppure sono casi limite (probabilmente quelli succedono spesso)
	//potrei non guardare le ridondanze finchè non trovo una soluzione e solo dopo eliminare stati ridondanti: probabilmente troppo complesso dato che possono essere diversi insiemi ridondanti

	//per vedere le ridondanze: quando aggiungo una regione nuova guardo con quali regioni si interseca e vedo se togliendo le regioni la copertura rimane invariata
	//i modi per togliere le regioni possono cambiare

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
	cout << "--------------------------------------------------- COST MAP FILLING --------------------------------------------" << endl;

	for(auto record: *not_essential_regions_map){
		for(Region* reg: *record.second){
			//non è ancora stato calcolato il costo per la regione reg
			if(cost_map->find(&(*reg)) == cost_map->end()){
				//todo: bugfix -> con input2 ho 2 regioni con lo stesso contenuto ma indirizzi diversi
				(*cost_map)[&(*reg)] = region_cost(&(*reg));
				cout << "Trovato regione non essenziale :";
				Utilities::print(*reg);
				cout << "[" << &(*reg)  << "] con il costo: " << (*cost_map)[&(*reg)]  << endl;
			}
		}
	}
}

int Minimal_and_irredundant_pn_creation_module::region_cost(Region *reg) {
	int cost = 1;
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
	return cost;
}


