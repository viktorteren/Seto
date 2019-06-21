//
// Created by ciuchino on 18/04/18.
//

#include "../include/Utilities.h"

bool print_step_by_step;
bool print_step_by_step_debug;
bool decomposition;
bool decomposition_debug;
bool decomposition_output;
bool log_file;
map<int, Region*>* aliases_region_pointer;
map<Region*, int>* aliases_region_pointer_inverted;
int max_alias_decomp;
int num_clauses;
map<pair<Region*, Region*>, bool> *overlaps_cache;

namespace Utilities {
// Region = set<int> ->ritorna un insieme di stati
    set<int> *regions_union(vector<Region *> *vec) {
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region : *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int> *regions_union(set<Region *> *vec) {
        // cout << "region union" << endl;
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region : *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                // cout << "Stato: " << *it << endl;
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int> *regions_union(Region *first, Region *second) {
        auto all_states = new Region();
        int size;
        Region::iterator it;
        it = first->begin();
        size = static_cast<int>(first->size());
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }
        it = second->begin();
        size = static_cast<int>(second->size());
        for (int i = 0; i < size; ++i) {
            all_states->insert(*it);
            ++it;
        }

        return all_states;
    }

    map<int, set<int> *> *do_regions_intersection(map<int, set<Region *> *> *regions) {

        auto pre_regions_intersection = new map<int, set<int> *>;

        //set<Region*>::iterator it;
        bool state_in_intersecton = true;

        // per ogni evento
        for (auto item : *regions) {
            (*pre_regions_intersection)[item.first] = new Region();
            for (auto state : **((*item.second).begin())) {
                state_in_intersecton = true;
                for (auto set : *item.second) {
                    if (set->find(state) == set->end()) { // non l'ho trovato
                        state_in_intersecton = false;
                        break;
                    }
                }
                if (state_in_intersecton) {
                    pre_regions_intersection->at(item.first)->insert(state);
                }
            }
        }
        /*cout << "intersezione****************" << endl;
        for (auto el : *pre_regions_intersection) {
          cout << "event " << el.first << endl;
          println(*el.second);
        }*/

        return pre_regions_intersection;
    }

/*map<int, set<int> *> *do_regions_intersection2(map<int, set<Region*> *>
*regions){

    auto pre_regions_intersection = new map<int, set<int> *>;

    std::vector<Region>::iterator it;
    bool state_in_intersecton=true;

    //per ogni evento
    for (auto item: *regions) {
        (*pre_regions_intersection)[item.first]=new Region();
        for (auto state: *(*(item.second)->begin()) ) {
            state_in_intersecton=true;
            for(auto set:*item.second){
                if(set->find(state)==set->end()) {//non l'ho trovato

                    state_in_intersecton = false;
                    break;
                }
            }
            if(state_in_intersecton){
                pre_regions_intersection->at(item.first)->insert(state);
            }
        }
    }
    cout << "intersezione****************" << endl;
    for(auto el:*pre_regions_intersection){
        cout<<"event "<<el.first<<endl;
        println(*el.second);
    }

    return pre_regions_intersection;
}*/

    set<int> *regions_intersection(set<Region *> *regions) {
        //println(*regions);

        auto pre_regions_intersection = new set<int>();
        bool state_in_intersecton;

        if (*regions->begin() == nullptr)
            return pre_regions_intersection;

        for (auto state : **regions->begin()) {
            state_in_intersecton = true;
            for (auto s : *regions) {
                if (s->find(state) == s->end()) { // non l'ho trovato
                    state_in_intersecton = false;
                    break;
                }
            }
            if (state_in_intersecton) {
                pre_regions_intersection->insert(state);
            }
        }

        return pre_regions_intersection;
    }

    set<int> *regions_intersection(Region *first, Region *second) {
        auto intersection = new set<int>();
        for (auto state : *first) {
            if (second->find(state) !=
                second->end()) { // trovo lo stato (appartiene a entrambe)
                intersection->insert(state);
            }
        }
        return intersection;
    }

    void print(Region &region) {
        int pos = 0;
        auto size = static_cast<int>(region.size());
        for (auto state : region) {
            pos++;
            cout << state;
            if (pos != size) {
                cout << ",";
            }
        }
    }

    void println(Region &region) {
        print(region);
        cout << endl;
    }

    void print_place(int pos, Region &region) {
        cout << "r" << pos << ": { ";
        print(region);
        cout << " } ";
    }

    void print_transactions() {
        cout << "Transazioni: " << endl;
        for (unsigned int i = 0; i < num_transactions; i++) {
            cout << "t" << i;
            if (i != num_transactions - 1)
                cout << ",";
        }
    }

    set<int> *region_difference(set<int> &first, set<int> &second) {
        auto s = new set<int>();
        for (auto state : first) {
            if (second.find(state) == second.end()) {
                s->insert(state);
            }
        }
        return s;
    }

    vector<Region> *copy_map_to_vector(map<int, vector<Region> *> *map) {
        auto input = new set<Region>();
        for (auto record : *map) {
            for (const auto &region : *record.second) {
                input->insert(region);
            }
        }
        vector<Region> *vec = new vector<Region>(input->size());
        std::copy(input->begin(), input->end(), vec->begin());
        delete input;
        return vec;
    }

    vector<Region*> *copy_map_to_vector3(map<int, vector<Region> *> *map) {
        auto vec = new vector<Region *>();
        for (auto record : *map) {
            for (const auto& region : *record.second) {
                auto r=new set<int>(region);
                if (!contains(vec, r)) {
                    vec->push_back(r);
                }
            }
        }

        return vec;
    }

    vector<Region *> *copy_map_to_vector2(map<int, set<Region *> *> *map) {
        auto vec = new vector<Region *>();
        for (auto record : *map) {
            for (auto region : *record.second) {
                if (!contains(vec, region)) {
                    vec->push_back(region);
                }
            }
        }

        return vec;
    }

    set<Region *> *copy_map_to_set(map<int, set<Region *> *> *map) {
        auto input = new set<Region *>();
        for (auto record : *map) {
            for (auto region : *record.second) {
                //cout << "region: ";
                //println(*region);
                if (!contains(input, region))
                    input->insert(region);
            }
        }
        return input;
    }

    set<Region *> *copy_map_to_set(map<int, Region *> *map) {
        auto input = new set<Region *>();
        for (auto record : *map) {
            auto region = record.second;
            if (!contains(input, region))
                input->insert(region);
        }
        return input;
    }

    bool is_bigger_than(Region *region, set<int> *region2) {
        if (region->size() > region2->size()) {
            for (auto elem : *region2) {
                // nella regione non trovo un elem
                if (region->find(elem) == region->end()) {
                    //cout << "****FALSE ";
                    // print(*region), cout << " is not bigger than ";
                    //println(*region2);
                    return false;
                }
            }
        } else if (region->size() <= region2->size())
            return false;

        // nella regione trovo tutti gli stati della reg2
        // cout << "****TRUE";
        //print(*region), cout << " is bigger than ";
        //println(*region2);
        return true;
    }

    bool is_bigger_than_or_equal_to(Region *region, set<int> *intersection) {

        if (region->size() >= intersection->size()) {
            //cout << "TRUE**************" << endl;
            return true;
        }

        for (auto elem : *intersection) {
            // nella regione non trovo un elem delll'intersez
            if (region->find(elem) == region->end()) {
                //cout << "FALSE**************" << endl;
                return false;
            }
        }

        // nella regione trovo tutti gli stati dell'intersezione
        // cout << "TRUE**************" << endl;
        return true;
    }

    bool are_equal(Region *region1, Region *region2) {
        return are_equal(*region1, *region2);
    }

    bool are_equal(const Region& region1, Region region2) {
        if (region1.size() != region2.size())
            return false;
        for (auto elem : region1) {
            if (region2.find(elem) == region2.end()) {
                return false;
            }
        }
        return true;
    }

    bool contains(set<Region *> *set, Region *region) {
        for (auto elem : *set) {
            if (are_equal(elem, region)) {
                return true;
            }
        }
        return false;
    }


    bool contains(vector<Region *> *vector, Region *region) {
        for (auto elem : *vector) {
            if (are_equal(elem, region)) {
                return true;
            }
        }
        return false;
    }

    void print_ts_dot_file(string file_path, map<int, int> *aliases) {
        //cout << "CREATION OF INPUT .DOT FILE" << endl;
        string output_name = std::move(file_path);
        string in_name;
        while (output_name[output_name.size() - 1] != '.') {
            output_name = output_name.substr(0, output_name.size() - 1);
        }
        output_name = output_name.substr(0, output_name.size() - 1);
        unsigned long lower = 0;
        for (unsigned long i = output_name.size() - 1; i > 0; i--) {
            if (output_name[i] == '/') {
                lower = i;
                break;
            }
        }
        in_name = output_name.substr(lower + 1, output_name.size());
        // cout << "out name: " << in_dot_name << endl;

        if (aliases != nullptr) {
            in_name += "_ECTS";
            output_name = output_name + "_ECTS.dot";
        } else {
            output_name = output_name + ".dot";
        }

        ofstream fout(output_name);
        fout << "digraph ";
        fout << in_name;
        fout << "{\n";
        fout << "\tlabel=\"(name=" << in_name << ",n=" << num_states
             << ",m=" << num_transactions << ")\";\n";
        fout << "\t_nil [style = \"invis\"];\n";
        fout << "\tnode [shape = doublecircle]; ";
        if(g_input){
            fout << (*aliases_map_state_number_name)[initial_state] << ";\n";
        }
        else{
            fout << initial_state << ";\n";
        }
        fout << "\tnode [shape = circle];\n";
        fout << "\t_nil -> ";
        if(g_input){

            fout << (*aliases_map_state_number_name)[initial_state] << ";\n";
        }
        else{
            fout << initial_state << ";\n";
        }

        map<int, int> *alias_counter = nullptr;
        map<int, int> *alias_counter_original = nullptr;
        if (aliases != nullptr) {
            alias_counter = new map<int, int>();
            alias_counter_original = new map<int, int>();
            for (auto al:*aliases) {
                (*alias_counter)[al.second] = 0;
                (*alias_counter_original)[al.second] = 0;
            }
            for (auto al:*aliases) {
                (*alias_counter)[al.second]++;
                (*alias_counter_original)[al.second]++;
            }

            /*cout << "alias counter" << endl;
            for (auto r: *alias_counter)
                cout << r.first << "->" << r.second << endl;*/

        }

        for (const auto& rec : *ts_map) {
            //l'evento è un alias
            int label;
            string to_add;
            if (aliases != nullptr && aliases->find(rec.first) != aliases->end()) {
                label = aliases->at(rec.first);
                for (int i = 0; i < (*alias_counter_original)[label] - (*alias_counter)[label] + 1; ++i) {
                    to_add += "'";
                }
                (*alias_counter)[label]--;
                //cout << " ev " << rec.first << "to add " << to_add << endl;
            } else {
                label = rec.first;
            }
            for (auto edge : rec.second) {
                if(!g_input){
                    fout << "\t" << edge->first << "->" << edge->second << "[label=\""
                         << label << to_add << "\"];\n";
                }
                else{
                    fout << "\t" << (*aliases_map_state_number_name)[edge->first] << "->" << (*aliases_map_state_number_name)[edge->second] << "[label=\""
                         << (*aliases_map_number_name)[label] << to_add << "\"];\n";
                }

            }

        }

        delete alias_counter;
        delete alias_counter_original;

        fout << "}\n";
        fout.close();
    }

    string convert_to_dimacs(string file_path, int num_var, int num_clauses, vector<vector<int>*>* clauses, set<set<int>*>* new_results_to_avoid){
        cout << "================[DIMACS FILE CREATION]====================" << endl;
        string output_name = std::move(file_path);
        string in_name;
        while (output_name[output_name.size() - 1] != '.') {
            output_name = output_name.substr(0, output_name.size() - 1);
        }
        output_name = output_name.substr(0, output_name.size() - 1);
        unsigned long lower = 0;
        for (unsigned long i = output_name.size() - 1; i > 0; i--) {
            if (output_name[i] == '/') {
                lower = i;
                break;
            }
        }
        in_name = output_name.substr(lower + 1, output_name.size());

        output_name = output_name + ".dimacs";
        //====================== END OF FILE CREATION =====================

        ofstream fout(output_name);
        string temp;
        for(auto clause: *clauses){
            for(auto lit: *clause){
                temp.append(to_string(lit)+" ");
            }
            temp.append("0\n");
        }
        //add the new clauses found in the previous iterations
        if(new_results_to_avoid != nullptr) {
            for (auto clause: *new_results_to_avoid) {
                for (auto lit: *clause) {
                    temp.append("-" + to_string(lit) + " ");
                }
                temp.append("0\n");
            }
        }
        fout << "p cnf ";
        if(new_results_to_avoid != nullptr)
            fout << num_var << " " << num_clauses+new_results_to_avoid->size() << endl;
        else
            fout << num_var << " " << num_clauses << endl;
        fout << temp;
        fout.close();
        return output_name;
    }

    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses, set<set<int>*>* new_results_to_avoid){
        if(print_step_by_step_debug || decomposition_debug)
            cout << "================[DIMACS FILE CREATION]====================" << endl;
        string output_name = std::move(file_path);
        string in_name;
        while (output_name[output_name.size() - 1] != '.') {
            output_name = output_name.substr(0, output_name.size() - 1);
        }
        output_name = output_name.substr(0, output_name.size() - 1);
        unsigned long lower = 0;
        for (unsigned long i = output_name.size() - 1; i > 0; i--) {
            if (output_name[i] == '/') {
                lower = i;
                break;
            }
        }
        in_name = output_name.substr(lower + 1, output_name.size());

        output_name = output_name + ".dimacs";
        //====================== END OF FILE CREATION =====================

        ofstream fout(output_name);
        string temp;
        //temp.append("p cnf");
        for(const auto& clause: clauses){
            for(auto lit: clause){
                //fout << lit << " ";
                temp.append(to_string(lit)+" ");
            }
            //fout << "0" << endl;
            temp.append("0\n");
        }
        //add the new clauses found in the previous iterations
        if(new_results_to_avoid != nullptr) {
            for (auto clause: *new_results_to_avoid) {
                for (auto lit: *clause) {
                    //fout << "-" <<lit << " ";
                    temp.append("-" + to_string(lit) + " ");
                }
                //fout << "0" << endl;
                temp.append("0\n");
            }
        }
        fout << "p cnf ";
        if(new_results_to_avoid != nullptr)
            fout << num_var << " " << num_clauses+new_results_to_avoid->size() << endl;
        else
            fout << num_var << " " << num_clauses << endl;
        fout << temp;
        fout.close();
        //f.close();
        return output_name;
    }


    void print_pn_dot_file(map<int, set<Region *> *> *pre_regions,
                           map<int, set<Region *> *> *post_regions,
                           map<int, int> *aliases, string file_name) {
        auto initial_reg = initial_regions(pre_regions);
        string output_name = std::move(file_name);
        string in_dot_name;
        string output;
        // creazione della mappa tra il puntatore alla regione ed un intero univoco
        // corrispondente
        map<Region *, int> *regions_mapping;
        /*cout << "preregions prima del print" << endl;
        print(*pre_regions);*/
        auto regions_set = copy_map_to_set(pre_regions);
        /*cout << "regions set " << endl;
        println(*regions_set);*/
        auto not_initial_regions =
                region_pointer_difference(regions_set, initial_reg);
        regions_mapping = get_regions_map(pre_regions);


        while (output_name[output_name.size() - 1] != '.') {
            output_name = output_name.substr(0, output_name.size() - 1);
        }
        output_name = output_name.substr(0, output_name.size() - 1);
        int lower = 0;
        for (int i = static_cast<int>(output_name.size() - 1); i > 0; i--) {
            if (output_name[i] == '/') {
                lower = i;
                break;
            }
        }
        in_dot_name = output_name.substr(lower + 1, output_name.size());
        // cout << "out name: " << in_dot_name << endl;

        output_name += "_PN.dot";
        //cout << "file output PN: " << output_name << endl;

        ofstream fout(output_name);
        fout << "digraph ";
        fout << in_dot_name + "_PN";
        fout << "{\n";
        // regioni iniziali
        //cout << "scrittura regioni iniziali" << endl;
        fout << "subgraph initial_place {\n"
                "\tnode [shape=doublecircle,fixedsize=true, fixedsize = 2, color = "
                "black, fillcolor = gray, style = filled];\n";
        for (auto reg : *initial_reg) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
        }

        fout << "}\n";
        // regioni non iniziali
        fout << "subgraph place {     \n"
                "\tnode [shape=circle,fixedsize=true, fixedsize = 2];\n";
        for (auto reg : *not_initial_regions) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
        }
        fout << "}\n";
        // transazioni (eventi)
        fout << "subgraph transitions {\n"
                "\tnode [shape=rect,height=0.2,width=2, forcelabels = false];\n";
        auto alias_counter = new map<int, int>();
        for (auto al:*aliases) {
            (*alias_counter)[al.second] = 0;
        }
        for (auto record : *aliases) {
            //fout << "\t" << record.first << ";\n";
            int label;
            label = record.second;
            (*alias_counter)[label]++;
            if(g_input){
                fout << "\t" << record.first << " [label = \""
                     << (*aliases_map_number_name)[label];
            }
            else{
                fout << "\t" << record.first << " [label = \""
                     << label;
            }
            //cout<<"debug alias counter di "<< record.second << (*alias_counter)[record.second]<<endl;
            for (int i = 0; i < (*alias_counter)[record.second]; ++i) {
                fout << "'";
            }
            fout << "\"];\n";

        }
        delete alias_counter;
        //transazioni (eventi) iniziali
        for (auto record : *pre_regions) {
            if (record.first < num_events) {
                if(g_input){
                    fout << "\t" << record.first << " [label = \""
                         << (*aliases_map_number_name)[record.first];
                    fout << "\"];\n";
                }
                else{
                    fout << "\t" << record.first << ";\n";
                }

            }
        }
        fout << "}\n";

        //archi tra tansazioni e posti (tra eventi e regioni)
        //regione -> evento
        for (auto record : *pre_regions) {
            for (auto reg : *record.second) {
                if (record.first < num_events) {
                    //if (regions_mapping->find(reg) != regions_mapping->end()) {
                    fout << "\tr" << regions_mapping->at(reg) << " -> "
                             << record.first << ";\n";

                    //} else {
                    //cout << "regions_mapping non contiene ";
                    // println(*reg);
                    //}
                } else {
                    //int label=aliases->at(record.first);
                    if (regions_mapping->find(reg) != regions_mapping->end()) {
                        fout << "\tr" << regions_mapping->at(reg) << " -> "
                             << record.first << ";\n";
                    } else {
                        //cout << "regions_mapping non contiene ";
                        //println(*reg);
                    }
                }
            }
        }
        //evento -> regione
        for (auto record : *post_regions) {
            for (auto reg : *record.second) {
                if (regions_mapping->find(reg) != regions_mapping->end()) {
                    fout << "\t" << record.first << " -> "
                             << "r" << regions_mapping->at(reg) << ";\n";
                } else {
                    // entra qui 2 volte
                    // cout << "regions_mapping non contiene ";
                    // println(*reg);
                }
            }
        }
        fout << "}";
        fout.close();
        delete regions_set;
        delete not_initial_regions;
        delete initial_reg;
        delete regions_mapping;
    }

    void print_sm_dot_file(map<int, Region  *> *pre_regions,
                           map<int, Region *> *post_regions,
                           map<int, int> *aliases, string file_name) {
        auto initial_reg = initial_regions(pre_regions);
        string output_name = std::move(file_name);
        string in_dot_name;
        string output;
        // creazione della mappa tra il puntatore alla regione ed un intero univoco
        // corrispondente
        map<Region *, int> *regions_mapping;
        /*cout << "preregions prima del print" << endl;
        print(*pre_regions);*/
        auto regions_set = copy_map_to_set(pre_regions);
        /*cout << "regions set " << endl;
        println(*regions_set);*/
        auto not_initial_regions =
                region_pointer_difference(regions_set, initial_reg);
        regions_mapping = get_regions_map(pre_regions);


        while (output_name[output_name.size() - 1] != '.') {
            output_name = output_name.substr(0, output_name.size() - 1);
        }
        output_name = output_name.substr(0, output_name.size() - 1);
        int lower = 0;
        for (int i = static_cast<int>(output_name.size() - 1); i > 0; i--) {
            if (output_name[i] == '/') {
                lower = i;
                break;
            }
        }
        in_dot_name = output_name.substr(lower + 1, output_name.size());
        // cout << "out name: " << in_dot_name << endl;

        output_name += "_PN.dot";
        //cout << "file output PN: " << output_name << endl;

        ofstream fout(output_name);
        fout << "digraph ";
        fout << in_dot_name + "_PN";
        fout << "{\n";
        // regioni iniziali
        //cout << "scrittura regioni iniziali" << endl;
        fout << "subgraph initial_place {\n"
                "\tnode [shape=doublecircle,fixedsize=true, fixedsize = 2, color = "
                "black, fillcolor = gray, style = filled];\n";
        for (auto reg : *initial_reg) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
        }

        fout << "}\n";
        // regioni non iniziali
        fout << "subgraph place {     \n"
                "\tnode [shape=circle,fixedsize=true, fixedsize = 2];\n";
        for (auto reg : *not_initial_regions) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
        }
        fout << "}\n";
        // transazioni (eventi)
        fout << "subgraph transitions {\n"
                "\tnode [shape=rect,height=0.2,width=2, forcelabels = false];\n";
        auto alias_counter = new map<int, int>();
        for (auto al:*aliases) {
            (*alias_counter)[al.second] = 0;
        }
        for (auto record : *aliases) {
            //fout << "\t" << record.first << ";\n";
            //control if this label is used in this SM
            bool used = false;
            if (pre_regions->find(record.first) != pre_regions->end()) {
                used = true;
            } else if (post_regions->find(record.first) != post_regions->end()) {
                used = true;
            }
            if(used){
                int label;
                label = record.second;
                (*alias_counter)[label]++;
                if (g_input) {
                    fout << "\t" << record.first << " [label = \""
                         << (*aliases_map_number_name)[label];
                } else {
                    fout << "\t" << record.first << " [label = \""
                         << label;
                }
                //cout<<"debug alias counter di "<< record.second << (*alias_counter)[record.second]<<endl;
                for (int i = 0; i < (*alias_counter)[record.second]; ++i) {
                    fout << "'";
                }
                fout << "\"];\n";
            }
        }
        delete alias_counter;
        //transazioni (eventi) iniziali
        for (auto record : *pre_regions) {
            if (record.first < num_events) {
                if(g_input){
                    fout << "\t" << record.first << " [label = \""
                         << (*aliases_map_number_name)[record.first];
                    fout << "\"];\n";
                }
                else{
                    fout << "\t" << record.first << ";\n";
                }

            }
        }
        fout << "}\n";

        //archi tra tansazioni e posti (tra eventi e regioni)
        //regione -> evento
        for (auto record : *pre_regions) {
            auto reg = record.second;
            if (record.first < num_events) {
                //if (regions_mapping->find(reg) != regions_mapping->end()) {
                fout << "\tr" << regions_mapping->at(reg) << " -> "
                     << record.first << ";\n";

                //} else {
                //cout << "regions_mapping non contiene ";
                // println(*reg);
                //}
            } else {
                //int label=aliases->at(record.first);
                if (regions_mapping->find(reg) != regions_mapping->end()) {
                    fout << "\tr" << regions_mapping->at(reg) << " -> "
                         << record.first << ";\n";
                } else {
                    //cout << "regions_mapping non contiene ";
                    //println(*reg);
                }
            }
        }
        //evento -> regione
        for (auto record : *post_regions) {
            auto reg = record.second;
            if (regions_mapping->find(reg) != regions_mapping->end()) {
                fout << "\t" << record.first << " -> "
                     << "r" << regions_mapping->at(reg) << ";\n";
            } else {
                // entra qui 2 volte
                // cout << "regions_mapping non contiene ";
                // println(*reg);
            }
        }
        fout << "}";
        fout.close();
        delete regions_set;
        delete not_initial_regions;
        delete initial_reg;
        delete regions_mapping;
    }

    map<Region *, int> *get_regions_map(map<int, set<Region *> *> *net) {
        auto regions_map = new map<Region *, int>();
        int counter = 0;
        for (auto record : *net) {
            for (auto reg : *record.second) {
                if (regions_map->find(reg) == regions_map->end()) {
                    (*regions_map)[reg] = int(counter);
                    counter++;
                }
            }
        }
        // x debug
        /*cout << "reg mapping:" << endl;
        for (auto record : *regions_map) {
            cout << record.second << ": ";
            println(*record.first);
        }*/
        return regions_map;
    }

    map<Region *, int> *get_regions_map(map<int, Region *> *net) {
        auto regions_map = new map<Region *, int>();
        int counter = 0;
        for (auto record : *net) {
            auto reg = record.second;
            if (regions_map->find(reg) == regions_map->end()) {
                (*regions_map)[reg] = int(counter);
                counter++;
            }
        }
        // x debug
        /*cout << "reg mapping:" << endl;
        for (auto record : *regions_map) {
            cout << record.second << ": ";
            println(*record.first);
        }*/
        return regions_map;
    }

    set<Region *> *initial_regions(map<int, set<Region *> *> *reg_map) {
        auto init_reg = new set<Region *>();
        for (auto rec : *reg_map) {
            for (auto reg : *rec.second) {
                if (is_initial_region(reg)) {
                    init_reg->insert(reg);
                }
            }
        }
        return init_reg;
    }

    set<Region *> *initial_regions(map<int, Region *> *reg_map) {
        auto init_reg = new set<Region *>();
        for (auto rec : *reg_map) {
            auto reg = rec.second;
            if (is_initial_region(reg)) {
                init_reg->insert(reg);
            }
        }
        return init_reg;
    }

    set<Region *> *region_pointer_difference(set<Region *> *first,
                                             set<Region *> *second) {
        /*cout << "primo insieme: " << endl;
        for(auto reg: *first){
                println(*reg);
        }
            cout << "secondo insieme: " << endl;
            for(auto reg: *second){
                    println(*reg);
            }*/
        auto difference = new set<Region *>();
        for (auto reg : *first) {
            if (second->find(reg) == second->end()) {
                difference->insert(reg);
            }
        }
        /*cout << "risultato: " << endl;
        for(auto reg: *difference){
                println(*reg);
        }*/
        return difference;
    }

    char translate_label(int label) {
        char base = 'a';
        base += label;
        return base;
    }

    Region *get_ptr_into(set<Region *> *set, Region *region) {

        std::set<Region *>::iterator it;
        for (it = set->begin(); it != set->end(); ++it) {
            auto elem = *it;
            // for(auto elem:*set){
            if (are_equal(elem, region)) {
                return *it;
            }
        }

        return nullptr;
    }

    bool contains_state(Region *reg, int state) {

        for (auto el : *reg) {
            if (el == state)
                return true;
        }

        return false;
    }

    void println(set<Region *> &regions) {
        for (auto reg : regions) {
            println(*reg);
            //cout << "reg. in.: " << reg << endl;
        }
    }

    void print(map<int, set<Region *> *> &net) {
        for (auto rec : net) {
            cout << "event: " << rec.first << endl;
            if (rec.second == nullptr)
                cout << "NULL" << endl;
            println(*rec.second);
        }
    }

    void print_SM(set<Region *>* SM){
        println(*SM);
        cout << endl;
    }

    set<Region *> *region_pointer_union(set<Region *> *first,
                                        set<Region *> *second) {
        /*cout << "primo insieme: " << endl;
        println(*first);

        cout << "secondo insieme: " << endl;
        println(*second);*/

        auto un = new set<Region *>(*first);
        for (auto reg : *second) {
            un->insert(reg);
        }
        /*cout << "risultato: " << endl;
        println(*un);*/

        return un;
    }

    void restore_default_labels(map<int, set<Region *> *> *net,
                                map<int, int> &aliases) {
        /*cout << "mappa alias: " << endl;
        for(auto rec: aliases){
            cout << rec.first << " : " << rec.second << endl;
        }
        cout << "vecchia mappa:" << endl;
        print(*net);*/
        int counter = 0;
        for (auto rec : *net) {
            // l'etichetta rec.first è stata splittata
            if (aliases.find(rec.first) != aliases.end()) {
                net->at(rec.first) =
                        region_pointer_union(rec.second, net->at(aliases.at(rec.first)));
            }
            counter++;
            if (counter == num_events)
                break;
        }
        for (auto rec : aliases) {
            net->erase(rec.second);
        }
        /*cout << "nuova mappa:" << endl;
        print(*net);*/
    }

    //for each region is created a clause with only thi region identifier
    //set on true value and also
    /*vec<int>* region_to_clause(Region* region) {
        auto clause = new vec<int>;
        region_mapping(region);
        max_alias_decomp++;
        int reg = max_alias_decomp -1;
        clause->push(  reg );
        return clause;
    }*/

    void region_mapping(Region* region){
        (*aliases_region_pointer)[max_alias_decomp] = region;
        (*aliases_region_pointer_inverted)[region] = max_alias_decomp;
        max_alias_decomp++;
    }

    set<vector<int>*>* overlapping_regions_clause(set<Region *> *overlapping_regions){
        vector<Region *>* v = new vector<Region *>(overlapping_regions->begin(), overlapping_regions->end());
        auto clauses = new set<vector<int>*>();
        int reg_alias;
        //create a clause for each couple of regions of the overlapping set
        for(unsigned int i=0; i < overlapping_regions->size();i++){
            for(unsigned int k = i+1; k < overlapping_regions->size();k++){
                if(overlaps_cache->find(make_pair((*v)[i], (*v)[k])) == overlaps_cache->end()){
                    if(overlaps_cache->find(make_pair((*v)[k], (*v)[i])) == overlaps_cache->end()){
                        auto clause = new vector<int>();
                        reg_alias = (*aliases_region_pointer_inverted)[(*v)[i]];
                        clause->push_back(  -reg_alias );
                        reg_alias = (*aliases_region_pointer_inverted)[(*v)[k]];
                        clause->push_back(  -reg_alias );
                        clauses->insert(clause);
                        (*overlaps_cache)[make_pair((*v)[i], (*v)[k])] = true;
                    }
                    /*else{
                        cout << "CACHE USED !!!!" << endl;
                    }*/
                }
                /*else{
                    cout << "CACHE USED !!!!" << endl;
                }*/
            }
        }
        delete v;
        return clauses;
    }

    vector<int>* covering_state_clause(set<Region *> *overlapping_regions){
        auto clause = new vector<int>();
        int reg_alias;
        for(auto reg: *overlapping_regions){
            reg_alias = (*aliases_region_pointer_inverted)[reg];
            clause->push_back(reg_alias);
        }
        return clause;
    }

     vector<vector<int>*>* add_regions_clauses_to_solver(map<int, set<Region *> *> *regions){
        auto clauses = new vector<vector<int>*>();
        auto regions_set = copy_map_to_set(regions);
        //mapping of region aliases
        for(auto region: *regions_set){
            region_mapping(region);
        }

        auto map_of_overlapped_regions = new map<int, set<Region*>*>();
        for(auto region: *regions_set){
            //adding of the region to the aliases map
            for(auto state: *region){
                if(map_of_overlapped_regions->find(state) == map_of_overlapped_regions->end()){
                    (*map_of_overlapped_regions)[state] = new set<Region*>();
                }
                (*map_of_overlapped_regions)[state]->insert(region);
            }
        }
        //creation of set of clauses for each set of overlapping regions on a state
        for (auto const& record : *map_of_overlapped_regions)
        {
            //clauses->push_back(covering_state_clause(record.second));
            auto overlapping_regions_clauses = overlapping_regions_clause(record.second);
            for(auto clause: * overlapping_regions_clauses){
                clauses->push_back(clause);
            }
            num_clauses+=overlapping_regions_clauses->size(); //+1 with covering state_clause
            //solver.addClause(overlapping_regions_clause(record.second));
            /*for(auto rec: *overlapping_regions_clauses){
                delete rec;
            }*/
            delete overlapping_regions_clauses;
        }
        delete regions_set;
        for(auto rec: *map_of_overlapped_regions){
            delete rec.second;
        }
        delete map_of_overlapped_regions;
        return clauses;
    }

    map<int, set<Region *> *>* merge_2_maps(map<int, set<Region *> *> *first, map<int, set<Region *> *> *second) {
        //  cout << "MERGIN ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

        map<int, set<Region *> *> *total_pre_regions_map = nullptr;
        total_pre_regions_map = new map<int, set<Region *> *>();

        /*cout << "first" << endl;
        for (auto el : *first) {
            cout << "ev " << el.first << endl;
            for (auto reg : *el.second)
                println(*reg);
        }*/

        //second può essere nullptr
        /*cout << "  SECOND" << endl;
        for (auto el:*second) {
            cout << "ev " << el.first << endl;
            for (auto reg:*el.second)
                println(*reg);
        }*/

        if (second != nullptr) {
            for (int event = 0; event < num_events_after_splitting; event++) {
                // cout << "evento:" << event << endl;

                // trovo entrambi gli eventi
                if (first->find(event) != first->end() &&
                    second->find(event) != second->end()) {

                    auto merged_vector = new vector<Region *>(first->at(event)->size() +
                                                              second->at(event)->size());

                    set_union(first->at(event)->begin(), first->at(event)->end(),
                              second->at(event)->begin(), second->at(event)->end(),
                              merged_vector->begin());

                    /*cout << "merged vector: ";
                    for (auto el : *merged_vector)
                      println(*el);*/

                    (*total_pre_regions_map)[event] =
                            new set<Region *>(merged_vector->begin(), merged_vector->end());
                    delete merged_vector;
                    /*cout<<"entrambe"<<endl;
                    for(auto el: *total_pre_regions_map->at(event))
                        println(*el);*/
                }
                    // l'evento è solo in first(essential)
                else if (first->find(event) != first->end()) {
                    auto merged_vector = new vector<Region *>(first->at(event)->size());

                    (*total_pre_regions_map)[event] = new set<Region *>(
                            first->at(event)->begin(), first->at(event)->end());
                    delete merged_vector;
                    /*cout<<"first"<<endl;
                    for(auto el: *total_pre_regions_map->at(event))
                            println(*el);*/
                }
                    // l'evento è solo in second(irredundant)
                else if (second->find(event) != second->end()) {
                    auto merged_vector = new vector<Region *>(second->at(event)->size());

                    (*total_pre_regions_map)[event] = new set<Region *>(
                            second->at(event)->begin(), second->at(event)->end());
                    delete merged_vector;
                    /* cout<<"secodn"<<endl;
                     for(auto el: *total_pre_regions_map->at(event))
                         println(*el);*/
                }
            }
        } else {
            for (auto record : *first) {
                auto event = record.first;
                (*total_pre_regions_map)[event] =
                        new set<Region *>(first->at(event)->begin(), first->at(event)->end());
            }
        }

        /*cout << "debug merging 2 mappe" << endl;
        for (auto el : *total_pre_regions_map) {
            cout << "ev: " << el.first << endl;
            for (auto r : *el.second) {
                print(*r);
                cout << " ind.: " << r << endl;
            }
        }*/
        return total_pre_regions_map;
    }

    map<int, set<Region *> *>* merge_2_maps(map<int, set<Region *> *> *first, map<int, Region *> *second) {
        //  cout << "MERGIN ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

        map<int, set<Region *> *> *total_pre_regions_map = nullptr;
        total_pre_regions_map = new map<int, set<Region *> *>();

        if (second != nullptr) {
            for (int event = 0; event < num_events_after_splitting; event++) {
                // cout << "evento:" << event << endl;

                // trovo entrambi gli eventi
                if (first->find(event) != first->end() &&
                    second->find(event) != second->end()) {

                    auto merged_vector = new vector<Region *>(first->at(event)->size() + 1);

                    /*set_union(first->at(event)->begin(), first->at(event)->end(),
                              second->at(event)->begin(), second->at(event)->end(),
                              merged_vector->begin());*/
                    for(auto elem: *first->at(event)){
                        merged_vector->push_back(elem);
                    }
                    merged_vector->push_back(second->at(event));

                    /*cout << "merged vector: ";
                    for (auto el : *merged_vector)
                      println(*el);*/

                    (*total_pre_regions_map)[event] =
                            new set<Region *>(merged_vector->begin(), merged_vector->end());
                    delete merged_vector;
                    /*cout<<"entrambe"<<endl;
                    for(auto el: *total_pre_regions_map->at(event))
                        println(*el);*/
                }
                    // l'evento è solo in first(essential)
                else if (first->find(event) != first->end()) {
                    auto merged_vector = new vector<Region *>(first->at(event)->size());

                    (*total_pre_regions_map)[event] = new set<Region *>(
                            first->at(event)->begin(), first->at(event)->end());
                    delete merged_vector;
                    /*cout<<"first"<<endl;
                    for(auto el: *total_pre_regions_map->at(event))
                            println(*el);*/
                }
                    // l'evento è solo in second(irredundant)
                else if (second->find(event) != second->end()) {
                    auto merged_vector = new vector<Region *>(1);

                    (*total_pre_regions_map)[event] = new set<Region *>();
                    (*total_pre_regions_map)[event]->insert(second->at(event));
                    delete merged_vector;
                    /* cout<<"secodn"<<endl;
                     for(auto el: *total_pre_regions_map->at(event))
                         println(*el);*/
                }
            }
        } else {
            for (auto record : *first) {
                auto event = record.first;
                (*total_pre_regions_map)[event] =
                        new set<Region *>(first->at(event)->begin(), first->at(event)->end());
            }
        }

        return total_pre_regions_map;
    }

    map<int, set<Region *> *>* merge_2_maps(map<int, Region *> *first, map<int, Region *> *second) {
        //  cout << "MERGIN ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

        map<int, set<Region *> *> *total_pre_regions_map = nullptr;
        total_pre_regions_map = new map<int, set<Region *> *>();

        if (second != nullptr) {
            for (int event = 0; event < num_events_after_splitting; event++) {
                // cout << "evento:" << event << endl;

                // trovo entrambi gli eventi
                if (first->find(event) != first->end() &&
                    second->find(event) != second->end()) {

                    auto merged_vector = new vector<Region *>(2);

                    merged_vector->push_back(first->at(event));
                    merged_vector->push_back(second->at(event));

                    /*cout << "merged vector: ";
                    for (auto el : *merged_vector)
                      println(*el);*/

                    (*total_pre_regions_map)[event] =
                            new set<Region *>(merged_vector->begin(), merged_vector->end());
                    delete merged_vector;
                    /*cout<<"entrambe"<<endl;
                    for(auto el: *total_pre_regions_map->at(event))
                        println(*el);*/
                }
                    // l'evento è solo in first(essential)
                else if (first->find(event) != first->end()) {
                    auto merged_vector = new vector<Region *>(1);
                    (*total_pre_regions_map)[event] = new set<Region *>();
                    (*total_pre_regions_map)[event]->insert(first->at(event));
                    delete merged_vector;
                    /*cout<<"first"<<endl;
                    for(auto el: *total_pre_regions_map->at(event))
                            println(*el);*/
                }
                    // l'evento è solo in second(irredundant)
                else if (second->find(event) != second->end()) {
                    auto merged_vector = new vector<Region *>(1);
                    (*total_pre_regions_map)[event] = new set<Region *>();
                    (*total_pre_regions_map)[event]->insert(second->at(event));
                    delete merged_vector;
                    /* cout<<"secodn"<<endl;
                     for(auto el: *total_pre_regions_map->at(event))
                         println(*el);*/
                }
            }
        } else {
            for (auto record : *first) {
                auto event = record.first;
                (*total_pre_regions_map)[event] = new set<Region *>();
                (*total_pre_regions_map)[event]->insert(first->at(event));
            }
        }

        return total_pre_regions_map;
    }


    void add_region_to_SM(set<Region*>* SM, Region* region){
        SM->insert(region);
    }

    bool check_sat_formula_from_dimacs(Minisat::Solver& solver, const string& file_path){
        FILE* f;
        f = fopen(file_path.c_str(), "r");
        Minisat::parse_DIMACS(f, solver);
        fclose(f);

        if(decomposition_debug)
            cout << "=============================[SAT-SOLVER RESOLUTION]=====================" << endl;

        if (!solver.simplify()) {
            return false;
        }

        Minisat::vec<Minisat::Lit> dummy;
        Minisat::lbool ret = solver.solveLimited(dummy);
        //True return value
        if (ret == Minisat::lbool((uint8_t)0)) {
            return true;
            //false return value
        } else if (ret == Minisat::lbool((uint8_t)1))
            return false;
        else {
            cerr << "NO RESULT" << endl;
            exit(1);
        }
    }

    bool is_excitation_closed(map<int, set<Region *> *> *pre_regions, map<int, ER> *ER_set ) {

        auto regions_intersection_map = new map<int, set<int> *>() ;

        // per ogni evento
        // se per un evento non vale che l'intersezione è uguale all'er la TS non è
        // exitation-closed
        // bool res=true;

        //for (auto item : *pre_regions) {
        //cout<<"num evens after splitting "<<num_events_after_splitting<<endl;
        for(int event=0;event<num_events_after_splitting;++event){
            //cout << "event: " << item.first;
            //auto event = item.first;
            auto er = ER_set->at((event));
            //cout << "ER at" << event << " : " << endl;
            //println(*er);
            //l'evento non ha preregioni allora non vale la EC
            if(pre_regions->find(event)==pre_regions->end()){
                if(print_step_by_step_debug || decomposition_debug){
                    cout << "event " << event << "not satisfy EC becouse it havent preregions" << endl;
                }
                for(auto rec: *regions_intersection_map){
                    delete rec.second;
                }
                delete regions_intersection_map;
                return false;
            } else {
                auto intersec = regions_intersection(pre_regions->at(event));
                (*regions_intersection_map)[event] = intersec;
                //cout << "Intersec at" << event << " :" << endl;
                //println(*intersec);
                if (!(are_equal(er, intersec))) {
                    // cout << "regione delle'evento:" << event;
                    /*if(print_step_by_step_debug || decomposition_debug){
                        cout << "event " << event << " not satisfy ec becouse the intersection of regions is different from er" << endl;
                        cout << "the intersection is ";
                        if(intersec->empty()){
                            cout << "empty" << endl;
                        }
                        else{
                            println(*intersec);
                        }
                        cout << "the pre-regions of event are: " << endl;
                        for(auto val: *pre_regions->at(event)){
                            println(*val);
                        }
                        cout << "er: ";
                        println(*er);
                    }*/
                    for(auto rec: *regions_intersection_map){
                        delete rec.second;
                    }
                    delete regions_intersection_map;
                    return false;
                }
            }
        }
        for(auto rec: *regions_intersection_map){
            delete rec.second;
        }
        delete regions_intersection_map;
        return true;
    }

    string remove_extension(string path){
        int lower = 0;
        for (int i = static_cast<int>(path.size() - 1); i > 0; i--) {
            if (path[i] == '.') {
                lower = i;
                break;
            }
        }
        string res = path.substr(0, lower);
        return res;
    }

    bool is_initial_region(Region *reg){
        return reg->find(initial_state) != reg->end();
    }

    int getStatesSum(set<SM*>* SMs){
        int sum = 0;
        for(auto SM: *SMs){
            sum += getNumStates(SM);
        }
        return sum;

    }

    int getNumStates(SM* sm){
        return sm->size();
    }
}
