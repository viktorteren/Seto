//
// Created by viktor on 18/04/18.
//

#pragma once

#include "Essential_region_search.h"
using namespace std;

//irridondante perchè per tutti i rami computazionali non si prenderà mai un per corso che contiene ridondanze
//minimo perchè tra tutti i percorsi irridondanti verrà scelto quello che ha il costo minore
//minimo -> irridondante ma non viceversa
class Place_irredundant_pn_creation_module {
public:
	explicit Place_irredundant_pn_creation_module(map<int, set<Region*> *>* pre_reg, map<int, set<Region*> *>* post_reg);
	~Place_irredundant_pn_creation_module();

	map<int, set<Region*>*> * get_irredundant_regions();
	map<int, set<Region*>*> * get_essential_regions();


private:
	Essential_regions_search *ers;
	map<int, set<Region*> *> * pre_regions= nullptr;
	map<int, set<Region*> *> * post_regions= nullptr;
	set<Region *>* essential_regions= nullptr;
	set<Region *> *not_essential_regions= nullptr;
	set<int> *uncovered_states= nullptr;
	void search_not_essential_regions();
	map<int, set<Region*> *> * not_essential_regions_map; //mappa: evento -> insieme di regioni non essenziali per quel evento
	set<int>* search_not_covered_states_per_event();
	map<Region* , int> *cost_map; //costo per ogni regione (dato da: numero eventi per i quali è una pre-regione + numero eventi per i quali è una post-regione + 1)
	void cost_map_filling();
	//ritorna il miglior risultato dei rami sottostanti
	int minimum_cost_search(set<int> states_to_cover, set<Region *> *used_regions, int last_best_cost, int father_cost);
	unsigned long region_cost(Region *reg);
	set<Region *>* irredundant_regions = nullptr;
	set<set<Region *>> *computed_paths_cache = nullptr; //se ho già calcolato il costo di un percorso [insieme di regioni] allora è presente all'interno della cache
	map<int, set<Region*>*> *irredundant_regions_map = nullptr;
	void calculate_irredundant_regions_map();

};