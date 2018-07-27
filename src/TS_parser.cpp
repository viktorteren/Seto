//
// Created by ciuchino on 13/04/18.
//

#include "../include/TS_parser.h"

using namespace Utilities;

My_Map *ts_map;
int num_states, initial_state, num_events, num_events_after_splitting;
unsigned int num_transactions;
map<int, string> *aliases_map_number_name;
map<string, int> *aliases_map_name_number;
map<int, string> *aliases_map_state_number_name;
map<string, int> *aliases_map_state_name_number;
bool g_input;

void TS_parser::parse(string file) {
    g_input = false;
    // Open the file:
    ts_map = new My_Map();
    aliases_map_name_number = new map<string, int>();
    aliases_map_number_name = new map<int, string>();
    aliases_map_state_name_number = new map<string, int>();
    aliases_map_state_number_name = new map<int, string>();

    ifstream fin(file);
    if (!fin) {
        cout << "File non trovato." << endl;
        exit(0);
    }

    // il file è nel nostro formato ts
    if ((file[file.size() - 2]) == 't' && (file[file.size() - 1] == 's')) {
        parse_TS(fin);
        print_ts_dot_file(file, nullptr);

        if (print_step_by_step) {
            cout << "Numero di transizioni: " << num_transactions << endl;
            cout << "Numero di stati: " << num_states << endl;
            cout << "Numero di eventi: " << num_events << endl;
        }

    }
        //file nel fromato SIS .g
    else if (file[file.size() - 1] == 'g') {
        parse_SIS(fin);
        print_ts_dot_file(file, nullptr);
    } else {
        cout << "Estensione non supportata" << endl;
        exit(0);
    }

    // cout << "---------END OF PARSING----------" << endl;
}

void TS_parser::parse_TS(ifstream &fin) {
    // Read defining parameters:
    fin >> num_states;
    fin >> num_transactions;
    fin >> initial_state;
    int src, dst, ev;

    // aggiungo gli archi al grafo
    for (unsigned int i = 0; i < num_transactions; ++i) {
        fin >> src;
        fin >> dst;
        fin >> ev;
        // add_edge(vertex_array[src], vertex_array[dst], event(ev), g);
        // non c'è l'entry relativa all'evento ev
        if (ts_map->find(ev) == ts_map->end()) {
            (*ts_map)[ev] = Edges_list();
        }
        auto pair_ptr = new pair<int, int>(src, dst);
        (*ts_map)[ev].insert(pair_ptr);
    }
    num_events = static_cast<int>((*ts_map).size());
    fin.close();

    /*cout<<"DEBUG TS_MAP"<<endl;
    for(auto record:*ts_map){
            cout<<"evento:" <<record.first;
            for(auto tr:record.second){
                    cout<<"trans: "<< tr.first << ", " << tr.second <<endl;
            }
    }*/
}

void TS_parser::parse_SIS(ifstream &fin) {
    g_input = true;
    // Read defining parameters:
    pair<int, int> *pair_ptr;
    int max = 0;
    int max_state = 0;
    string temp, start, label, finish;
    int label_int, start_int, finish_int;
    bool exit;
    while (true) {
        fin >> temp;
        if (print_step_by_step_debug)
            cout << "temp: " << temp << endl;
        if (temp == "#" || temp == ".model") {
            if (print_step_by_step_debug)
                cout << "inizio commento" << endl;
            exit = false;
            while (!exit) {
                fin >> temp;
                if (temp == ".inputs") {
                    exit = true;
                }
                if (temp == ".outputs") {
                    exit = true;
                }
                if (temp == ".state") {
                    exit = true;
                }
                if (temp == ".marking") {
                    exit = true;
                }
                if (temp == ".end") {
                    exit = true;
                }
            }
        }
        if (temp == ".inputs") {
            if (print_step_by_step_debug)
                cout << "inputs" << endl;
            exit = false;
            while (!exit) {
                fin >> temp;
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".outputs") {
                    if (print_step_by_step_debug)
                        cout << "exit from inputs to outputs" << endl;
                    exit = true;
                }
                //si tratta di un'etichetta
                if (!exit) {
                    //add_new_label_with_alias(max, temp);
                    //max++;
                }
            }
        }
        if (temp == ".outputs") {
            if (print_step_by_step_debug)
                cout << "outputs" << endl;
            exit = false;
            while (!exit) {
                fin >> temp;
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".internal") {
                    exit = true;
                }
                if (temp == ".state") {
                    exit = true;
                }
                if (temp == ".dummy") {
                    exit = true;
                }
                //si tratta di un'etichetta
                if (!exit) {
                    // add_new_label_with_alias(max, temp);
                    //max++;
                }
            }
        }
        if (temp == ".internal") {
            if (print_step_by_step_debug)
                cout << "internal" << endl;
            exit = false;
            while (!exit) {
                fin >> temp;
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".dummy") {
                    exit = true;
                }
                if (temp == ".state") {
                    exit = true;
                }
                //si tratta di un'etichetta
                if (!exit) {
                    add_new_label_with_alias(max, temp);
                    max++;
                }
            }
        }
        if (temp == ".dummy") {
            if (print_step_by_step_debug)
                cout << "dummy" << endl;
            exit = false;
            while (!exit) {
                fin >> temp;
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".state") {
                    exit = true;
                }
                //si tratta di un'etichetta
                if (!exit) {
                    add_new_label_with_alias(max, temp);
                    max++;
                }
            }
        }
        if (temp == ".state") {
            fin >> temp;
            if (print_step_by_step_debug)
                cout << "temp: " << temp << endl;
            if (temp == "graph") {
                if (print_step_by_step_debug) {
                    cout << "state graph" << endl;
                }
                while (true) {
                    fin >> start;
                    if (start == ".marking") {
                        temp = start;
                        if (print_step_by_step_debug)
                            cout << "uscita da state graph a marking" << endl;
                        break;
                    }
                    fin >> label;

                    if (aliases_map_name_number->find(label) == aliases_map_name_number->end()) {
                        add_new_label_with_alias(max, label);
                        max++;
                        if (print_step_by_step_debug) {
                            cout << "MAX " << max << endl;
                        }
                    }

                    fin >> finish;
                    //lo stato start non è presente nella mappa
                    if (aliases_map_state_name_number->find(start) == aliases_map_state_name_number->end()) {
                        add_new_state_with_alias(max_state, start);
                        max_state++;
                    }
                    if (aliases_map_state_name_number->find(finish) == aliases_map_state_name_number->end()) {
                        add_new_state_with_alias(max_state, finish);
                        max_state++;
                    }
                    label_int = (*aliases_map_name_number)[label];

                    if (print_step_by_step_debug) {
                        for (auto el: *aliases_map_name_number)
                            cout << "NAME " << el.first << endl;
                        cout << "label:" << label << endl;
                        cout << "label_int:" << label_int << endl;
                    }


                    start_int = (*aliases_map_state_name_number)[start];
                    finish_int = (*aliases_map_state_name_number)[finish];
                    if (ts_map->find(label_int) == ts_map->end()) {
                        (*ts_map)[label_int] = Edges_list();
                    }
                    pair_ptr = new pair<int, int>(start_int, finish_int);
                    (*ts_map)[label_int].insert(pair_ptr);
                    if (print_step_by_step_debug) {
                        cout << start << " -> " << finish << " con etichetta:" << label << endl;
                    }
                    num_transactions++;
                }
            }
        }
        if (temp == ".marking") {
            fin >> temp;
            if (print_step_by_step_debug)
                cout << "temp dopo marking: " << temp << endl;
            //stato iniziale dentrro le parentesi graffe: {s0}
            //cout << "substring: " << temp.substr(1, temp.size()-2) << endl;
            initial_state = (*aliases_map_state_name_number)[temp.substr(1, temp.size() - 2)];
            if (print_step_by_step_debug) {
                cout << "stato iniziale: " << initial_state << endl;
            }
        }
        if (temp == ".end") {
            break;
        }
    }
    num_events = static_cast<int>(aliases_map_number_name->size());
    num_states = static_cast<int>(aliases_map_state_number_name->size());
    if (print_step_by_step) {
        cout << "Numero di transizioni: " << num_transactions << endl;
        cout << "Numero di stati: " << num_states << endl;
        cout << "Numero di eventi: " << num_events << endl;
    }
}

void TS_parser::add_new_label_with_alias(int num, string name) {
    num_events++;
    (*aliases_map_number_name)[num] = name;
    (*aliases_map_name_number)[name] = num;
    if (print_step_by_step_debug) {
        cout << "aggiunta coppia alias etichetta: " << num << " " << name << endl;
    }
    if (print_step_by_step_debug) {
        cout << "num events: " << num_events << endl;
        for (auto record: *aliases_map_number_name) {
            cout << record.first << " -> " << record.second << endl;
        }
    }
}

void TS_parser::add_new_state_with_alias(int num, string name) {
    (*aliases_map_state_number_name)[num] = name;
    (*aliases_map_state_name_number)[name] = num;
    if (print_step_by_step_debug) {
        cout << "aggiunta coppia alias stato: " << num << " " << name << endl;
    }
}

