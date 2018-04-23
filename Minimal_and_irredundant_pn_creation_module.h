//
// Created by viktor on 18/04/18.
//

#ifndef MINIMAL_AND_IRREDUNDANT_PN_CREATION_MODULE_H
#define MINIMAL_AND_IRREDUNDANT_PN_CREATION_MODULE_H

#include "Utilities.h"
using namespace std;

//irridondante perchè per tutti i rami computazionali non si prenderà mai un per corso che contiene ridondanze
//minimo perchè tra tutti i percorsi irridondanti verrà scelto quello che ha il costo minore
//minimo -> irridondante ma non viceversa
class Minimal_and_irredundant_pn_creation_module {
public:
	explicit Minimal_and_irredundant_pn_creation_module(map<int, vector<Region*> *>* pre_reg, map<int, vector<Region*> *>* post_reg);
	~Minimal_and_irredundant_pn_creation_module();
private:
	map<int, vector<Region*> *> * pre_regions;
	map<int, vector<Region*> *> * post_regions;
	set<Region *> essential_regions;
	void search_events_with_not_essential_regions();
	map<int, vector<Region*> *> * not_essential_regions; //mappa: evento -> insieme di regioni non essenziali per quel evento
	void search_not_covered_states_per_event();
	map<Region* , int> *cost_map; //costo per ogni regione (dato da: numero eventi per i quali è una pre-regione + numero eventi per i quali è una post-regione + 1)
	void cost_map_filling();
	/*todo: in questa mappa bisognerebbe fare un ordinamento degli insiemi di regioni per il loro costo nella cost_map prima di utilizzarla
	 (in questo modo si seguirebbe un'euristica per migliorare ii tempi di esecuzione)*/
	map<set<int>, set<set<Region *> >> *covering_map; //mappa tra insiemi di stati non coperti e l'insieme di insiemi di regioni che coprono completamente tali insiemi di stati
	//insieme di stati -> insieme di insiemi di regioni: esistono diverse combinazioni irridondanti di regioni che possono coprire un insieme di stati
	void minimal_cost_search();
	int region_cost(const Region& reg);
};


#endif //MINIMAL_AND_IRREDUNDANT_PN_CREATION_MODULE_H
