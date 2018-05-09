//
// Created by ciuchino on 13/04/18.
//

#include "../include/TS_parser.h"

My_Map* ts_map;
int num_states, num_transactions, initial_state,num_events;

//todo: aggiungere parser dot

void TS_parser::parse(string file) {

    // Open the file:

    ts_map = new My_Map();

	ifstream fin(file);
	if(!fin){
		cout << "The file wasn't found." << endl;
		exit(0);
	}

    //il file è nel nostro formato ts
    if((file[file.size()-2]) == 't' && (file[file.size()-1] == 's')){
	    parse_TS(fin);
    }
    //il file è nel formato dot
    else if((file[file.size()-3] == 'd' && (file[file.size()-2]) == 'o' && (file[file.size()-1] == 't'))){
		parse_DOT(fin);
    }
    else {
    	cout << "The file extension is not supported" << endl;
    	exit(0);
    }
	cout << "---------END OF PARSING----------" << endl;
}


void TS_parser::parse_TS(ifstream& fin){
	// Read defining parameters:
	fin >> num_states;
	fin >> num_transactions;
	//todo: è proprio vero che abbiamo un solo stato iniziale???
	fin >> initial_state;
	int src, dst, ev;

	//aggiungo gli archi al grafo
	for (int i = 0; i < num_transactions; ++i) {
		fin >> src;
		fin >> dst;
		fin >> ev;
		// add_edge(vertex_array[src], vertex_array[dst], event(ev), g);
		//non c'è l'entry relativa all'evento ev
		if (ts_map->find(ev) == ts_map->end()) {
			(*ts_map)[ev] = Lista_archi();
		}
		(*ts_map)[ev].push_back(make_pair(src, dst));
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

void TS_parser::parse_DOT(ifstream& fin){
	cout << "--------------------.dot FILE PARSING------------------------" << endl;
	string temp1, temp2, temp3, temp4, tempOld, label, src, dst;
	int ev;
	bool found;
	while(fin){
		found = true;
		fin >> temp1;
		if(fin)
			fin >> temp2;
		if(fin)
			fin >> temp3;
		if(fin)
			fin >> temp4;
		if(temp1.compare("}") == 0){
			break;
		}
		if(temp2.compare("}") == 0){
			break;
		}
		if(temp3.compare("}") == 0){
			break;
		}
		if(temp1.compare("->") == 0){
			//cout << "temp1" << endl;
			label = temp3;
			src = tempOld;
			dst = temp2;
		}
			//caso ideale
		else if(temp2.compare("->") == 0){
			//cout << "temp2" << endl;
			src = temp1;
			dst = temp3;
			label = temp4;
		}
		else if(temp3.compare("->") == 0){
			//cout << "temp3" << endl;
			src = temp2;
			dst = temp4;
			fin >> temp4;
			label = temp4;
		}
		else if(temp4.compare("->") == 0){
			//cout << "temp4" << endl;
			src = temp3;
			fin >> dst;
			fin >> label;
		}
		else{
			found = false;
		}
		//salvataggio stato iniziale
		if(found){
			if(label[0] != '['){
				//caso in cui ci sia il carattere ';' unito al numero della destinazione
				if(dst[dst.size()-1] == ';'){
					dst = dst.substr(0, dst.size()-1);
				}
				initial_state = stoi (dst);
				found = false;
			}
			cout << "SRC: " << src << endl;
			cout << "DST: " << dst << endl;
		}

		if(found){
			unsigned long lower=0, upper=label.size();
			for(int i= 0; i< label.size();i++){
				if(label[i] == '"'){
					lower = i;
					//cout << "found "<< i << endl;
					break;
				}
			}
			for(int i= lower+1; i< label.size();i++){
				if(label[i] == '"'){
					upper = i;
					//cout << "found " << i << endl;
					break;
				}
			}
			string temp = label.substr(lower+1);
			int diff = upper - lower-1;
			temp = temp.substr(0, diff);
			cout << "prova: " <<label << endl << "in: " << temp << endl;
			ev = stoi(temp);
			if (ts_map->find(ev) == ts_map->end()) {
				(*ts_map)[ev] = Lista_archi();
			}
			(*ts_map)[ev].push_back(make_pair(stoi(src), stoi(dst)));
		}
		if(temp4.compare("}") == 0){
			break;
		}
		tempOld = temp4;
	}
	num_events = static_cast<int>((*ts_map).size());
	fin.close();
}