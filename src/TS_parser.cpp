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
  }
  // il file è nel formato dot
  else if ((file[file.size() - 3] == 'd' && (file[file.size() - 2]) == 'o' &&
            (file[file.size() - 1] == 't'))) {
    parse_DOT(fin);
  }
  //file nel fromato SIS .g
  else if (file[file.size() - 1] == 'g') {
    parse_SIS(fin);
    print_ts_dot_file(file, nullptr);
  }else {
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

void TS_parser::parse_DOT(ifstream &fin) {
  cout << "--------------------.dot FILE PARSING------------------------"
       << endl;
  string temp1, temp2, temp3, temp4, tempOld, label, src, dst;
  int ev;
  bool found;
  while (fin) {
    found = true;
    fin >> temp1;
    if (fin)
      fin >> temp2;
    if (fin)
      fin >> temp3;
    if (fin)
      fin >> temp4;
    if (temp1.compare("}") == 0) {
      break;
    }
    if (temp2.compare("}") == 0) {
      break;
    }
    if (temp3.compare("}") == 0) {
      break;
    }
    if (temp1.compare("->") == 0) {
      // cout << "temp1" << endl;
      label = temp3;
      src = tempOld;
      dst = temp2;
    }
    // caso ideale
    else if (temp2.compare("->") == 0) {
      // cout << "temp2" << endl;
      src = temp1;
      dst = temp3;
      label = temp4;
    } else if (temp3.compare("->") == 0) {
      // cout << "temp3" << endl;
      src = temp2;
      dst = temp4;
      fin >> temp4;
      label = temp4;
    } else if (temp4.compare("->") == 0) {
      // cout << "temp4" << endl;
      src = temp3;
      fin >> dst;
      fin >> label;
    } else {
      found = false;
    }
    // salvataggio stato iniziale
    if (found) {
      if (label[0] != '[') {
        // caso in cui ci sia il carattere ';' unito al numero della
        // destinazione
        if (dst[dst.size() - 1] == ';') {
          dst = dst.substr(0, dst.size() - 1);
        }
        initial_state = stoi(dst);
        found = false;
      }
      //cout << "SRC: " << src << endl;
      //cout << "DST: " << dst << endl;
    }

    if (found) {
      unsigned long lower = 0, upper = label.size();
      for (unsigned int i = 0; i < label.size(); i++) {
        if (label[i] == '"') {
          lower = i;
          // cout << "found "<< i << endl;
          break;
        }
      }
      for (unsigned long i = lower + 1; i < label.size(); i++) {
        if (label[i] == '"') {
          upper = i;
          // cout << "found " << i << endl;
          break;
        }
      }
      string temp = label.substr(lower + 1);
      unsigned long diff = upper - lower - 1;
      temp = temp.substr(0, diff);
     // cout << "prova: " << label << endl << "in: " << temp << endl;
      ev = stoi(temp);
      if (ts_map->find(ev) == ts_map->end()) {
        (*ts_map)[ev] = Edges_list();
      }
      auto pair_ptr = new pair<int, int>(stoi(src), stoi(dst));
      (*ts_map)[ev].insert(pair_ptr);
    }
    if (temp4.compare("}") == 0) {
      break;
    }
    tempOld = temp4;
  }
  num_events = static_cast<int>((*ts_map).size());
  fin.close();
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
    while(true){
        fin >> temp;
        if(print_step_by_step_debug)
            cout << "temp: " << temp << endl;
        if(temp == "#" || temp == ".model"){
            if(print_step_by_step_debug)
                cout << "inizio commento" << endl;
            exit = false;
            while(!exit){
                fin >> temp;
                if(temp == ".inputs"){
                    exit = true;
                }
                if(temp == ".outputs"){
                    exit = true;
                }
                if(temp == ".state"){
                    exit = true;
                }
                if(temp == ".marking"){
                     exit = true;
                }
                if(temp == ".end"){
                    exit = true;
                }
            }
        }
        if(temp == ".inputs"){
            if(print_step_by_step_debug)
                cout << "inputs" <<endl;
            exit = false;
            while(!exit){
                fin >> temp;
                if(print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if(temp == ".outputs"){
                    if(print_step_by_step_debug)
                        cout << "exit from inputs to outputs" << endl;
                    exit = true;
                }
                //si tratta di un'etichetta
                if(!exit){
                    //add_new_label_with_alias(max, temp);
                    //max++;
                }
            }
        }
        if(temp == ".outputs"){
            if(print_step_by_step_debug)
                cout << "outputs" <<endl;
            exit = false;
            while(!exit){
                fin >> temp;
                if(print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if(temp == ".internal"){
                    exit = true;
                }
                if(temp==".state"){
                    exit = true;
                }
                if(temp==".dummy"){
                    exit = true;
                }
                //si tratta di un'etichetta
                if(!exit){
                   // add_new_label_with_alias(max, temp);
                    //max++;
                }
            }
        }
        if(temp == ".internal"){
            if(print_step_by_step_debug)
                cout << "internal" <<endl;
            exit = false;
            while(!exit){
                fin >> temp;
                if(print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if(temp == ".dummy"){
                    exit = true;
                }
                if(temp == ".state"){
                    exit = true;
                }
                //si tratta di un'etichetta
                if(!exit){
                    add_new_label_with_alias(max, temp);
                    max++;
                }
            }
        }
        if(temp == ".dummy"){
            if(print_step_by_step_debug)
                cout << "dummy" <<endl;
            exit = false;
            while(!exit){
                fin >> temp;
                if(print_step_by_step_debug)
                    cout << "temp: " << temp << endl;
                if(temp == ".state"){
                    exit = true;
                }
                //si tratta di un'etichetta
                if(!exit){
                    add_new_label_with_alias(max, temp);
                    max++;
                }
            }
        }
        if(temp == ".state"){
            fin >> temp;
            if(print_step_by_step_debug)
                cout << "temp: " << temp << endl;
            if(temp == "graph"){
                if(print_step_by_step_debug) {
                    cout << "state graph" << endl;
                }
                while(true){
                    fin >> start;
                    if(start == ".marking"){
                        temp = start;
                        if(print_step_by_step_debug)
                            cout << "uscita da state graph a marking" << endl;
                        break;
                    }
                    fin >> label;

                    if(aliases_map_name_number->find(label) == aliases_map_name_number->end()){
                        add_new_label_with_alias(max, label);
                        if(print_step_by_step_debug){
                        max++;
                        cout<<"MAX "<<max<<endl;}
                    }

                    fin >> finish;
                    //lo stato start non è presente nella mappa
                    if(aliases_map_state_name_number->find(start) == aliases_map_state_name_number->end()){
                        add_new_state_with_alias(max_state, start);
                        max_state++;
                    }
                    if(aliases_map_state_name_number->find(finish) == aliases_map_state_name_number->end()){
                        add_new_state_with_alias(max_state, finish);
                        max_state++;
                    }
                    label_int = (*aliases_map_name_number)[label];

                    if(print_step_by_step_debug) {
                        for (auto el: *aliases_map_name_number)
                            cout << "NAME " << el.first << endl;
                        cout<<"label:"<<label <<endl;
                        cout<<"label_int:"<<label_int <<endl;
                    }


                    start_int = (*aliases_map_state_name_number)[start];
                    finish_int = (*aliases_map_state_name_number)[finish];
                    if (ts_map->find(label_int) == ts_map->end()) {
                        (*ts_map)[label_int] = Edges_list();
                    }
                    pair_ptr = new pair<int, int>(start_int, finish_int);
                    (*ts_map)[label_int].insert(pair_ptr);
                    if(print_step_by_step_debug){
                        cout << start << " -> " << finish << " con etichetta:" << label << endl;
                    }
                }
            }
        }
        if(temp == ".marking"){
            fin >> temp;
            if(print_step_by_step_debug)
            cout << "temp dopo marking: "<< temp << endl;
            //stato iniziale dentrro le parentesi graffe: {s0}
            //cout << "substring: " << temp.substr(1, temp.size()-2) << endl;
            initial_state = (*aliases_map_state_name_number)[temp.substr(1, temp.size()-2)];
            if(print_step_by_step_debug){
                cout << "stato iniziale: " << initial_state << endl;
            }
        }
        if(temp == ".end"){
            break;
        }
    }
}

void TS_parser::add_new_label_with_alias(int num, string name){
    (*aliases_map_number_name)[num] = name;
    (*aliases_map_name_number)[name] = num;
    if(print_step_by_step_debug){
        cout << "aggiunta coppia alias etichetta: " << num << " " << name << endl;
    }
}

void TS_parser::add_new_state_with_alias(int num,string name){
    (*aliases_map_state_number_name)[num] = name;
    (*aliases_map_state_name_number)[name] = num;
    if(print_step_by_step_debug){
        cout << "aggiunta coppia alias stato: " << num << " " << name << endl;
    }
}

