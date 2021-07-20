/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "../include/Utilities.h"

#include <utility>

bool print_step_by_step;
bool print_step_by_step_debug;
bool decomposition;
bool decomposition_debug;
bool decomposition_output;
bool decomposition_output_sis;
bool python_all;
bool ts_output;
bool ects_output;
bool k_fcpn_decomposition;
bool no_merge;
bool composition;
//bool log_file;
bool info;
bool fcptnet;
bool aut_output;
__attribute__((unused)) bool fcpn_modified;
__attribute__((unused)) bool blind_fcpn;
__attribute__((unused)) bool fcpn_with_levels;
bool pn_synthesis;
bool no_fcpn_min;
map<int, Region*>* aliases_region_pointer;
map<Region*, int>* aliases_region_pointer_inverted;
map<Region*, int>* sm_region_aliases;
int max_alias_decomp;
int num_clauses;
map<pair<Region*, Region*>, bool> *overlaps_cache;
bool benchmark_script;
map<set<Region*>, set<int>*> *intersection_cache;

namespace Utilities {
    __attribute__((unused)) set<Region *> *regions_set_union(set<set<Region*>*> *region_set){
        auto res = new set<Region *>();
        for(auto reg_set: *region_set){
            for(auto reg: *reg_set){
                res->insert(reg);
            }
        }
        return res;
    }

    __attribute__((unused)) set<Region *> *regions_set_union(const set<Region*> *region_set1,const set<Region*> *region_set2){
        auto res = new set<Region *>();
        for(auto reg: *region_set1){
            res->insert(reg);
        }
        for(auto reg: *region_set2){
            res->insert(reg);
        }
        return res;
    }

    __attribute__((unused)) set<Region *> *regions_set_union(const set<Region*>& region_set1,const set<Region*>& region_set2){
        auto res = new set<Region *>();
        for(auto reg: region_set1){
            res->insert(reg);
        }
        for(auto reg: region_set2){
            res->insert(reg);
        }
        return res;
    }

    set<Region *> regions_set_union_stack(const set<Region*>& region_set1,const set<Region*>& region_set2){
        set<Region *> res;
        for(auto reg: region_set1){
            res.insert(reg);
        }
        for(auto reg: region_set2){
            res.insert(reg);
        }
        return res;
    }

    __attribute__((unused)) bool regions_set_intersection_is_empty(const set<Region*> *region_set1,const set<Region*> *region_set2){
        for(auto reg: *region_set1){
            if(region_set2->find(reg) != region_set2->end()){
                return false;
            }
        }
        return true;
    }

    __attribute__((unused)) bool equal_sets(const set<Region*> *region_set1,const set<Region*> *region_set2){
        for(auto reg: *region_set1){
            if(region_set2->find(reg) != region_set2->end()){
                return false;
            }
        }
        for(auto reg: *region_set2){
            if(region_set1->find(reg) != region_set1->end()){
                return false;
            }
        }
        return true;
    }

    Region *regions_union(vector<Region *> *vec) {
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

    Region *regions_union(set<Region *> *vec) {
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
        Region::iterator it;
        it = first->begin();
        int size = static_cast<int>(first->size());
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

    __attribute__((unused)) map<int, set<int> *> *do_regions_intersection(map<int, set<Region *> *> *regions) {

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
        bool state_in_intersection;

        if (*regions->begin() == nullptr)
            return pre_regions_intersection;

        for (auto state : **regions->begin()) {
            state_in_intersection = true;
            //todo: questo è un possibile miglioramento, da testare
            /*_Rb_tree_const_iterator<set<int> *> it;
            for(it=next(regions->begin());it != regions->end();++it){
                auto s = *it;
                if (s->find(state) == s->end()) { // state not found
                    state_in_intersection = false;
                    break;
                }
            }*/
            for (auto s : *regions) {
                if (s->find(state) == s->end()) { // state not found
                    state_in_intersection = false;
                    break;
                }
            }
            if (state_in_intersection) {
                pre_regions_intersection->insert(state);
            }
        }

        return pre_regions_intersection;
    }

    set<int> *regions_intersection(const set<Region *>& regions) {
        //println(*regions);

        auto pre_regions_intersection = new set<int>();
        bool state_in_intersection;

        if (*regions.begin() == nullptr)
            return pre_regions_intersection;

        for (auto state : **regions.begin()) {
            state_in_intersection = true;
            //todo: questo è un possibile miglioramento, da testare
            /*_Rb_tree_const_iterator<set<int> *> it;
            for(it=next(regions->begin());it != regions->end();++it){
                auto s = *it;
                if (s->find(state) == s->end()) { // state not found
                    state_in_intersection = false;
                    break;
                }
            }*/
            for (auto s : regions) {
                if (s->find(state) == s->end()) { // state not found
                    state_in_intersection = false;
                    break;
                }
            }
            if (state_in_intersection) {
                pre_regions_intersection->insert(state);
            }
        }

        return pre_regions_intersection;
    }

    set<int> *regions_intersection(Region *first, Region *second) {
        auto intersection = new set<int>();
        for (auto state : *first) {
            if (second->find(state) != second->end()) { // trovo lo stato (appartiene a entrambe)
                intersection->insert(state);
            }
        }
        return intersection;
    }

    bool at_least_one_state_from_first_in_second(Region *first, Region *second){
        for (auto state : *first) {
            if (second->find(state) !=
                second->end()) { // trovo lo stato (appartiene a entrambe)
                return true;
            }
        }
        return false;
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

    bool empty_region_set_intersection(set<Region *> *first, set<Region *> *second){
        for(auto r1: *first){
            for(auto r2: *second){
                auto intersection = regions_intersection(r1, r2);
                if(!intersection->empty()) {
                    delete intersection;
                    return false;
                }
                delete intersection;
            }
        }
        return true;
    }

    bool have_common_regions(set<Region *> *first, set<Region *> *second){
        for(auto r1: *first){
            for(auto r2: *second){
                if(r1 == r2) {
                    return true;
                }
            }
        }
        return false;
    }

    vector<Region> *copy_map_to_vector(map<int, vector<Region> *> *map) {
        auto input = new set<Region>();
        for (auto record : *map) {
            for (const auto &region : *record.second) {
                input->insert(region);
            }
        }
        auto vec = new vector<Region>(input->size());
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
                else{
                    delete r;
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

    __attribute__((unused)) bool is_bigger_than(Region *region, set<int> *region2) {
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

    bool are_equal(Region *region1, Region region2) {
        return are_equal(*region1, std::move(region2));
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

    template<typename T>
    bool contains(T container, Region *region) {
        if (!is_same<T, set<Region *>*>::value && !is_same<T, vector<Region *>*>::value){
            cerr << "wrong parameter type" << endl;
            exit(1);
        }
        for (auto elem : *container) {
            if (are_equal(elem, region)) {
                return true;
            }
        }
        return false;
    }


    template<typename T, typename T2>
    bool contains(T bigger_set, T2 smaller_set) {
        if (!is_same<T, set<Region *>*>::value && !is_same<T, vector<Region *>*>::value){
            cerr << "wrong parameter type" << endl;
            exit(1);
        }
        if (!is_same<T2, set<Region *>*>::value && !is_same<T2, vector<Region *>*>::value){
            cerr << "wrong parameter type" << endl;
            exit(1);
        }

        for (auto elem : *smaller_set) {
            if(bigger_set->find(elem) == bigger_set->end())
                return false;
        }
        return true;
    }

    __attribute__((unused)) bool contains(const set<Region *>& set, const Region& region) {
        for (auto elem : set) {
            if (are_equal(elem, region)) {
                return true;
            }
        }
        return false;
    }

    __attribute__((unused)) bool contains_region(const set<Region *>& set, Region *region) {
        for (auto elem : set) {
            if (are_equal(elem, region)) {
                return true;
            }
        }
        return false;
    }

    bool contains(set<Region *> bigger_set, const set<Region *>& smaller_set) {
        for (auto elem : smaller_set) {
            if(bigger_set.find(elem) == bigger_set.end())
                return false;
        }
        return true;
    }

    bool contains(set<Region *> *bigger_set, set<Region *> *smaller_set) {
        for (auto elem : *smaller_set) {
            if(bigger_set->find(elem) == bigger_set->end())
                return false;
        }
        return true;
    }

    __attribute__((unused)) bool contains(set<int> bigger_set, const set<int>& smaller_set){
        for (auto elem : smaller_set) {
            if(bigger_set.find(elem) == bigger_set.end())
                return false;
        }
        return true;
    }

    __attribute__((unused)) bool contains(set<int> *bigger_set, set<int> *smaller_set){
        for (auto elem : *smaller_set) {
            if(bigger_set->find(elem) == bigger_set->end())
                return false;
        }
        return true;
    }

    string get_file_name(string path){
        string output_name = std::move(path);
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
        return output_name.substr(lower + 1, output_name.size());
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
        std::replace( in_name.begin(), in_name.end(), '-', '_');

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

    void print_ts_aut_file(string file_path, map<int, int> *aliases) {
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
        std::replace( in_name.begin(), in_name.end(), '-', '_');

        if (aliases != nullptr) {
            in_name += "_ECTS";
            output_name = output_name + "_ECTS.aut";
        } else {
            output_name = output_name + ".aut";
        }

        ofstream fout(output_name);
        fout << "des (";
        fout << initial_state;
        fout << ",";
        fout << num_transactions;
        fout <<",";
        fout << num_states;
        fout << ")" << endl;



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
                fout << "(";
                fout << edge->first;
                fout << ",\"";
                if(!g_input){
                    fout << label;
                }
                else{
                    fout << (*aliases_map_number_name)[label];
                }
                fout << to_add;
                fout << "\",";
                fout << edge->second;
                fout << ")" << endl;
            }

        }

        delete alias_counter;
        delete alias_counter_original;
        fout.close();
    }

    void print_ts_aut_file(string file_path, map <map<set<Region *>*, set<Region *>>, int> *state_aliases, vector<edge> *arcs, map<set<Region *>*, set<Region *>> initial_state_TS){
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
        std::replace( in_name.begin(), in_name.end(), '-', '_');


        output_name = output_name + "_composed.aut";


        ofstream fout(output_name);
        fout << "des (";
        fout << state_aliases->at(initial_state_TS);
        fout << ",";
        fout << arcs->size();
        fout <<",";
        fout << state_aliases->size();
        fout << ")" << endl;

        for(const auto& arc: *arcs){
            fout << "(";
            fout << state_aliases->at(arc.start);
            fout << ",\"";
            fout << arc.event;
            fout << "\",";
            fout << state_aliases->at(arc.end);
            fout << ")" << endl;
        }

        fout.close();
    }


    __attribute__((unused)) string convert_to_dimacs(string file_path, int num_var, int num_clauses, vector<vector<int>*>* clauses, set<set<int>*>* new_results_to_avoid){
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
                    if(lit > 0)
                        temp.append("-" + to_string(lit) + " ");
                    else
                        temp.append(to_string(-lit) + " ");
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

    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses){
        return convert_to_dimacs(file_path, num_var, num_clauses, clauses, nullptr);
    }

    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses, vector<set<int>>* new_results_to_avoid){
        if(decomposition_debug)
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
        for(const auto& clause: clauses){
            for(auto lit: clause){
                temp.append(to_string(lit)+" ");
            }
            temp.append("0\n");
        }
        //add the new clauses found in the previous iterations
        if(new_results_to_avoid != nullptr) {
            for (const auto& clause: *new_results_to_avoid) {
                for (auto lit: clause) {
                    if(lit > 0)
                        temp.append("-" + to_string(lit) + " ");
                    else
                        temp.append(to_string(-lit) + " ");
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

    string convert_to_dimacs_simplified(const string& file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses){
        cout << "================[DIMACS FILE CREATION]====================" << endl;
        string output_name = file_path;
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
        for(const auto& clause: clauses){
            for(auto lit: clause){
                temp.append(to_string(lit)+" ");
            }
            temp.append("0\n");
        }
        fout << "p cnf ";
        fout << num_var << " " << num_clauses << endl;
        fout << temp;
        fout.close();
        return output_name;
    }

    void print_fcpn_dot_file(map<int, set<Region *> *> *pre_regions,
                             map<int, set<Region *> *> *post_regions,
                             map<int, int> *aliases, const string& file_name, int FCPN_number){
        print_fcpn_dot_file(nullptr,pre_regions,post_regions,aliases,file_name,FCPN_number);
    }

    void print_fcpn_dot_file(map<Region *, int> *regions_mapping,
                             map<int, set<Region *> *> *pre_regions,
                             map<int, set<Region *> *> *post_regions,
                             map<int, int> *aliases, const string& file_name, int FCPN_number){
        auto initial_reg = initial_regions(pre_regions);
        /*
        if(initial_reg->empty()){
            cerr << "any initial region found" << endl;
            exit(1);
        }*/
        string output_name = file_name;
        string in_dot_name;
        string output;
        bool delete_regions_mapping = false;
        // creazione della mappa tra il puntatore alla regione ed un intero univoco
        // corrispondente
        if(regions_mapping == nullptr){
            regions_mapping = get_regions_map(pre_regions);
            delete_regions_mapping = true;
        }

        /*cout << "preregions prima del print" << endl;
        print(*pre_regions);*/
        auto regions_set = copy_map_to_set(pre_regions);
        /*cout << "regions set " << endl;
        println(*regions_set);*/
        auto not_initial_regions =
                region_pointer_difference(regions_set, initial_reg);

        /*
        cout << "initial regions:" << endl;
        for(auto reg: *initial_reg){
            cout << "r" << regions_mapping->at(reg) << " : ";
            println(*reg);
        }
        cout << "not initial regions:" << endl;
        for(auto reg: *not_initial_regions){
            cout << "r" << regions_mapping->at(reg) << " : ";
            println(*reg);
        }*/

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
        std::replace( in_dot_name.begin(), in_dot_name.end(), '-', '_');
        // cout << "out name: " << in_dot_name << endl;

        if(FCPN_number >= 0){
            output_name += "_FCPN_";
            output_name+=std::to_string(FCPN_number);
            output_name+=".dot";
        }
        else{
            output_name += "_FCPN.dot";
        }
        //cout << "file output PN: " << output_name << endl;

        ofstream fout(output_name);
        fout << "digraph ";
        if(FCPN_number >= 0){
            fout << in_dot_name + "_FCPN_";
            fout << std::to_string(FCPN_number);
        }
        else{
            fout << in_dot_name + "_PN";
        }

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
            if(pre_regions->find(record.first) != pre_regions->end() || post_regions->find(record.first) != post_regions->end()) {
                //fout << "\t" << record.first << ";\n";
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
                fout << "/" << (*alias_counter)[record.second];
                /*for (int i = 0; i < (*alias_counter)[record.second]; ++i) {
                    fout << "'";
                }*/
                fout << "\"];\n";
            }
            else{
                (*alias_counter)[record.second]++;
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
        for (auto record : *post_regions) {
            if (record.first < num_events) {
                if(pre_regions->find(record.first) == pre_regions->end()) {
                    if (g_input) {
                        fout << "\t" << record.first << " [label = \""
                             << (*aliases_map_number_name)[record.first];
                        fout << "\"];\n";
                    } else {
                        fout << "\t" << record.first << ";\n";
                    }
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
                    //} else {
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
        if(delete_regions_mapping)
            delete regions_mapping;
    }

    void print_pn_dot_file(map<int, set<Region *> *> *pre_regions,
                           map<int, set<Region *> *> *post_regions,
                           map<int, int> *aliases, const string& file_name) {
        print_fcpn_dot_file(pre_regions, post_regions, aliases, file_name, -1);
    }

    //TODO
    __attribute__((unused)) void print_pn_g_file(map<int, set<Region *> *> *pre_regions,
                         map<int, set<Region *> *> *post_regions,
                         map<int, int> *aliases, const string& file_name){
        cout << "Code still have to be written" << endl;
        exit(1);
    }

    void print_sm_g_file(map<int, Region  *> *pre_regions,
                           map<int, Region *> *post_regions,
                           map<int, int> *aliases, string file_name) {
        auto initial_reg = initial_regions(pre_regions);
        if(initial_reg->empty()){
            cerr << "any initial region found" << endl;
            exit(1);
        }
        string output_name = std::move(file_name);
        string in_sis_name;
        string output;
        // creazione della mappa tra il puntatore alla regione ed un intero univoco corrispondente
        map<Region *, int> *regions_mapping;
        /*cout << "pre-regions prima del print" << endl;
        print(*pre_regions);*/
        auto regions_set = copy_map_to_set(pre_regions);
        /*cout << "regions set " << endl;
        println(*regions_set);*/
        regions_mapping = get_regions_map(pre_regions);

        // counter for the number of splits for each label
        auto alias_counter = new map<int, int>();
        for (auto al:*aliases) {
            (*alias_counter)[al.second] = 0;
        }
        map<int, int> alias_event_index_map;
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
                alias_event_index_map[record.first]=(*alias_counter)[label];
            }
        }

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
        in_sis_name = output_name.substr(lower + 1, output_name.size());
        std::replace( in_sis_name.begin(), in_sis_name.end(), '-', '_');
        // cout << "out name: " << in_dot_name << endl;

        output_name += ".g";
        //cout << "file output PN: " << output_name << endl;

        ofstream fout(output_name);

        set<string> used_labels;
        for(auto rec: *pre_regions){
            int ev = rec.first;
            if(ev >= num_events){
                ev = (*aliases)[ev];
            }
            string eventString = (*aliases_map_number_name)[ev];
            eventString.erase(std::remove(eventString.begin(), eventString.end(), '-'), eventString.end());
            eventString.erase(std::remove(eventString.begin(), eventString.end(), '+'), eventString.end());
            used_labels.insert(eventString);
        }
        fout << ".model " << in_sis_name;
        //events/transitions
        if(g_input){
            if(!inputs.empty()){
                fout << "\n.inputs ";
                for(const auto& ev: used_labels){
                    if(inputs.find(ev) != inputs.end()){
                        fout << ev << " ";
                    }
                    used_labels.erase(ev);
                }
            }
            if(!outputs.empty()){
                if(!inputs.empty())
                    fout << endl;
                fout << "\n.outputs ";
                for(const auto& ev: used_labels){
                    if(outputs.find(ev) != outputs.end()){
                        fout << ev << " ";
                    }
                    used_labels.erase(ev);
                }
            }
            if(!internals.empty()){
                fout << "\n.internal ";
                for(const auto& ev: used_labels){
                    if(internals.find(ev) != internals.end()){
                        fout << ev << " ";
                    }
                    used_labels.erase(ev);
                }
            }
            if(!dummies.empty()){
                fout << "\n.dummy ";
                for(const auto& ev: used_labels){
                    if(dummies.find(ev) != dummies.end()){
                        fout << ev << " ";
                    }
                    used_labels.erase(ev);
                }
            }
        }
        else{
            fout << ".internal ";
            for (auto record : *pre_regions) {
                if (record.first < num_events) {
                    fout << "e" << record.first << " ";
                }
            }
        }

        fout << "\n.graph\n";

        auto pre_regions_inverted = new map<Region *, set<int> *>();
        for(auto record: *pre_regions){
            if(pre_regions_inverted->find(record.second) == pre_regions_inverted->end()){
                (*pre_regions_inverted)[record.second] = new set<int>();
            }
            (*pre_regions_inverted)[record.second]->insert(record.first);
        }

        //archi tra tansazioni e posti (tra eventi e regioni)
        //regione -> evento
        for(auto record: *pre_regions_inverted){
            auto reg = record.first;
            fout << "r" << regions_mapping->at(reg);
            for(auto ev: *record.second){
                if(g_input){
                    if(ev >= num_events){
                        int original_label = (*aliases)[ev];
                        fout << " " << (*aliases_map_number_name)[original_label];
                        fout << "/" << alias_event_index_map[ev];
                    }
                    else{
                        fout << " " << (*aliases_map_number_name)[ev];
                    }
                }
                else {
                    if (ev < num_events) {
                        fout << " e" << ev;
                    } else {
                        int original_label = (*aliases)[ev];
                        fout << " e" << original_label; //nome dell'etichetta originale
                        fout << "/" << alias_event_index_map[ev];
                    }
                }
            }
            fout << endl;
        }
        //event -> region
        for (auto record : *post_regions) {
            auto reg = record.second;
            if(g_input){
                if(record.first >= num_events){
                    int original_label = (*aliases)[record.first];
                    fout << (*aliases_map_number_name)[original_label];
                    fout << "/" << alias_event_index_map[record.first];
                }
                else{
                    fout << (*aliases_map_number_name)[record.first];
                }
            }
            else {
                if (record.first < num_events) {
                    fout << "e" << record.first;
                } else {
                    int original_label = (*aliases)[record.first];
                    fout << "e" << original_label; //nome dell'etichetta originale
                    fout << "/" << alias_event_index_map[record.first];
                }
            }
            fout << " r" << regions_mapping->at(reg) << "\n";
        }
        //initial marking
        fout << ".marking {";
        int count = 0;
        for (auto reg : *initial_reg) {
            if(count == 0){
                fout << "r" << regions_mapping->at(reg);
            }
            else{
                fout << " r" << regions_mapping->at(reg);
            }
            count++;
        }
        fout << "}\n";
        fout << ".end ";
        fout.close();
        for(auto record: *pre_regions_inverted){
            delete record.second;
        }
        delete alias_counter;
        delete pre_regions_inverted;
        delete regions_set;
        delete initial_reg;
        delete regions_mapping;
    }

    void print_sm_dot_file(map<int, Region  *> *pre_regions,
                           map<int, Region *> *post_regions,
                           map<int, int> *aliases,
                           string file_name) {
        auto initial_reg = initial_regions(pre_regions);
        if(initial_reg->empty()){
            cerr << "any initial region found" << endl;
            exit(1);
        }
        string output_name = std::move(file_name);
        string in_dot_name;
        string output;
        map<Region *, int> *regions_mapping;
        set<Region *> *regions_set = copy_map_to_set(pre_regions);
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
        std::replace( in_dot_name.begin(), in_dot_name.end(), '-', '_');
        // cout << "out name: " << in_dot_name << endl;

        output_name += ".dot";
        //cout << "file output SM: " << output_name << endl;

        ofstream fout(output_name);
        fout << "digraph ";
        fout << in_dot_name;
        fout << "{\n";
        // initial regions
        fout << "subgraph initial_place {\n"
                "\tnode [shape=doublecircle,fixedsize=true, fixedsize = 2, color = "
                "black, fillcolor = gray, style = filled];\n";
        for (auto reg : *initial_reg) {
            fout << "\tr" << regions_mapping->at(reg) << " [label = \"r" << (*sm_region_aliases)[reg] << "\"];\n";
        }

        fout << "}\n";
        // not initial regions
        fout << "subgraph place {     \n"
                "\tnode [shape=circle,fixedsize=true, fixedsize = 2];\n";
        for (auto reg : *not_initial_regions) {
            fout << "\tr" << regions_mapping->at(reg) << " [label = \"r" << (*sm_region_aliases)[reg] << "\"];\n";
        }
        fout << "}\n";
        // transitions (events)
        fout << "subgraph transitions {\n"
                "\tnode [shape=rect,height=0.2,width=2, forcelabels = false];\n";
        auto alias_counter = new map<int, int>();
        for (auto al:*aliases) {
            (*alias_counter)[al.second] = 0;
        }
        for (auto record : *aliases) {
            //cout << "\t" << record.first << ";\n";
            //control if this label is used in this SM
            bool used = false;
            if (pre_regions->find(record.first) != pre_regions->end()) {
                used = true;
                //cout << "used: " << record.first << endl;
            } else if (post_regions->find(record.first) != post_regions->end()) {
                used = true;
                //cout << "used: " << record.first << endl;
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
                fout << "/" << (*alias_counter)[record.second];
                fout << "\"];\n";
            }
        }
        delete alias_counter;
        //initial transitions (events)
        auto initial_pre = new set<int>();
        for (auto record : *pre_regions) {
            if (record.first < num_events) {
                initial_pre->insert(record.first);
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
        for (auto record : *post_regions) {
            if (record.first < num_events) {
                //if the event takes part of initial_pre it has been already written on the file
                if(initial_pre->find(record.first) == initial_pre->end()) {
                    if (g_input) {
                        fout << "\t" << record.first << " [label = \""
                             << (*aliases_map_number_name)[record.first];
                        fout << "\"];\n";
                    } else {
                        fout << "\t" << record.first << ";\n";
                    }
                }
            }
        }
        fout << "}\n";
        delete initial_pre;

        //arcs between transitions and places
        //region -> event
        for (auto record : *pre_regions) {
            auto reg = record.second;
            if (record.first < num_events) {
                fout << "\tr" << regions_mapping->at(reg) << " -> "
                     << record.first << ";\n";
            } else {
                if (regions_mapping->find(reg) != regions_mapping->end()) {
                    fout << "\tr" << regions_mapping->at(reg) << " -> "
                         << record.first << ";\n";
                } else {
                    //cout << "regions_mapping does not contains ";
                    //println(*reg);
                }
            }
        }
        //event -> region
        for (auto record : *post_regions) {
            auto reg = record.second;
            if (regions_mapping->find(reg) != regions_mapping->end()) {
                fout << "\t" << record.first << " -> "
                     << "r" << regions_mapping->at(reg) << ";\n";
            } else {
                 cout << "regions_mapping does not contains ";
                 println(*reg);
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
        auto difference = new set<Region *>();
        for (auto reg : *first) {
            if (second->find(reg) == second->end()) {
                difference->insert(reg);
            }
        }
        return difference;
    }

    __attribute__((unused)) char translate_label(int label) {
        char base = 'a';
        base += label;
        return base;
    }

    __attribute__((unused)) Region *get_ptr_into(set<Region *> *set, Region *region) {
        std::set<Region *>::iterator it;
        for (it = set->begin(); it != set->end(); ++it) {
            auto elem = *it;
            if (are_equal(elem, region)) {
                return *it;
            }
        }
        return nullptr;
    }

    __attribute__((unused)) bool contains_state(Region *reg, int state) {
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

    void println(set<Region *> *regions) {
        for (auto reg : *regions) {
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

    __attribute__((unused)) void print_SM(set<Region *>* SM){
        println(*SM);
        cout << endl;
    }

    set<Region *> *region_pointer_union(set<Region *> *first,
                                        set<Region *> *second) {
        auto un = new set<Region *>(*first);
        for (auto reg : *second) {
            un->insert(reg);
        }
        return un;
    }

    __attribute__((unused)) void restore_default_labels(map<int, set<Region *> *> *net,
                                map<int, int> &aliases) {
        int counter = 0;
        for (auto rec : *net) {
            // thhe label rec.first was splitted
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
    }

    void region_mapping(Region* region){
        (*aliases_region_pointer)[max_alias_decomp] = region;
        (*aliases_region_pointer_inverted)[region] = max_alias_decomp;
        max_alias_decomp++;
    }

    set<vector<int>*>* overlapping_regions_clause(set<Region *> *overlapping_regions){
        auto v = new vector<Region *>(overlapping_regions->begin(), overlapping_regions->end());
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

    __attribute__((unused)) vector<int>* covering_state_clause(set<Region *> *overlapping_regions){
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
            auto overlapping_regions_clauses = overlapping_regions_clause(record.second);
            for(auto clause: * overlapping_regions_clauses){
                clauses->push_back(clause);
            }
            num_clauses+=overlapping_regions_clauses->size();
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
        //  cout << "MERGING ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

        map<int, set<Region *> *> *total_pre_regions_map = nullptr;
        total_pre_regions_map = new map<int, set<Region *> *>();

        if (second != nullptr) {
            for (int event = 0; event < num_events_after_splitting; event++) {
                // cout << "evento:" << event << endl;

                // found both events
                if (first->find(event) != first->end() &&
                    second->find(event) != second->end()) {

                    auto merged_vector = new vector<Region *>(first->at(event)->size() +
                                                              second->at(event)->size());

                    set_union(first->at(event)->begin(), first->at(event)->end(),
                              second->at(event)->begin(), second->at(event)->end(),
                              merged_vector->begin());
                    (*total_pre_regions_map)[event] =
                            new set<Region *>(merged_vector->begin(), merged_vector->end());
                    delete merged_vector;
                }
                    // the event is only in first(essential)
                else if (first->find(event) != first->end()) {
                    auto merged_vector = new vector<Region *>(first->at(event)->size());
                    (*total_pre_regions_map)[event] = new set<Region *>(
                            first->at(event)->begin(), first->at(event)->end());
                    delete merged_vector;
                }
                    // thhe event is only in second(irredundant)
                else if (second->find(event) != second->end()) {
                    auto merged_vector = new vector<Region *>(second->at(event)->size());

                    (*total_pre_regions_map)[event] = new set<Region *>(
                            second->at(event)->begin(), second->at(event)->end());
                    delete merged_vector;
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

    map<int, set<Region *> *>* merge_2_maps(map<int, set<Region *> *> *first, map<int, Region *> *second) {
        //  cout << "MERGING ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

        map<int, set<Region *> *> *total_pre_regions_map = nullptr;
        total_pre_regions_map = new map<int, set<Region *> *>();

        if (second != nullptr) {
            for (int event = 0; event < num_events_after_splitting; event++) {
                // found both events
                if (first->find(event) != first->end() &&
                    second->find(event) != second->end()) {

                    auto merged_vector = new vector<Region *>(first->at(event)->size() + 1);

                    for(auto elem: *first->at(event)){
                        merged_vector->push_back(elem);
                    }
                    merged_vector->push_back(second->at(event));

                    (*total_pre_regions_map)[event] =
                            new set<Region *>(merged_vector->begin(), merged_vector->end());
                    delete merged_vector;
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
                    /* cout<<"second"<<endl;
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
        //  cout << "MERGING ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

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


    __attribute__((unused)) void add_region_to_SM(set<Region*>* SM, Region* region){
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

    __attribute__((unused)) bool check_sat_formula_from_dimacs2(Minisat::Solver& solver, const string& file_path){
        FILE* f;
        f = fopen(file_path.c_str(), "r");
        Minisat::parse_DIMACS(f, solver);
        fclose(f);

        if(decomposition_debug)
            cout << "=============================[SAT-SOLVER RESOLUTION]=====================" << endl;

        if (!solver.simplify()) {
            return false;
        }

        auto ret = solver.solve();

        return ret;
    }

    __attribute__((unused)) bool check_ER_intersection(int event, set<Region*> *pre_regions_set, map<int, ER> *ER_set){
        auto er = ER_set->at(event);
        auto intersection = regions_intersection(pre_regions_set);
        bool res = are_equal(er, intersection);
        delete intersection;
        return res;
    }

    __attribute__((unused)) bool check_ER_intersection_with_mem(int event, set<Region*> *pre_regions_set, map<int, ER> *ER_set){
        if(intersection_cache == nullptr)
            intersection_cache = new map<set<Region *>, set<int>*>();
        auto er = ER_set->at(event);
        set<int> *intersection;
        if(intersection_cache->find(*pre_regions_set) != intersection_cache->end()){
            intersection = intersection_cache->at(*pre_regions_set);
            //cout << "cache used"  << endl;
        }
        else{
            intersection = regions_intersection(pre_regions_set);
            (*intersection_cache)[*pre_regions_set] = intersection;
        }
        bool res = are_equal(er, intersection);
        return res;
    }

    bool check_ER_intersection_with_mem(int event, const set<Region*>& pre_regions_set, map<int, ER> *ER_set){
        if(intersection_cache == nullptr)
            intersection_cache = new map<set<Region *>, set<int>*>();
        auto er = ER_set->at(event);
        set<int> *intersection;
        if(intersection_cache->find(pre_regions_set) != intersection_cache->end()){
            intersection = intersection_cache->at(pre_regions_set);
            //cout << "cache used"  << endl;
        }
        else{
            intersection = regions_intersection(pre_regions_set);
            (*intersection_cache)[pre_regions_set] = intersection;
        }
        bool res = are_equal(er, intersection);
        return res;
    }

    bool check_ER_intersection_cache(set<Region*> *pre_regions_set){
        if(intersection_cache->find(*pre_regions_set) != intersection_cache->end())
            return true;
        return false;
    }

    void clear_ER_intersection_cache(){
        for(const auto& rec: *intersection_cache){
            delete rec.second;
        }
        delete intersection_cache;
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
                    cout << "event " << event << "not satisfy EC because it haven't pre-regions" << endl;
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
                    if(print_step_by_step_debug || decomposition_debug){
                        /*cout << "event " << event << " not satisfy ec because the intersection of regions is different from er" << endl;
                        cout << "the intersection is ";
                        if(intersec->empty()){
                            cout << "empty" << endl;
                        }
                        else{
                            println(*intersec);
                        }*/
                        /*cout << "the pre-regions of event are: " << endl;
                        for(auto val: *pre_regions->at(event)){
                            println(*val);
                        }
                        cout << "er: ";
                        println(*er);*/
                    }
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

    double getStatesAvg(set<SM*>* SMs){
        int sum = 0;
        int cont = 0;
        for(auto SM: *SMs){
            sum += getNumStates(SM);
            cont++;
        }
        return ((double)sum / cont);
    }

    double getStatesVar(set<SM*>* SMs){
        double statesAvg = getStatesAvg(SMs);
        double sum = 0;
        int cont = 0;
        int statesCurrentSM = 0;
        for(auto SM: *SMs){
            statesCurrentSM = getNumStates(SM);
            //cout << "states current sm: " << statesCurrentSM << endl;
            sum += (statesCurrentSM - statesAvg)*(statesCurrentSM - statesAvg);
            cont++;
        }
        //cout << "sum: " << sum << endl;
        //cout << "cont: " << cont << endl;
        return (sum / cont);
    }

    int getNumStates(SM* sm){
        return sm->size();
    }

    void create_dimacs_graph(int num_regions, vector<vector<int32_t> *> *clauses){
        ofstream fout("Graph.dimacs");
        fout << "p " << num_regions << " " << clauses->size() << endl;
        for(auto clause: *clauses){
            fout << "a " << clause->at(0)*(-1) << " " << clause->at(1)*(-1) << endl;
        }
        fout.close();
    }

    void read_SMs(const string& file, set<SM*>* SMs, map<int, Region *> &aliases){
        ifstream fin(file);
        int num_SMs;
        fin >> num_SMs;
        //cout << "num SMs: " << num_SMs << endl;
        string temp;
        stringstream ss;
        int temp_region;
        std::getline(fin, temp);
        for(int i=0; i < num_SMs; i++) {
            SM *newSM = new SM();
            std::getline(fin, temp);
            //cout << "temp: " << temp << endl;
            ss.clear();
            ss << temp;
            while (!ss.eof()) {
                ss >> temp_region;
                newSM->insert(aliases.at(temp_region));
                //cout << "temp region: " << temp_region << endl;
            }
            SMs->insert(newSM);
            //cout << "new SM: " << endl;
            //println(*newSM);
        }
    }

    int getTransitionsSum(map<SM*, map<int, Region *>*> *pre_regions){
        int sum = 0;
        for(auto rec: *pre_regions){
            sum += rec.second->size();
        }
        return sum;
    }

    double getTransitionsAvg(map<SM*, map<int, Region *>*> *pre_regions){
        int sum = 0;
        int cont = 0;
        for(auto rec: *pre_regions){
            sum += rec.second->size();
            cont++;
        }
        return  ((double)sum/cont);
    }

    double getTransitionsVar(map<SM*, map<int, Region *>*> *pre_regions){
        double transitionsAvg = getTransitionsAvg(pre_regions);
        double sum = 0;
        int cont = 0;
        int current_SM_transitions = 0;
        for(auto rec: *pre_regions){
            current_SM_transitions = rec.second->size();
            //cout << "current transitions: " << current_SM_transitions << endl;
            sum += (current_SM_transitions - transitionsAvg)*(current_SM_transitions - transitionsAvg);
            cont++;
        }
        //cout << "sum: " << sum << endl;
        //cout << "cont: " << cont << endl;
        return  sum/cont;
    }

    int getMaxPTSum(map<SM*, map<int, Region *>*> *pre_regions){
        int max = 0;
        for(auto rec: *pre_regions){
            int p = getNumStates(rec.first);
            int t = rec.second->size();
            if(p+t > max)
                max = p+t;
        }
        return max;
    }

    int getMaxTransitionsNumber(map<SM*, map<int, Region *>*> *pre_regions){
        int max = 0;
        for(auto rec: *pre_regions){
            int t = rec.second->size();
            if(t > max)
                max = t;
        }
        return max;
    }

    int getMaxAlphabet(map<SM*, map<int, Region *>*>* pre_regions, map<int, int> *label_aliases){
        int max = 0;
        for(auto rec: *pre_regions){
            auto SM_map = rec.second;
            int counter = 0;
            set<int> used_labels;
            for(auto record: *SM_map){
                auto label = record.first;
                if(label < num_events){
                    used_labels.insert(label);
                    counter++;
                }
                else{
                    auto original_label = (*label_aliases)[label];
                    if(used_labels.find(original_label) == used_labels.end()){
                        used_labels.insert(original_label);
                        counter++;
                    }
                }
            }
            if(counter > max)
                max = counter;
        }
        return max;
    }

    int getMaxAlphabet(map<SM*, map<int, set<Region *>*>*>* pre_regions, map<int, int> *label_aliases){
        int max = 0;
        for(auto rec: *pre_regions){
            auto SM_map = rec.second;
            int counter = 0;
            set<int> used_labels;
            for(auto record: *SM_map){
                auto label = record.first;
                if(label < num_events){
                    used_labels.insert(label);
                    counter++;
                }
                else{
                    auto original_label = (*label_aliases)[label];
                    if(used_labels.find(original_label) == used_labels.end()){
                        used_labels.insert(original_label);
                        counter++;
                    }
                }
            }
            if(counter > max)
                max = counter;
        }
        return max;
    }

    double getAvgAlphabet(map<SM*, map<int, set<Region *>*>*>* pre_regions, map<int, int> *label_aliases){
        int sum = 0;
        for(auto rec: *pre_regions){
            auto SM_map = rec.second;
            int counter = 0;
            set<int> used_labels;
            for(auto record: *SM_map){
                auto label = record.first;
                if(label < num_events){
                    used_labels.insert(label);
                    counter++;
                }
                else{
                    auto original_label = (*label_aliases)[label];
                    if(used_labels.find(original_label) == used_labels.end()){
                        used_labels.insert(original_label);
                        counter++;
                    }
                }
            }
            sum += counter;
        }
        return  (double)sum/(pre_regions->size());
    }

    __attribute__((unused)) bool checkSMUnionForFCPTNet(set<SM*> *sm_set, map<int, set<Region*> *> *post_regions){
        cerr << "function checkSMUnionFCPTNet ith set argument not implemented yet" << endl;
        exit(1);
        SM *target_PN = new SM();
        for(auto sm: *sm_set) {
            for (auto reg: *sm) {
                target_PN->insert(reg);
            }
        }

        /*todo: if I find a couple without common events or places it could be still connected via other SMs
         * therefore I cannot check in this way, I have to find a way in order to understand if the result
         * have disconnected subset (it can be split without any violation)
         * Probably I have to check if there is an SM which haven't any common event/place with all of the other SMs
         * but in this case we could skip the case where a couple of SMs is disconnected from the others
         */
         /*bool common_regions = true;
        bool temp;
        for(auto sm1: *sm_set){
            for(auto sm2: *sm_set){
                if(sm1 != sm2){
                    temp = checkCommonRegionsBetweenSMs(sm1, sm2);
                    if(!temp){
                        common_regions = false;
                        break;
                    }
                }
            }
            if(!common_regions) break;
        }*/

    }

    bool checkCommonRegionsBetweenSMs(SM *sm1, SM *sm2){
        for(auto r1: *sm1){
            for(auto r2: *sm2){
                if(r1 == r2){
                    return true;
                }
            }
        }
        return false;
    }

    __attribute__((unused)) bool checkSMUnionForFCPTNet(SM* sm1, SM* sm2, map<int, set<Region*> *> *post_regions){
        SM *target_PN = new SM();
        for (auto reg: *sm1) {
            target_PN->insert(reg);
        }
        for (auto reg: *sm2) {
            target_PN->insert(reg);
        }

        //check if there are common regions
        bool common_regions = checkCommonRegionsBetweenSMs(sm1, sm2);

        //check common labels
        bool common_events = false;
        for(auto rec: *post_regions){
            auto ev = rec.first;
            if(have_common_regions(rec.second, sm1)){
                if(have_common_regions(rec.second, sm2)){
                    common_events = true;
                    break;
                }
            }
        }

        if(!common_regions && !common_events){
            cout << "No common events and no common regions" << endl;
            delete target_PN;
            return false;
        }

        auto PN_post_regions = new map<int, set<Region *> *>();

        //creation of  the map of post-regions of the new PN in order to check on a subset and not all possible regions
        for (auto rec: *post_regions) {
            if (PN_post_regions->find(rec.first) == PN_post_regions->end()) {
                (*PN_post_regions)[rec.first] = new set<Region *>();
            }
            for (auto reg: *rec.second) {
                if (target_PN->find(reg) != target_PN->end()) {
                    (*PN_post_regions)[rec.first]->insert(reg);
                }
            }
        }

        for (auto rec: *PN_post_regions) {
            int event = rec.first;
            //the event have at least 2 post-regions
            if (rec.second->size() > 1) {
                for (auto rec2: *PN_post_regions) {
                    //different events
                    if (rec.first != rec2.first) {
                        for (auto region: *rec.second) {
                            if (rec2.second->find(region) != rec2.second->end()) {
                                //cout << "couple of events: " << rec.first << " " << rec2.first << endl;
                                //cout << "not compatible SMs " << sm1 << " and " << sm2 << endl;
                                for(auto rec: *PN_post_regions){
                                    delete rec.second;
                                }
                                delete PN_post_regions;
                                delete target_PN;
                                return false;
                            }
                        }
                    }
                }
            }
        }
        if(decomposition_debug)
            cout << "compatible SMs " << sm1 << " and " << sm2 << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        for(auto rec: *PN_post_regions){
            delete rec.second;
        }
        delete PN_post_regions;
        delete target_PN;
        return true;
    }

    //todo: this code probably can be improved, maybe using sat in order to check if the union is possible
    __attribute__((unused)) SM* SMUnionForFCPTNetWithCheck(SM* sm1, SM* sm2, map<int, set<Region*> *> *post_regions) {
        SM *target_PN = new SM();
        for (auto reg: *sm1) {
            target_PN->insert(reg);
        }
        for (auto reg: *sm2) {
            target_PN->insert(reg);
        }

        //check if there are common regions
        bool common_regions = false;
        for(auto r1: *sm1){
            for(auto r2: *sm2){
                if(r1 == r2){
                    common_regions = true;
                    break;
                }
            }
            if(common_regions) break;
        }


        //check common labels
        bool common_events = false;
        for(auto rec: *post_regions){
            auto ev = rec.first;
            if(have_common_regions(rec.second, sm1)){
                if(have_common_regions(rec.second, sm2)){
                    common_events = true;
                    break;
                }
            }
        }

        if(!common_regions && !common_events){
            cout << "No common events and no common regions" << endl;
            delete target_PN;
            return nullptr;
        }

        auto PN_post_regions = new map<int, set<Region *> *>();

        //creation of  the map of post-regions of the new PN in order to check on a subset and not all possible regions
        for (auto rec: *post_regions) {
            if (PN_post_regions->find(rec.first) == PN_post_regions->end()) {
                (*PN_post_regions)[rec.first] = new set<Region *>();
            }
            for (auto reg: *rec.second) {
                if (target_PN->find(reg) != target_PN->end()) {
                    (*PN_post_regions)[rec.first]->insert(reg);
                }
            }
        }

        for (auto rec: *PN_post_regions) {
            int event = rec.first;
            //the event have at least 2 post-regions
            if (rec.second->size() > 1) {
                for (auto rec2: *PN_post_regions) {
                    //different events
                    if (rec.first != rec2.first) {
                        for (auto region: *rec.second) {
                            if (rec2.second->find(region) != rec2.second->end()) {
                                //cout << "couple of events: " << rec.first << " " << rec2.first << endl;
                                //cout << "not compatible SMs " << sm1 << " and " << sm2 << endl;
                                for(auto rec3: *PN_post_regions){
                                    delete rec3.second;
                                }
                                delete PN_post_regions;
                                delete target_PN;
                                return nullptr;
                            }
                        }
                    }
                }
            }
        }
        if(decomposition_debug)
            cout << "compatible SMs " << sm1 << " and " << sm2 << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        for(auto rec: *PN_post_regions){
            delete rec.second;
        }
        delete PN_post_regions;
        return target_PN;
    }

    void print_clause(vector<int32_t> *clause){
        for(auto val: *clause){
            cout << val << " ";
        }
        cout << endl;
    }


    void print_clause(set<int32_t> *clause){
        for(auto val: *clause){
            cout << val << " ";
        }
        cout << endl;
    }

    map<int, set<Region *>*>* get_map_of_used_regions(set<set<Region *> *> *SMs_or_PNs, map<int, set<Region *> *> *pre_regions){
        set<Region *> new_used_regions_tmp;
        for (auto tmp_SM: *SMs_or_PNs) {
            for (auto region: *tmp_SM) {
                new_used_regions_tmp.insert(region);
            }
        }
        auto new_used_regions_map_tmp = new map < int, set<Region *>* > ();
        for (auto rec: *pre_regions) {
            (*new_used_regions_map_tmp)[rec.first] = new set < Region * > ();
        }

        for (auto reg: new_used_regions_tmp) {
            for (auto rec: *pre_regions) {
                if (rec.second->find(reg) != rec.second->end()) {
                    (*new_used_regions_map_tmp)[rec.first]->insert(reg);
                }
            }
        }
        return new_used_regions_map_tmp;
    }

    __attribute__((unused)) void map_of_pre_regions_union(map<int, set<Region *> *> *map1, map<int, set<Region *> *> *output_map){
        for(auto rec2: *map1){
            auto ev = rec2.first;
            auto region_set = rec2.second;
            if(output_map->find(ev) == output_map->end()){
                (*output_map)[ev]= new set<Region *>();
            }
            for(auto reg: *region_set){
                (*output_map)[ev]->insert(reg);
            }
        }
    }

    map<int, set<set<Region *>*>*>* dnf_to_cnf(map<int, set<set<Region *>>*>* er_satisfiable_set){
        auto cnf_set = new map<int, set<set<Region *>*>*>();
        for(auto rec: *er_satisfiable_set){
            //cout  << "EV: " << rec.first << endl;
            //(*cnf_set)[rec.first] = new set<set<Region *>*>();
            set<set<Region *>>::iterator it;
            it = rec.second->begin();
            (*cnf_set)[rec.first] = dnf_to_cnf_core(rec.second, it);
        }
        return cnf_set;
    }

    set<set<Region *>*>* dnf_to_cnf_core(set<set<Region *>>*cl_set, set<set<Region *>>::iterator it){
        auto new_clauses = new set<set<Region *>*>();
        auto current_clause = *it;
        if(next(it) == cl_set->end()){
            for (auto val: current_clause) {
                auto tmp_cl = new set<Region *>();
                tmp_cl->insert(val);
                new_clauses->insert(tmp_cl);
            }
            /*
            for(auto cl: *new_clauses){
                println(cl);
            }
            cout << endl;*/
            return new_clauses;
        }
        auto next_clauses = dnf_to_cnf_core(cl_set, next(it));

        for (auto val: current_clause) {
            for (auto cl: *next_clauses) {
                auto tmp_cl = new set<Region *>();
                tmp_cl->insert(val);
                for (auto reg: *cl) {
                    tmp_cl->insert(reg);
                }
                new_clauses->insert(tmp_cl);
            }
        }
        for(auto cl: *next_clauses){
            delete cl;
        }
        delete next_clauses;
        return new_clauses;
    }

    vector<set<Region *>> *split_not_connected_regions(set<Region *> *pn, map<int, set<Region *> *> *connections){
        auto vector_of_sets = new vector<set<Region *>>();
        cout << "size: " << pn->size() << endl;
        for(auto reg: *pn){
            auto new_set = new set<Region *>();
            new_set->insert(reg);
            vector_of_sets->push_back(*new_set);
            delete new_set;
        }
        bool end = false;
        int last_size;
        while(!end) {
            last_size = vector_of_sets->size();
            //cout << "vector_of_sets_size: " << vector_of_sets->size() << endl;
            if (vector_of_sets->size() > 1) {
                for(int i= vector_of_sets->size()-1; i > 0; --i){
                    for(int k=i-1; k>=0; --k) {
                        if (are_connected((*vector_of_sets)[i], (*vector_of_sets)[k], connections)) {
                            for (auto reg: (*vector_of_sets)[i]) {
                                (*vector_of_sets)[k].insert(reg);
                            }
                            vector_of_sets->erase(vector_of_sets->begin() + i);
                            //i is decreased because the size of vector was decreased by 1,
                            //in case ok k == 0 there is no need to decrease i because for structure does it automatically
                            if(k > 0)
                                i--;
                        }
                    }
                }
            }
            if(vector_of_sets->size() == last_size) {
                end = true;
            }
            //cout << "vector_of_sets_size after: " << vector_of_sets->size() << endl;
        }
        return vector_of_sets;
    }

    bool are_connected(const set<Region *>& first, const set<Region *>& second, map<int, set<Region *> *> *connections){
        for(auto r1: first){
            for(auto r2: second){
                for(auto rec: *connections){
                    //tro regions are connected to the same event therefore these regions are connected
                    if(rec.second->find(r1) != rec.second->end() && rec.second->find(r2) != rec.second->end())
                        return true;
                }
            }
        }
        //cout << "found two not connected sets" << endl;
        return false;
    }

}
