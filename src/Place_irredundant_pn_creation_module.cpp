//
// Created by viktor on 18/04/18.
//

#include "../include/Place_irredundant_pn_creation_module.h"
#include "../include/Essential_region_search.h"
#include <climits>

Place_irredundant_pn_creation_module::Place_irredundant_pn_creation_module(
        map<int, set<Region *> *> *pre_reg, map<int, ER> *er_map) {

    pre_regions = pre_reg;
    // post_regions = post_reg;

    cost_map = new map<Region *, int>();
    not_essential_regions_map = new map<int, set<Region *> *>();
    not_essential_regions = new set<Region *>();
    ers = new Essential_regions_search(pre_regions);
    essential_regions = ers->search();
    all_events = new set<int>();
    for (auto record: *pre_regions) {
        all_events->insert(record.first);
    }
    events_without_essential_regions = calculate_events_without_essential_regions();
    search_not_essential_regions();
    irredundant_regions = new set<Region *>();
    er = er_map;
    if (!not_essential_regions->empty()) {
        cost_map_filling();
        if (print_step_by_step) {
            cout << "Regioni non essenziali:" << endl;
            println(*not_essential_regions);
            cout << endl;
        }
        //cout << "num regioni non essenziali: " << not_essential_regions->size() << endl;
        //println(*not_essential_regions);
        uncovered_states = search_not_covered_states_per_event();
        if (!uncovered_states->empty()) {
            set<int> states_to_cover = *uncovered_states;
            delete uncovered_states;
            auto used_regions = new set<Region *>();
            computed_paths_cache = new set<set<Region *>>();
            //cout << "--------------------------------------------------- MINIMUM "
            //     "COST SEARCH --------------------------------------------"
            //<< endl;
            //minimum_cost_search(states_to_cover, used_regions, INT_MAX, 0);
            //cout << "-----------IRREDUNDANT REGIONS SEARCH-------------" << endl;
            minimum_cost_search_with_label_costraints(states_to_cover, used_regions, INT_MAX, 0, 0);
            //  cout << "min cost: " << min << endl;
            if (print_step_by_step) {
                cout << "Irredundant set of regions: " << endl;
                for (auto region : *irredundant_regions) {
                    // cout << "[" << &(*region)  << "] ";
                    println(*region);
                }
                cout << "" << endl;
            }
            delete used_regions;
        } else {
            if (print_step_by_step) {
                cout << "Each region is essential" << endl;
                cout << "" << endl;
            }
        }

    } else {
        if (print_step_by_step) {
            cout << "Each region is essential" << endl;
            cout << "" << endl;
        }
    }
}

Place_irredundant_pn_creation_module::~Place_irredundant_pn_creation_module() {
    delete ers;
    delete essential_regions;
    delete not_essential_regions;
    delete cost_map;
    for (auto rec : *not_essential_regions_map) {
        delete rec.second;
    }
    delete not_essential_regions_map;

    if (irredundant_regions_map != nullptr) {
        for (auto rec : *irredundant_regions_map) {
            delete rec.second;
        }
        delete irredundant_regions_map;
    }
    delete irredundant_regions;
    delete computed_paths_cache;
    delete events_without_essential_regions;
    delete all_events;
}

set<int> *Place_irredundant_pn_creation_module::calculate_events_without_essential_regions() {
    int cont = 0;
    auto events = new set<int>();
    for (auto record: *pre_regions) {
        for (auto region: *record.second) {
            cont = 0;
            if (essential_regions->find(region) != essential_regions->end()) {
                cont++;
                break;
            }
        }
        if (cont == 0) {
            events->insert(record.first);
        }
    }
    /*cout << "events without essential regions: ";
    for(auto ev: *events){
      cout << " " << ev;
    }*/
    cout << endl;
    return events;
}

bool Place_irredundant_pn_creation_module::irredundant_set_of_regions(set<Region *> *irredundant_regions) {
    auto candidate_set_of_regions = new set<Region *>(*irredundant_regions);
    //unione tra le regioni essenziali e quelle irridondanti
    candidate_set_of_regions->insert(essential_regions->begin(), essential_regions->end());

    for (auto ev: *all_events) {
        //vedo quali regioni dell'evento ev sono rimaste
        auto regions_of_ev = new set<Region *>();
        for (auto reg: *pre_regions->at(ev)) {
            if (candidate_set_of_regions->find(reg) != candidate_set_of_regions->end()) {
                regions_of_ev->insert(reg);
            }
        }
        //guardo se tutte le sue regioni fanno soddisfare l'ec
        if (!irredundant_set_for_event(ev, regions_of_ev)) {
            delete regions_of_ev;
            delete candidate_set_of_regions;
            return false;
        }
        delete regions_of_ev;
    }
    delete candidate_set_of_regions;

    return true;
}

bool Place_irredundant_pn_creation_module::irredundant_set_for_event(int event, set<Region *> *events_regions) {
    set<Region *> *regions_of_event = pre_regions->at(event);
    int total_states = regions_union(regions_of_event)->size();
    int temp_states;
    for (auto reg: *events_regions) {
        auto regions_copy = new set<Region *>(*events_regions);
        regions_copy->erase(reg);
        temp_states = regions_union(regions_copy)->size();
        if (temp_states == total_states) {
            delete regions_copy;
            return false;
        }
        delete regions_copy;
    }
    return true;
}

bool Place_irredundant_pn_creation_module::all_events_have_ec_satisfied(set<Region *> &irredundant_regions) {
    //per ogni evento che non ha regioni essenziali

    auto candidate_set_of_regions = new set<Region *>(irredundant_regions);
    //unione tra le regioni essenziali e quelle irridondanti
    candidate_set_of_regions->insert(essential_regions->begin(), essential_regions->end());

    for (auto ev: *all_events) {
        //vedo quali regioni dell'evento ev sono rimaste
        auto regions_of_ev = new set<Region *>();
        for (auto reg: *pre_regions->at(ev)) {
            if (candidate_set_of_regions->find(reg) != candidate_set_of_regions->end()) {
                regions_of_ev->insert(reg);
            }
        }
        //guardo se tutte le sue regioni fanno soddisfare l'ec
        if (!ec_satisfied(ev, regions_of_ev)) {
            delete regions_of_ev;
            delete candidate_set_of_regions;
            return false;
        }
        delete regions_of_ev;
    }
    delete candidate_set_of_regions;
    //cout << "all ec satiisfied" << endl;
    return true;
}

bool Place_irredundant_pn_creation_module::ec_satisfied(int event, set<Region *> *events_regions) {
    auto intersection = regions_intersection(events_regions);
    if (intersection->size() != er->at(event)->size()) {
        delete intersection;
        //cout << "return false ec satisfied" << endl;
        return false;
    } else {
        for (auto reg: *er->at(event)) {
            if (intersection->find(reg) == intersection->end()) {
                delete intersection;
                //cout << "return false ec satisfied" << endl;
                return false;
            }
        }
    }
    delete intersection;
    //cout << "return true ec satisfied" << endl;
    return true;
}


map<int, set<Region *> *> *
Place_irredundant_pn_creation_module::get_irredundant_regions() {
    if (!irredundant_regions->empty()) {
        calculate_irredundant_regions_map();
        return irredundant_regions_map;
    } else {
        return nullptr;
    }
}

map<int, set<Region *> *> *
Place_irredundant_pn_creation_module::get_essential_regions() {
    return ers->get_essential_regions_map();
}

void Place_irredundant_pn_creation_module::search_not_essential_regions() {
    for (auto record : *pre_regions) {

        // cout << "nuovo evento: " << record.first << endl;
        for (auto region : *record.second) {
            // cout << &(*region) << endl;
            // regione non essenziale
            if (essential_regions->find(&(*region)) == essential_regions->end()) {
                if (not_essential_regions_map->find(record.first) ==
                    not_essential_regions_map->end()) {
                    (*not_essential_regions_map)[record.first] = new set<Region *>();
                }
                (*not_essential_regions_map)[record.first]->insert(region);
                not_essential_regions->insert(region);
            }
        }
    }
    // per debug:
    /*
    for(auto record: *not_essential_regions_map){
            cout << "evento: " << record.first << endl;
            for(auto region: *record.second){
                    cout << &(*region) << endl;
            }
    }*/
}

set<int> *Place_irredundant_pn_creation_module::search_not_covered_states_per_event() {
    //cout << "--------------------------------------------------- SEARCHING FOR "
    //     "UNCOVERED STATES --------------------------------------------"
    //<< endl;
    int event;
    set<int> *event_states = nullptr;
    set<int> *essential_states = nullptr;
    // set<int> *uncovered_states = nullptr;
    set<int> *total_uncovered_states = nullptr;
    set<Region *> *regions;
    auto essential_regions_of_event = new vector<Region *>();
    bool first_iteration = true;

    // per ogni evento che ha regioni non essenziali:
    for (auto record : *not_essential_regions_map) {
        if (!first_iteration)
            delete uncovered_states;
        first_iteration = false;
        set<int> *uncovered_states = nullptr;
        event = record.first;
        // cout << "uncovered states for event " << event << ":" << endl;
        // calcolo l'unione degli stati coperti dalle pre-regioni di quel evento
        regions = (*(pre_regions)->find(event)).second;
        event_states = regions_union(regions);
        // calcolo gli stati coperti da pre-regioni essenziali
        // scorro tutte le regioni dell'evento estraendo solo quelle essenziali
        // cout << "aggiunta regioni essenziali dell'evento " << event << endl;
        for (auto reg : *(pre_regions->find(event)->second)) {
            if (essential_regions->find(reg) != essential_regions->end()) {
                /*cout << "regione essenziale: ";
                print(*reg);*/
                essential_regions_of_event->push_back(reg);
            }
        }
        essential_states = regions_union(essential_regions_of_event);

        // calcolo gli stati non ancora coperti
        uncovered_states = region_difference(*event_states, *essential_states);
        // println(*uncovered_states);
        // cout << "---------------" << endl;
        // cout << "evento: " << record.first << endl;
        /*cout << "tutti gli stati degli eventi: ";
        print(event_states);
        cout << "stati coperti da eventi essenziali: ";
        print(essential_states);*/
        /*cout << "stati non coperti da eventi essenziali: ";
        println(uncovered_states);
        cout << "---------------" << endl;*/

        if (total_uncovered_states == nullptr) {
            total_uncovered_states = uncovered_states;
        } else {
            if (total_uncovered_states->empty()) {
                total_uncovered_states = uncovered_states;
            } else {
                //				total_uncovered_states =
                //regions_union(total_uncovered_states, uncovered_states);
                auto tmp = regions_union(total_uncovered_states, uncovered_states);
                delete total_uncovered_states;
                total_uncovered_states = tmp;
                delete uncovered_states;
            }
        }

        // svuoto le variabili per ogni iterazione
        essential_regions_of_event->erase(essential_regions_of_event->begin(),
                                          essential_regions_of_event->end());
        delete event_states;
        delete essential_states;
    }
    delete essential_regions_of_event;
    delete uncovered_states;

    return total_uncovered_states;
}

int Place_irredundant_pn_creation_module::minimum_cost_search_with_label_costraints(
        set<int> states_to_cover, set<Region *> *used_regions, int last_best_cost,
        int father_cost, int level) {
    // per l'insieme degli stati non coperti devo trovare le possibili coperture
    // irridondanti
    // trovare quali insiemi di regioni non essenziali

    // metto insieme gli stati non coperti senza fare la divisione per evento
    // e aggiungo regioni non essenziali che coprono più stati possibili tra
    // quelli non coperti
    // mi fermo se supero il vecchio risultato migliore -> alla fine  l'insieme è
    // sicuramente irridondante

    //cout << "Regioni di mio padre: ";
    //println(*used_regions);

    auto candidate = reinterpret_cast<Region *>(-1);
    int cover_of_candidate;
    int temp_cover;
    // coppio il contenuto del padre per non sovrascrivere i dati con l'insiieme
    // delle regioni del figlio
    int cost_of_candidate;
    set<int> *new_states_to_cover;
    set<Region *> *new_states_used = nullptr;
    auto chosen_candidates = new set<Region *>(*used_regions);
    set<Region *> *temp_aggregation;
    int new_best_cost = last_best_cost; // uno dei sotto-rami potrebbe aver migliorato il
    // risultato, di conseguenza devo aggiornare la variabile
    // e non utilizzare il parametro in ingresso alla funzione
    // finchè ci sono candidati che aumentano la copertura
    while (true) {
        cover_of_candidate = -1;
        delete new_states_used;
        new_states_used = new set<Region *>(*used_regions);
        // scelta del prossimo candidato
        for (auto region : *not_essential_regions) {
            // la regione nuova non può essere un vecchio candidato
            if (chosen_candidates->find(region) == chosen_candidates->end()) {
                // controllo se l'insieme con il candidato è già stato calcolato o no
                // [faccio prima questo controllo dato che lo ritengo più leggero
                // dell'intersezione nella condizione successiva]
                temp_aggregation = new set<Region *>(*used_regions);
                temp_aggregation->insert(region);
                if (computed_paths_cache->find(*temp_aggregation) ==
                    computed_paths_cache->end()) {
                    // devo vedere la dimensione dell'intersezione tra gli stati da
                    // coprire e la regione
                    auto cover = regions_intersection(&states_to_cover, region);
                    temp_cover = static_cast<int>(cover->size());
                    delete cover;
                    if (temp_cover > cover_of_candidate) {
                        // cout << "candidato nuovo" << endl;
                        cover_of_candidate = temp_cover;
                        candidate = region;
                    } else {
                        //cout << "temp cover ha una copertura minore di cover of candidate" << endl;
                    }
                } else {
                    /*if(print_step_by_step_debug) {
                        cout << "insieme già presente nella cache: " << endl;
                        println(*temp_aggregation);
                    }*/
                }
                // ogni volta che entro nell'if alloco un nuovo spazio di conseguenza
                // devo deallocarlo
                delete temp_aggregation;
            }
        }
        //non è stato trovato il candidato
        if (cover_of_candidate == -1) {
            //cout << "candidato non trovato" << endl;
            break;
        }
        // cout << "cover of candidate: " << cover_of_candidate << endl;

        // non posso migliorare la copertura e ho la condizione di ec soddisfatta
        if (cover_of_candidate == 0) {
            if (all_events_have_ec_satisfied(*new_states_used)) {
                //cout << "break uscita. cover = 0 && all..." << endl;
                break;
            }
        }

        if (candidate == reinterpret_cast<Region *>(-1)) {
            //cout << "candidato non inizializzato" << endl;
            break;
        }

        // salvo il nuovo candidato
        chosen_candidates->insert(candidate);

        cost_of_candidate = (*cost_map)[&(*candidate)];
        // cout << "cost of candidate: " << cost_of_candidate << endl;
        // cout << "candidate cover: " << cover_of_candidate << endl;
        // non devo fare una chiamata ricorsiva se il costo è troppo grande oppure
        // se ho completato la copertura

        // non potrò trovare una soluzione migliore con il seguente candidato
        int current_cost = cost_of_candidate + father_cost;
        new_states_used->insert(candidate);

        if (print_step_by_step_debug) {
            //cout << "sono al livello " << level << endl;
            //cout << "l'insieme dei candidati è: " << endl;
            //println(*new_states_used);
        }

        // salvo il percorso nella cache per non ripeterlo
        computed_paths_cache->insert(*new_states_used);
        if (current_cost >= new_best_cost) {
            //non prosegue su questa strada
            //if (print_step_by_step_debug)
               // cout << "taglio" << endl;
        }
            // ho completato la copertura ed è meglio di quella precedente
        else if (states_to_cover.size() - cover_of_candidate == 0 && all_events_have_ec_satisfied(*new_states_used)) {
            new_best_cost = current_cost;
            // dealloco lo spazio vecchio per allocarne uno nuovo
            delete irredundant_regions;
            irredundant_regions = new set<Region *>(*new_states_used);
            //cout << "risultato trovato: ";
            //println(*irredundant_regions);
            //break;
        } else {
            // essedo già stato scelto il candidato e sapendo che devo fare la
            // chiamata ricorsiva devo calcolarmi il nuovo insieme di stati da coprire
            new_states_to_cover = region_difference(states_to_cover, *candidate);

            // chiamata ricorsiva per espandere ulteriormente la copertura con il
            // candidato scelto
            int cost = minimum_cost_search_with_label_costraints(*new_states_to_cover, new_states_used,
                                                                 new_best_cost, current_cost, level + 1);
            if (cost < new_best_cost) {
                new_best_cost = cost;
                // non devo salvarmi il risultato migliore dato che è già stato salvato
                // nella chiamata ricorsiva che ha fatto ritornare il costo minore
            }

            delete new_states_to_cover;
        }
    }

    // salvo il percorso nella cache per non ripeterlo: ho calcolato tutti i
    // sottorami di questo nodo per questo sono arrivato al return
    //cout << "regioni irridondanti correnti: ";
    //println(*used_regions);
    delete chosen_candidates;
    delete new_states_used;
    //cout << "new best cost: " << new_best_cost << endl;
    return new_best_cost;
}

// la mappa deve contenere solo i costi delle regioni non essenziali
void Place_irredundant_pn_creation_module::cost_map_filling() {
    // cout << "--------------------------------------------------- COST MAP "
    //       "FILLING --------------------------------------------"
    // << endl;

    for (auto record : *not_essential_regions_map) {
        for (Region *reg : *record.second) {
            // non è ancora stato calcolato il costo per la regione reg
            if (cost_map->find(&(*reg)) == cost_map->end()) {
                (*cost_map)[&(*reg)] = region_cost(&(*reg));
                //   cout << "Trovato regione non essenziale :";
                // print(*reg);
                //cout << "[" << &(*reg) << "] con il costo: " << (*cost_map)[&(*reg)]
                //   << endl;
            }
        }
    }
}

unsigned long Place_irredundant_pn_creation_module::region_cost(Region *reg) {
    // funzione di costo per minimizzare sia il numero di posti che il numero di
    // archi
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

    // funzione di costo per minimizzare solo il numero di posti
    return reg->size();
}

void Place_irredundant_pn_creation_module::calculate_irredundant_regions_map() {
    // algoritmo:
    // per ogni elemento di not_essential_regions_map se la regione è presente in
    // irredundant_regions

    //cout << "CALCULATE IRREDUNDANT REGION MAP_______________" << endl;
    irredundant_regions_map = new map<int, set<Region *> *>();
    for (auto record : *not_essential_regions_map) {
        for (auto reg : *record.second) {
            if (irredundant_regions->find(reg) != irredundant_regions->end()) {
                if (irredundant_regions_map->find(record.first) ==
                    irredundant_regions_map->end()) {
                    (*irredundant_regions_map)[record.first] = new set<Region *>();
                }

                //  cout << "REGION: " << reg << " AT " << record.first << endl;
                (*irredundant_regions_map)[record.first]->insert(reg);
            }
        }
    }

}
