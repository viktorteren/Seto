/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/TS_parser.h"

using namespace Utilities;

My_Map *ts_map;
unsigned int num_states;
int initial_state, num_events_before_label_splitting, num_events_after_splitting;
unsigned int num_transactions;
map<int, string> *aliases_map_number_name;
map<string, int> *aliases_map_name_number;
map<int, string> *aliases_map_state_number_name;
map<string, int> *aliases_map_state_name_number;
bool g_input;
set<string> inputs;
set<string> outputs;
set<string> internals;
set<string> dummies;

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
        cout << "File not found." << endl;
        exit(1);
    }

    // il file è nel nostro formato ts
    if ((file[file.size() - 2]) == 't' && (file[file.size() - 1] == 's')) {
        parse_TS(fin);
        if(ts_output){
            if(aut_output){
                print_ts_aut_file(file, nullptr);
            }
            else{
                print_ts_dot_file(file, nullptr);
            }
            exit(0);
        }

        if (print_step_by_step) {
            cout << "Number of transitions: " << num_transactions << endl;
            cout << "Number of states: " << num_states << endl;
            cout << "Number of events: " << num_events_before_label_splitting << endl;
        }

    }
    //file nel formato SIS .g
    else if (file[file.size() - 1] == 'g') {
        parse_SIS(fin);
        if(ts_output){
            if(aut_output){
                print_ts_aut_file(file, nullptr);
            }
            else{
                print_ts_dot_file(file, nullptr);
            }
            exit(0);
        }
    } else {
        cout << "Not supported extension" << endl;
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

    // add the arcs to the graph
    for (unsigned int i = 0; i < num_transactions; ++i) {
        fin >> src;
        fin >> dst;
        fin >> ev;
        if (ts_map->find(ev) == ts_map->end()) {
            (*ts_map)[ev] = Edges_list();
        }
        auto pair_ptr = new pair<int, int>(src, dst);
        (*ts_map)[ev].insert(pair_ptr);
    }
    num_events_before_label_splitting = static_cast<int>((*ts_map).size());
    fin.close();
}

void TS_parser::parse_SIS(ifstream &fin) {
    g_input = true;
    // Read defining parameters:
    pair<int, int> *pair_ptr;
    int max = 0;
    int max_state = 0;
    string temp, start, label, finish, previous_temp;
    int label_int, start_int, finish_int;
    bool exit_cycle;
    temp = "";
    while (true) {
        previous_temp = temp;
        fin >> temp;
        if(temp == previous_temp){
            if(temp == ".end"){
                cout << "syntax error" <<endl;
                exit(1);
            }
        }
        if (print_step_by_step_debug)
            cout << "temp: " << temp << endl;
        if (temp == "#" || temp == ".model") {
            if (print_step_by_step_debug)
                cout << "Start of comment" << endl;
            exit_cycle = false;
            while (!exit_cycle) {
                previous_temp = temp;
                fin >> temp;
                check_wrong_end(previous_temp,temp);
                if (temp == ".inputs") {
                    exit_cycle = true;
                }
                if (temp == ".outputs") {
                    exit_cycle = true;
                }
                if (temp == ".internal") {
                    exit_cycle = true;
                }
                if (temp == ".dummy") {
                    exit_cycle = true;
                }
                if (temp == ".state") {
                    exit_cycle = true;
                }
                if (temp == ".marking") {
                    exit_cycle = true;
                }
                if (temp == ".end") {
                    exit_cycle = true;
                }
            }
        }
        if (temp == ".inputs") {
            if (print_step_by_step_debug)
                cout << "inputs" << endl;
            exit_cycle = false;
            while (!exit_cycle) {
                previous_temp = temp;
                fin >> temp;
                check_wrong_end(previous_temp,temp);
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".outputs") {
                    if (print_step_by_step_debug)
                        cout << "exit_cycle from inputs to outputs" << endl;
                    exit_cycle = true;
                }
                if(temp == ".state"){
                    cout << "outputs row missing" << endl;
                    exit_cycle = true;
                    //abort();
                }
                //si tratta di un'etichetta
                if (!exit_cycle) {
                    inputs.insert(temp);
                    //add_new_label_with_alias(max, temp);
                    //max++;
                }
            }
        }
        if (temp == ".outputs") {
            if (print_step_by_step_debug)
                cout << "outputs" << endl;
            exit_cycle = false;
            while (!exit_cycle) {
                previous_temp = temp;
                fin >> temp;
                check_wrong_end(previous_temp,temp);
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".internal") {
                    exit_cycle = true;
                }
                if (temp == ".state") {
                    exit_cycle = true;
                }
                if (temp == ".dummy") {
                    exit_cycle = true;
                }
                //it's a label
                if (!exit_cycle) {
                    outputs.insert(temp);
                    // add_new_label_with_alias(max, temp);
                    //max++;
                }
            }
        }
        if (temp == ".internal") {
            if (print_step_by_step_debug)
                cout << "internal" << endl;
            exit_cycle = false;
            while (!exit_cycle) {
                previous_temp = temp;
                fin >> temp;
                check_wrong_end(previous_temp,temp);
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".dummy") {
                    exit_cycle = true;
                }
                if (temp == ".state") {
                    exit_cycle = true;
                }
                //it's a label
                if (!exit_cycle) {
                    internals.insert(temp);
                    //add_new_label_with_alias(max, temp);
                    //max++;
                }
            }
        }
        if (temp == ".dummy") {
            if (print_step_by_step_debug)
                cout << "dummy" << endl;
            exit_cycle = false;
            while (!exit_cycle) {
                previous_temp = temp;
                fin >> temp;
                check_wrong_end(previous_temp,temp);
                if (print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if (temp == ".state") {
                    exit_cycle = true;
                }
                //it's a label
                if (!exit_cycle) {
                    dummies.insert(temp);
                    //add_new_label_with_alias(max, temp);
                    //max++;
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
                    if(start != "#") {
                        if (start == ".marking") {
                            temp = start;
                            if (print_step_by_step_debug)
                                cout << "exit_cycle from state graph to marking" << endl;
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
                        //the start state is not available in the map
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
                            for (const auto &el: *aliases_map_name_number)
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
                            cout << start << " -> " << finish << " with label:" << label << endl;
                        }
                        num_transactions++;
                    }
                    else{
                        while ( fin.peek() != '\n'){
                            string temp;
                            fin >> temp;
                        }
                    }
                }
            }
        }
        if (temp == ".marking") {
            previous_temp = temp;
            fin >> temp;
            check_wrong_end(previous_temp,temp);
            if (print_step_by_step_debug)
                cout << "time after marking: " << temp << endl;
            //initial state inside brackets: {s0}
            //cout << "substring: " << temp.substr(1, temp.size()-2) << endl;
            initial_state = (*aliases_map_state_name_number)[temp.substr(1, temp.size() - 2)];
            if (print_step_by_step_debug) {
                cout << "initial state: " << initial_state << endl;
            }
        }
        if (temp == ".end") {
            break;
        }
    }
    num_events_before_label_splitting = static_cast<int>(aliases_map_number_name->size());
    num_states = static_cast<int>(aliases_map_state_number_name->size());
    if (print_step_by_step) {
        cout << "Number of transitions: " << num_transactions << endl;
        cout << "Number of states: " << num_states << endl;
        cout << "Number of events: " << num_events_before_label_splitting << endl;
    }
}

void TS_parser::add_new_label_with_alias(int num, const string& name) {
    num_events_before_label_splitting++;
    (*aliases_map_number_name)[num] = name;
    (*aliases_map_name_number)[name] = num;
    if (print_step_by_step_debug) {
        cout << "new label's alias added: " << num << " " << name << endl;
    }
    if (print_step_by_step_debug) {
        cout << "num events: " << num_events_before_label_splitting << endl;
        for (const auto& record: *aliases_map_number_name) {
            cout << record.first << " -> " << record.second << endl;
        }
    }
}

void TS_parser::add_new_state_with_alias(int num, const string& name) {
    (*aliases_map_state_number_name)[num] = name;
    (*aliases_map_state_name_number)[name] = num;
    if (print_step_by_step_debug) {
        cout << "added couple (alias, state): " << num << " " << name << endl;
    }
}

 void TS_parser::check_wrong_end(const string& previous_temp, const string& temp){
     if(temp == previous_temp){
         if(temp == ".end"){
             cout << "syntax error" <<endl;
             exit(1);
         }
     }
}

