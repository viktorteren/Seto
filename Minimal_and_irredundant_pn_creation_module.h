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
	explicit Minimal_and_irredundant_pn_creation_module(map<int, set<Region*> *>* pre_reg, map<int, set<Region*> *>* post_reg);
	~Minimal_and_irredundant_pn_creation_module();
private:
	map<int, set<Region*> *> * pre_regions;
	map<int, set<Region*> *> * post_regions;
	set<Region *> essential_regions;
	set<Region *> *not_essential_regions;
	set<int> uncovered_states;
	void search_not_essential_regions();
	map<int, set<Region*> *> * not_essential_regions_map; //mappa: evento -> insieme di regioni non essenziali per quel evento

	set<int>& search_not_covered_states_per_event();
	map<Region* , int> *cost_map; //costo per ogni regione (dato da: numero eventi per i quali è una pre-regione + numero eventi per i quali è una post-regione + 1)
	void cost_map_filling();
	/*todo: in questa mappa bisognerebbe fare un ordinamento degli insiemi di regioni per il loro costo nella cost_map prima di utilizzarla
	 (in questo modo si seguirebbe un'euristica per migliorare i tempi di esecuzione)*/
	//LA SEGUENTE MAPPA NON SERVE SE CONSIDERO TUTTI GLI STATI NON COPERTI INSIEME
	//BASTA SALVARE IN QUALCHE MODO I SEGUENTI COLLEGAMENTI
	//STATI DA COPRIRE -> INSIEMI DI REGIONI CHE COPRONO TALI STATI: in questo modo utilizzo la strategia di prima ma considerando sempre gli stati da coprire come un unico insieme
	//salvo tale mapa per ridurre i calcoli:
	//una volta che ho scelto uno stato l'insieme degli stati da coprire può variare ma posso ritrovarmi più volte con lo stesso insieme

	map<set<int>, set<set<Region *> >> *covering_map; //mappa tra insiemi di stati non coperti e l'insieme di insiemi di regioni che coprono completamente tali insiemi di stati
	//insieme di stati -> insieme di insiemi di regioni: esistono diverse combinazioni irridondanti di regioni che possono coprire un insieme di stati
	void minimal_cost_search();
	int region_cost(Region *reg);
};


#endif //MINIMAL_AND_IRREDUNDANT_PN_CREATION_MODULE_H
