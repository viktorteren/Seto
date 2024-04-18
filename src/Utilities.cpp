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
bool output;
bool decomposition_output_sis;
bool python_all;
bool ts_output;
bool ects_output;
bool no_merge;
bool dot_output;
bool composition;
bool bdd_usage;
bool info;
bool fcptnet;
bool acpn;
bool aut_output;
bool ignore_correctness;
bool no_bounds;
bool conformance_checking;
bool parallel;
bool python_available;
bool pn_synthesis;
bool no_fcpn_min;
bool benchmark_script;
bool greedy_exact;
bool check_structure;
bool mixed_strategy;
bool only_safeness_check;
bool safe_components;
bool safe_components_SM;
bool optimal;
bool no_reset;
bool count_SMs;
bool region_counter;
bool unsafe_path;
bool no_timeout;
bool counter_optimized;
int max_alias_decomp;
int num_clauses;
int places_after_initial_decomp;
int places_after_greedy;
int maxAlphabet;
double avgAlphabet;
map<int, Region*>* aliases_region_pointer;
map<Region*, int>* aliases_region_pointer_inverted;
map<Region*, int>* sm_region_aliases;
map<pair<Region*, Region*>, bool> *overlaps_cache;
map<set<Region*>, set<int>*> *intersection_cache;

namespace Utilities {
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
                // cout << "State: " << *it << endl;
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

    set<int> *regions_intersection(set<Region *> *regions) {

        auto pre_regions_intersection = new set<int>();
        bool state_in_intersection;

        if (*regions->begin() == nullptr)
            return pre_regions_intersection;

        for (auto state : **regions->begin()) {
            state_in_intersection = true;
            //todo: this is a possible improvement which should be tested
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

        auto pre_regions_intersection = new set<int>();
        bool state_in_intersection;

        if (*regions.begin() == nullptr)
            return pre_regions_intersection;

        for (auto state : **regions.begin()) {
            state_in_intersection = true;
            //todo: this is a possible improvement which should be tested
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

    bool empty_regions_intersection(Region *first, Region *second){
        for (auto state : *first) {
            if (second->find(state) != second->end()) {
                return false;
            }
        }
        return true;
    }

    bool at_least_one_state_from_first_in_second(Region *first, Region *second){
        for (auto state : *first) {
            if (second->find(state) !=
                second->end()) { // the state is found (belongs to both)
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

    void print(Region &region, std::ofstream *st) {
        int pos = 0;
        auto size = static_cast<int>(region.size());
        for (auto state : region) {
            pos++;
            *st << state;
            if (pos != size) {
                *st << ",";
            }
        }
    }

    void println(Region &region) {
        print(region);
        cout << endl;
    }

    void println(Region &region, std::ofstream *st) {
        print(region, st);
        *st << endl;
    }

    bool is_a_region(set<int> *set_of_states){
        for(const auto& rec: *ts_map){
            bool enter = false;
            bool exit = false;
            bool no_cross = false;
            for(auto edge: rec.second){
                if(set_of_states->find(edge->first) != set_of_states->end()){
                    //no cross
                    if(set_of_states->find(edge->second) != set_of_states->end()){
                        if(enter || exit)
                            return false;
                        if(!no_cross)
                            no_cross = true;
                    }
                    //exit
                    else{
                        if(enter || no_cross)
                            return false;
                        if(!exit)
                            exit = true;
                    }
                }
                else{
                    //enter
                    if(set_of_states->find(edge->second) != set_of_states->end()){
                        if(exit || no_cross)
                            return false;
                        if(!enter)
                            enter = true;
                    }
                    //no cross
                    else{
                        if(exit || enter)
                            return false;
                        if(!no_cross)
                            no_cross = true;
                    }
                }
            }
        }
        return true;
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
                if(!empty_regions_intersection(r1,r2))
                    return false;
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

    bool is_bigger_than_or_equal_to(Region *region, set<int> *intersection) {

        if (region->size() >= intersection->size()) {
            //cout << "TRUE**************" << endl;
            return true;
        }

        for (auto elem : *intersection) {
            // in the region cannot find an element of the intersection
            if (region->find(elem) == region->end()) {
                //cout << "FALSE**************" << endl;
                return false;
            }
        }

        // in the region all elements of the intersection are found
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

    bool contains(vector<set<int>> *container, const set<int>& result_to_check){
        for (const auto& elem : *container) {
            if (are_equal(elem, result_to_check)) {
                return true;
            }
        }
        return false;
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
    inline bool contains(T bigger_set, T2 smaller_set) {
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

    bool contains(const set<Region *>& container, const Region& region) {
        for (auto elem : container) {
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

    bool contains(set<set<Region *>>* set_of_sets, set<Region *> *reg_set){
        for(const auto& elem: *set_of_sets){
            if(elem == *reg_set)
                return true;
        }
        return false;
    }

    bool contains(set<Region *> *bigger_set, set<Region *> *smaller_set) {
        for (auto elem : *smaller_set) {
            if(bigger_set->find(elem) == bigger_set->end())
                return false;
        }
        return true;
    }

    bool contains(const set<Region *>& reg_set, Region * reg){
        if(reg_set.find(reg) != reg_set.end())
            return true;
        return false;
    }

    bool contains(set<int> bigger_set, const set<int>& smaller_set){
        for (auto elem : smaller_set) {
            if(bigger_set.find(elem) == bigger_set.end())
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
            //the event is an alias
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
            //the event is an alias
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

    void print_ts_aut_file(string file_path,
                           map <map<set<Region *>*, set<Region *>>, int> *state_aliases,
                           vector<edge> *arcs,
                           const map<set<Region *>*, set<Region *>>& initial_state_TS){
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


        if(decomposition){
            output_name = output_name + "_composed_SM.aut";
        }
        else{
            output_name = output_name + "_composed.aut";
        }



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
            fout << ")\n";
        }

        fout.close();
    }

    void print_ts_dot_file(string file_path,
                           map <map<set<Region *>*, set<Region *>>, int> *state_aliases,
                           vector<edge> *arcs,
                           const map<set<Region *>*, set<Region *>>& initial_state_TS){
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

        output_name = output_name + "_composed.dot";


        ofstream fout(output_name);
        fout << "digraph ";
        fout << in_name;
        fout << "{\n";
        fout << "\tlabel=\"(name=" << in_name << ",n=" << state_aliases->size()
             << ",m=" << arcs->size() << ")\";\n";
        fout << "\t_nil [style = \"invis\"];\n";
        fout << "\tnode [shape = doublecircle]; ";
        fout << state_aliases->at(initial_state_TS) << ";\n";
        fout << "\tnode [shape = circle];\n";
        fout << "\t_nil -> ";
        fout << state_aliases->at(initial_state_TS) << ";\n";

        for(const auto& arc: *arcs){
            fout << "\t";
            fout << state_aliases->at(arc.start);
            fout << "->";
            fout << state_aliases->at(arc.end);
            fout << "[label=\"";
            fout << arc.event;
            fout << "\"];\n";
        }

        fout << "}\n";
        fout.close();
    }


    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses){
        return convert_to_dimacs(std::move(file_path), num_var, num_clauses, clauses, nullptr);
    }

    string convert_to_dimacs(string file_path, int num_var, int num_clauses, const vector<vector<int32_t>>& clauses,
                             vector<set<int>>* new_results_to_avoid){
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

    void print_fcpn_dot_file(map<int, set<Region *> *> *pre_regions,
                             map<int, set<Region *> *> *post_regions,
                             map<int, int> *aliases, const string& file_name, int FCPN_number){
        print_pn_dot_file(nullptr, pre_regions, post_regions, aliases, file_name, FCPN_number);
    }

    void print_pn_dot_file(map<Region *, int> *regions_mapping,
                           map<int, set<Region *> *> *pre_regions,
                           map<int, set<Region *> *> *post_regions,
                           map<int, int> *aliases,
                           const string& file_name,
                           int PN_number,
                           bool unsafe){
        auto initial_reg = initial_regions(pre_regions);
        string output_name = file_name;
        string in_dot_name;
        string output;
        bool delete_regions_mapping = false;
        // creation of the map between a pointer to the region and a unique corresponding integer
        if(regions_mapping == nullptr){
            regions_mapping = get_regions_map(pre_regions);
            delete_regions_mapping = true;
        }

        /*cout << "preregions before print" << endl;
        print(*pre_regions);*/
        auto regions_set = copy_map_to_set(pre_regions);
        /*cout << "regions set " << endl;
        println(*regions_set);*/
        auto not_initial_regions = region_pointer_difference(regions_set, initial_reg);

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
        if(unsafe){
            output_name += "_UNSAFE";
        }
        if(PN_number >= 0){
            if(acpn)
                output_name += "_ACPN_";
            else if(fcptnet)
                output_name += "_FCPN_";
            else if(decomposition)
                output_name += "_SM_";
            output_name+=std::to_string(PN_number);
            output_name+=".dot";
        }
        else{
            output_name += "_PN.dot";
        }
        //cout << "file output PN: " << output_name << endl;

        ofstream fout(output_name);
        fout << "digraph ";
        if(PN_number >= 0){
            if(acpn)
                fout << in_dot_name + "_ACPN_";
            else if(fcptnet)
                fout << in_dot_name + "_FCPN_";
            else if(decomposition)
                fout << in_dot_name + "_SM_";
            fout << std::to_string(PN_number);
        }
        else{
            fout << in_dot_name + "_PN";
        }

        fout << "{\n";
        // initial regions
        //cout << "writing initial regions" << endl;
        fout << "subgraph initial_place {\n"
                "\tnode [shape=doublecircle,fixedsize=true, fixedsize = 2, color = "
                "black, fillcolor = gray, style = filled];\n";
        for (auto reg : *initial_reg) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
        }

        fout << "}\n";
        // not initial regions
        fout << "subgraph place {     \n"
                "\tnode [shape=circle,fixedsize=true, fixedsize = 2];\n";
        for (auto reg : *not_initial_regions) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
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
                //cout<<"debug alias counter of "<< record.second << (*alias_counter)[record.second]<<endl;
                fout << "/" << (*alias_counter)[record.second];
                fout << "\"];\n";
            }
            else{
                (*alias_counter)[record.second]++;
            }
        }
        delete alias_counter;
        //transition (events) initial
        for (auto record : *pre_regions) {
            if (record.first < num_events_before_label_splitting) {
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
            if (record.first < num_events_before_label_splitting) {
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

        //arcs between transitions and places (events and regions)
        //region -> event
        for (auto record : *pre_regions) {
            for (auto reg : *record.second) {
                if (record.first < num_events_before_label_splitting) {
                    //if (regions_mapping->find(reg) != regions_mapping->end()) {
                    fout << "\tr" << regions_mapping->at(reg) << " -> "
                         << record.first << ";\n";

                    //} else {
                    //cout << "regions_mapping does not contain ";
                    // println(*reg);
                    //}
                } else {
                    //int label=aliases->at(record.first);
                    if (regions_mapping->find(reg) != regions_mapping->end()) {
                        fout << "\tr" << regions_mapping->at(reg) << " -> "
                             << record.first << ";\n";
                    } else {
                        //cout << "regions_mapping does not contain ";
                        //println(*reg);
                    }
                }
            }
        }
        //event -> region
        for (auto record : *post_regions) {
            for (auto reg : *record.second) {
                if (regions_mapping->find(reg) != regions_mapping->end()) {
                    fout << "\t" << record.first << " -> "
                         << "r" << regions_mapping->at(reg) << ";\n";
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

    void print_sm_dot_file(map<Region *, int> *regions_mapping,
                           map<int, Region *> *pre_regions,
                           map<int, Region *> *post_regions,
                           map<int, int> *aliases,
                           const string& file_name,
                           int SM_number){
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
        if(regions_mapping == nullptr){
            regions_mapping = get_regions_map(pre_regions);
            delete_regions_mapping = true;
        }

        auto regions_set = copy_map_to_set(pre_regions);
        auto not_initial_regions =
                region_pointer_difference(regions_set, initial_reg);

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

        if(SM_number >= 0){
            if(acpn)
                output_name += "_ACPN_";
            else if(fcptnet)
                output_name += "_FCPN_";
            else if(decomposition)
                output_name += "_SM_";
            output_name+=std::to_string(SM_number);
            output_name+=".dot";
        }
        else{
            if(acpn)
                output_name += "_ACPN.dot";
            else if(fcptnet)
                output_name += "_FCPN.dot";
            else if(decomposition)
                output_name += "_SM.dot";
        }
        //cout << "file output PN: " << output_name << endl;

        ofstream fout(output_name);
        fout << "digraph ";
        if(SM_number >= 0){
            if(acpn)
                fout << in_dot_name + "_ACPN_";
            else if(fcptnet)
                fout << in_dot_name + "_FCPN_";
            else if(decomposition)
                fout << in_dot_name + "_SM_";
            else
                fout << in_dot_name + "_PN_";
            fout << std::to_string(SM_number);
        }
        else{
            if(acpn)
                fout << in_dot_name + "_ACPN";
            else if(fcptnet)
                fout << in_dot_name + "_FCPN";
            else if(decomposition)
                fout << in_dot_name + "_SM";
            else
                fout << in_dot_name + "_PN";
        }

        fout << "{\n";
        fout << "subgraph initial_place {\n"
                "\tnode [shape=doublecircle,fixedsize=true, fixedsize = 2, color = "
                "black, fillcolor = gray, style = filled];\n";
        for (auto reg : *initial_reg) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
        }

        fout << "}\n";
        fout << "subgraph place {     \n"
                "\tnode [shape=circle,fixedsize=true, fixedsize = 2];\n";
        for (auto reg : *not_initial_regions) {
            fout << "\tr" << regions_mapping->at(reg) << ";\n";
        }
        fout << "}\n";

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
                //cout<<"debug alias counter of "<< record.second << (*alias_counter)[record.second]<<endl;
                fout << "/" << (*alias_counter)[record.second];
                fout << "\"];\n";
            }
            else{
                (*alias_counter)[record.second]++;
            }
        }
        delete alias_counter;
        for (auto record : *pre_regions) {
            if (record.first < num_events_before_label_splitting) {
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
            if (record.first < num_events_before_label_splitting) {
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


        for (auto record : *pre_regions) {
            auto reg = record.second;
            if (record.first < num_events_before_label_splitting) {
                //if (regions_mapping->find(reg) != regions_mapping->end()) {
                fout << "\tr" << regions_mapping->at(reg) << " -> "
                     << record.first << ";\n";
            } else {
                //int label=aliases->at(record.first);
                if (regions_mapping->find(reg) != regions_mapping->end()) {
                    fout << "\tr" << regions_mapping->at(reg) << " -> "
                         << record.first << ";\n";
                }
            }
        }
        //event -> region
        for (auto record : *post_regions) {
            auto reg = record.second;
            if (regions_mapping->find(reg) != regions_mapping->end()) {
                fout << "\t" << record.first << " -> "
                     << "r" << regions_mapping->at(reg) << ";\n";
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
        map<Region *, int> *regions_mapping;
        auto regions_set = copy_map_to_set(pre_regions);
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
            if(ev >= num_events_before_label_splitting){
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
                if (record.first < num_events_before_label_splitting) {
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


        //region -> event
        for(auto record: *pre_regions_inverted){
            auto reg = record.first;
            fout << "r" << regions_mapping->at(reg);
            for(auto ev: *record.second){
                if(g_input){
                    if(ev >= num_events_before_label_splitting){
                        int original_label = (*aliases)[ev];
                        fout << " " << (*aliases_map_number_name)[original_label];
                        fout << "/" << alias_event_index_map[ev];
                    }
                    else{
                        fout << " " << (*aliases_map_number_name)[ev];
                    }
                }
                else {
                    if (ev < num_events_before_label_splitting) {
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
                if(record.first >= num_events_before_label_splitting){
                    int original_label = (*aliases)[record.first];
                    fout << (*aliases_map_number_name)[original_label];
                    fout << "/" << alias_event_index_map[record.first];
                }
                else{
                    fout << (*aliases_map_number_name)[record.first];
                }
            }
            else {
                if (record.first < num_events_before_label_splitting) {
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

    //TODO: check for memory leaks
    void print_cc_component_dot_file(Region *region,
                                     map<int, set<Region  *> *> *pre_regions,
                                     map<int, set<Region *> *> *post_regions,
                                     map<int, int> *aliases,
                                     string file_name,
                                     int component_counter){
        bool initial = is_initial_region(region);

        auto incoming_events = new set<int>();
        for(auto rec: *post_regions){
            if(rec.second->find(region) != rec.second->end()){
                incoming_events->insert(rec.first);
            }
        }

        auto outgoing_events = new set<int>();
        for(auto rec: *pre_regions){
            if(rec.second->find(region) != rec.second->end()){
                outgoing_events->insert(rec.first);
            }
        }

        if(component_counter == 3){
            cout << endl;
        }


        string output_name = file_name;
        string in_dot_name;
        string output;

        map<Region *,int> *regions_mapping = get_regions_map(pre_regions);
        bool delete_regions_mapping = true;


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

        output_name += "_component_";
        output_name+=std::to_string(component_counter);
        output_name+=".dot";

        ofstream fout(output_name);
        fout << "digraph ";
        fout << in_dot_name + "_component_";
        fout << std::to_string(component_counter);


        fout << "{\n";
        // initial regions
        fout << "subgraph initial_place {\n"
                "\tnode [shape=doublecircle,fixedsize=true, fixedsize = 2, color = "
                "black, fillcolor = gray, style = filled];\n";
        if(initial){
            fout << "\tr" << regions_mapping->at(region) << ";\n";
        }

        fout << "}\n";
        // not initial regions
        fout << "subgraph place {     \n"
                "\tnode [shape=circle,fixedsize=true, fixedsize = 2];\n";
        if(!initial){
            fout << "\tr" << regions_mapping->at(region) << ";\n";
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
            bool found = false;
            if(incoming_events->find(record.first) != incoming_events->end()){
                found = true;
            }
            else if(outgoing_events->find(record.first)!= outgoing_events->end()){
                found = true;
            }

            if(found) {
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
                //cout<<"debug alias counter of "<< record.second << (*alias_counter)[record.second]<<endl;
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
        //transition (events) initial
        for (auto ev : *incoming_events) {
            if (ev < num_events_before_label_splitting) {
                if (g_input) {
                    fout << "\t" << ev << " [label = \""
                         << (*aliases_map_number_name)[ev];
                    fout << "\"];\n";
                } else {
                    fout << "\t" << ev << ";\n";
                }
            }
        }
        for (auto ev : *outgoing_events) {
            if (ev < num_events_before_label_splitting) {
                if (g_input) {
                    fout << "\t" << ev << " [label = \""
                             << (*aliases_map_number_name)[ev];
                    fout << "\"];\n";
                } else {
                    fout << "\t" << ev << ";\n";
                }
            }
        }
        fout << "}\n";

        //arcs between transitions and places (events and regions)
        //region -> event
        for(auto ev: *outgoing_events){
            fout << "\t" << "r" << regions_mapping->at(region) << " -> "
                  << ev << ";\n";
        }
        //event -> region
        for(auto ev: *incoming_events){
            fout << "\t" << ev << " -> "
                 << "r" << regions_mapping->at(region) << ";\n";
        }
        fout << "}";
        fout.close();
        delete regions_mapping;
        delete incoming_events;
        delete outgoing_events;
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
            if (record.first < num_events_before_label_splitting) {
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
            if (record.first < num_events_before_label_splitting) {
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
            if (record.first < num_events_before_label_splitting) {
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

    set<Region *> *region_pointer_union(set<Region *> *first,
                                        set<Region *> *second) {
        auto un = new set<Region *>(*first);
        for (auto reg : *second) {
            un->insert(reg);
        }
        return un;
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

    map<int, set<Region *> *>* merge_2_maps(map<int, Region *> *first, map<int, Region *> *second) {
        //  cout << "MERGING ESSENTIAL AND IRREDUNDANT REGIONS**********" << endl;

        map<int, set<Region *> *> *total_pre_regions_map = nullptr;
        total_pre_regions_map = new map<int, set<Region *> *>();

        if (second != nullptr) {
            for (int event = 0; event < num_events_after_splitting; event++) {
                // cout << "evento:" << event << endl;

                // found both events
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
                }
                    // event only in first(essential)
                else if (first->find(event) != first->end()) {
                    auto merged_vector = new vector<Region *>(1);
                    (*total_pre_regions_map)[event] = new set<Region *>();
                    (*total_pre_regions_map)[event]->insert(first->at(event));
                    delete merged_vector;
                }
                    // event only in second(irredundant)
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

    bool check_sat_formula_from_dimacs(Minisat::Solver& solver, const string& file_path){
        FILE *f;
        f = fopen(file_path.c_str(), "r");
        parse_DIMACS(f, solver);
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
        }
        //false return value
        else if (ret == Minisat::lbool((uint8_t)1))
            return false;
        else {
            cerr << "NO RESULT" << endl;
            exit(1);
        }
    }

    bool is_excitation_closed(map<int, set<Region *> *> *pre_regions, map<int, ES> *ER_set ) {
        auto regions_intersection_map = new map<int, set<int> *>() ;
        //for (auto item : *pre_regions) {
        //cout<<"num evens after splitting "<<num_events_after_splitting<<endl;
        for(int event=0;event<num_events_after_splitting;++event){
            //cout << "event: " << item.first;
            //auto event = item.first;
            auto er = ER_set->at((event));
            //cout << "ER at" << event << " : " << endl;
            //println(*er);
            //the event does not have pre-regions: EC not satisfied
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
                    // cout << "region of the event:" << event;
                    if(print_step_by_step_debug || decomposition_debug){
                        cout << "event " << event << " not satisfy ec because the intersection of regions is different from er" << endl;
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
        if(pre_regions->empty()){
            cerr << "The function getTransitionsAvg received empty pre-regions map" << endl;
            exit(1);
        }
        int sum = 0;
        int cont = 0;
        for(auto rec: *pre_regions){
            sum += rec.second->size();
            cont++;
        }
        return  ((double)sum/cont);
    }

    double getTransitionsVar(map<SM*, map<int, Region *>*> *pre_regions){
        if(pre_regions->empty()){
            cerr << "The function getTransitionsVar received empty pre-regions map" << endl;
            exit(1);
        }
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
                if(label < num_events_before_label_splitting){
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
                if(label < num_events_before_label_splitting){
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

    template <typename T>
    double getAvgAlphabet(T *pre_regions, map<int, int> *label_aliases){
        if(pre_regions->empty()){
            cerr << "The function getAvgAlphabet received empty pre-regions map" << endl;
            exit(1);
        }
        int sum = 0;
        for(auto rec: *pre_regions){
            auto SM_map = rec.second;
            int counter = 0;
            set<int> used_labels;
            for(auto record: *SM_map){
                auto label = record.first;
                if(label < num_events_before_label_splitting){
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
    template double getAvgAlphabet(map < set<Region *> *, map<int, Region *> * > *pre_regions, map<int, int> *label_aliases);
    template double getAvgAlphabet(map < set<Region *> *, map<int, set<Region *>*> * > *pre_regions, map<int, int> *label_aliases);

    void print_clause(vector<int32_t> *clause){
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
        //cout << "size: " << pn->size() << endl;
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
                    //the regions are connected to the same event therefore these regions are connected
                    if(rec.second->find(r1) != rec.second->end() && rec.second->find(r2) != rec.second->end())
                        return true;
                }
            }
        }
        //cout << "found two not connected sets" << endl;
        return false;
    }

    void println_simplified(set<Region *> *regions, map<Region *, int> *regions_alias_mapping) {
        for (auto reg : *regions) {
            cout << "r" << regions_alias_mapping->at(reg) << ": ";
            println(*reg);
        }
    }

    bool safeness_check(set<Region *> *pn,
                        map<int, set<Region*> *> *map_of_pre_regions,
                        map<int, set<Region*> *> *map_of_post_regions,
                        map<Region *, int> *regions_alias_mapping) {
        set<Region *> *current_marking;
        map<set<Region *> *, vector<set<Region *>*>*> path;
        auto initial_regions=new set<Region *>();
        for(auto reg: *pn){
            if(reg->find(initial_state) != reg->end()){
                initial_regions->insert(reg);
            }
        }
        current_marking = initial_regions;
        vector<set<Region *>*> to_visit;
        set<set<Region *>> completely_explored_states;

        path[initial_regions] = new vector<set<Region *>*>();


        auto post_events = new map<Region *, set<int> *>();
        for(auto rec: *map_of_pre_regions){
            auto ev = rec.first;
            for(auto reg: *rec.second){
                if(post_events->find(reg) == post_events->end()){
                    (*post_events)[reg] = new set<int>();
                }
                (*post_events)[reg]->insert(ev);
            }
        }

        do{
            set<int> checked_events;
            for(auto reg: *current_marking){
                for(auto ev: *post_events->at(reg)){
                    bool can_fire = true;
                    if(checked_events.find(ev) == checked_events.end()){
                        checked_events.insert(ev);
                        for(auto pre_reg: *map_of_pre_regions->at(ev)){
                            if(pn->find(pre_reg) != pn->end()){
                                if(current_marking->find(pre_reg) == current_marking->end()){
                                    can_fire = false;
                                    break;
                                }
                            }
                        }
                        if(can_fire){
                            auto new_set = new set<Region *>();
                            for(auto reg1: *current_marking){
                                //region don't have to be moved after firing
                                if(map_of_pre_regions->at(ev)->find(reg1) == map_of_pre_regions->at(ev)->end()){
                                    new_set->insert(reg1);
                                }
                            }
                            for(auto reg2: *map_of_post_regions->at(ev)){
                                if(pn->find(reg2) != pn->end()) {
                                    if (current_marking->find(reg2) == current_marking->end()) {
                                        new_set->insert(reg2);
                                    }
                                    //unsafe marking
                                    else {
                                        if(regions_alias_mapping != nullptr && unsafe_path) {
                                            cout << "unsafe place: r" << regions_alias_mapping->at(reg2) << endl;
                                            cout << "path to arrive to it:"<< endl;
                                            auto reg_set_vec = path.at(current_marking);
                                            for(auto reg_set: *reg_set_vec){
                                                for(auto r: *reg_set){
                                                    cout << "r" << regions_alias_mapping->at(r) << ",";
                                                }
                                                cout << endl;
                                                //println(*reg_set);
                                            }
                                            exit(0);
                                        }
                                        for(auto rec: *post_events){
                                            delete rec.second;
                                        }
                                        delete post_events;
                                        for(auto rec: path){
                                            delete rec. second;
                                        }
                                        delete initial_regions;
                                        delete new_set;
                                        return false;
                                    }
                                }
                            }
                            //cout << "adding new set" << endl;
                            //println(new_set);
                            bool exists = false;
                            for(auto & i : to_visit){
                                if(*i == *new_set) {
                                    exists = true;
                                    break;
                                }
                            }
                            if(!exists) {
                                if(completely_explored_states.find(*new_set) == completely_explored_states.end()) {
                                    to_visit.push_back(new_set);
                                    path[new_set] = new vector<set<Region *>*>();
                                    auto reg_set_vector = path.at(current_marking);
                                    for(auto reg_set: *reg_set_vector){
                                        path.at(new_set)->push_back(reg_set);
                                    }
                                    path.at(new_set)->push_back(current_marking);
                                }
                            }
                            else {
                                if(decomposition_debug){
                                    cout << "cache hit!!!" << endl;
                                    println(new_set);
                                }
                                delete new_set;
                            }
                        }
                    }
                }
            }
            completely_explored_states.insert(*current_marking);
            current_marking = to_visit.at(to_visit.size()-1);
            to_visit.pop_back();
        }while(!to_visit.empty());
        for(auto rec: *post_events){
            delete rec.second;
        }
        delete post_events;
        for(auto rec: path){
            delete rec. second;
        }
        delete initial_regions;
        return true;
    }

    bool contains(vector<int32_t> *bigger_clause, vector<int32_t> *smaller_clause){
        for(auto lit: *smaller_clause){
            bool found = false;
            for(int i : *bigger_clause){
                if(i == lit){
                    found = true;
                }
            }
            if(!found)
                return false;
        }
        return true;
    }
}
