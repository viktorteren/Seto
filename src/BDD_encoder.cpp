/*
 * This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */

#include "include/BDD_encoder.h"
#include "libs/cudd-3.0.0/st/st.h"

using namespace Utilities;

//complexity (worst case): num_events*num_regions!
BDD_encoder::BDD_encoder(map<int, set<set<int> *> *> *pre_regions, map<int, ER> *ER_map) {
    //TODO: implement a mechanism which stops without having explored all possible EC combinations
    // a possible result could be a normal computation for n cycles and at this point a search for only events without
    // any set of regions for their EC
    cout << "==============[BDD ENCODER]===============" << endl;
    valid_sets = new map<int, set<set<Region *>>*>();
    invalid_sets = new map<int, set<set<Region *>>*>();
    // [IN ITALIAN]
    // qui dovrei scrivere un algoritmo che:
    // 1) prende gli eventi uno ad uno
    // 2) per questo evento prende tutte le pre-regioni e l'ER
    // 3) cerca tutte le combinazioni (minimali) di pre-regioni per soddisfare l'EC per tale evento
    // 4) potrei usare un codice ricorsivo: partendo da ogni regione, l'ordine ottimale sarebbe però
    // quello che segue una BFS e non DFS: prima controllo le singole regioni, poi coppie ...
    // Quindi scorro per ogni regione e faccio il controllo, salvando i vincitori nella cache
    // una volto scorso per tutti aggiungo solo quelli che sono maggiori dei precedenti e non sono nella
    // cache come vincenti e non contengono un vincente (significa che sono minimali)
    int approx_counter = 0;
    for(auto rec: *pre_regions){
        auto event = rec.first;
        //cout << "event: " << event << endl;
        (*valid_sets)[event] = new set<set<Region *>>();
        (*invalid_sets)[event] = new set<set<Region *>>();
        auto pre_region_set_for_event = rec.second;
        auto event_ER = ER_map->at(event);
        auto secondary_regions = new vector<Region *>(); //regions that cannot satisfy EC alone
        for(auto pre_region: *pre_region_set_for_event){
            bool enough = test_if_enough(event_ER,pre_region);
            auto temp = new set<Region *>();
            temp->insert(pre_region);
            if(enough){
                valid_sets->at(event)->insert(*temp);
                /*cout << "valid sets: \n";
                for(auto rec: *valid_sets){
                    cout << "new set: \n";
                    auto tmp = rec.second;
                    for(auto tmp2: *tmp){
                        println(tmp2);
                    }
                }*/
            }
            else{
                invalid_sets->at(event)->insert(*temp);
                secondary_regions->push_back(pre_region);
                if(invalid_sets->at(event)->size() > 45){
                    cerr << "TOO MANY EVENTS FOR BDD EXECUTION" << endl;
                    exit(1);
                }
            }
        }

        auto to_add_later = new set<set<Region *>>();
        auto cache = set<set<Region *>*>();
        do {
            for (const auto& reg_set: *to_add_later) {
                invalid_sets->at(event)->insert(reg_set);
            }
            to_add_later->clear();
            for (const auto &reg_set: *invalid_sets->at(event)) {
                for (auto reg: *secondary_regions) {
                    if (!contains(reg_set, reg)) {
                        auto temp_set = new set<Region *>(reg_set.begin(), reg_set.end());
                        temp_set->insert(reg);
                        if(cache.find(temp_set) == cache.end()) {
                            cache.insert(temp_set);
                            //if temp_set is already in invalid_set it means that it was already analyzed
                            if (!contains(invalid_sets->at(event), temp_set)) {
                                bool contains_a_valid_set = false;
                                //if temp_set contains at least one of the sets of valid_set it means that temp_set is
                                // redundant
                                for (const auto &valid_set: *valid_sets->at(event)) {
                                    if (contains(*temp_set, valid_set)) {
                                        contains_a_valid_set = true;
                                        break;
                                    }
                                }
                                if (!contains_a_valid_set) {
                                    bool enough = test_if_enough(event_ER, temp_set);
                                    if (enough) {
                                        valid_sets->at(event)->insert(*temp_set);
                                        /*cout << "valid sets: \n";
                                        for(auto rec: *valid_sets){
                                            cout << "new set: \n";
                                            auto tmp = rec.second;
                                            for(auto tmp2: *tmp){
                                                println(tmp2);
                                            }
                                        }*/
                                    } else {
                                        to_add_later->insert(*temp_set);
                                        //cout << "to add later: \n";
                                        //println(temp_set);
                                    }
                                } else {
                                    delete temp_set;
                                }
                            } else {
                                delete temp_set;
                            }
                        }
                        else{
                            //cout << "CACHE HIT!!!" << endl;
                        }
                    }
                }
            }
        } while (!to_add_later->empty());
        delete to_add_later;
        delete secondary_regions;
        //map_of_EC_clauses = dnf_to_cnf(valid_sets);
    }
}

BDD_encoder::~BDD_encoder() {
    for(auto rec: *valid_sets){
        delete rec.second;
    }
    delete valid_sets;
    for(auto rec: *invalid_sets){
        delete rec.second;
    }
    delete invalid_sets;
    delete map_of_EC_clauses;
}

bool BDD_encoder::test_if_enough(ER er, set<Region *> *regions) {
    auto intersection = regions_intersection(regions);
    if(*er == *intersection){
        return true;
    }
    return false;
}

bool BDD_encoder::test_if_enough(ER er, Region *region) {
    if(*region == *er)
        return true;
    return false;
}

void BDD_encoder::encode(set<Region *> *regions, map<Region *, int> *regions_alias_mapping, map<int, set<set<int> *> *> *pre_regions){
    Cudd mgr(0,0);
    //regions_bdd_map is a map containing a vector of bdd variables, one BDD for each FCPN
    map<Region *, BDD> regions_bdd_map;
    map<int, BDD> event_bdd_encodings;
    //creation of variables for each region
    for(auto reg: *regions){
        regions_bdd_map[reg]=mgr.bddVar();
    }

    if(decomposition_debug){
        cout << "number of regions: " << regions->size()<<endl;
        cout << "valid sets" << endl;
        for(auto rec: *valid_sets){
            cout << "ev: " << rec.first << endl;
            for(auto reg_set: *rec.second){
                println(reg_set);
                cout << "--" << endl;
            }
        }
    }

    FILE* fstdout = stdout;

    //todo: rifare usando la notazione vecchia oppure scoprire l'uso di restrict con questa notazione
    //BDD creation
    for(int i=0; i < num_events_after_splitting; ++i){
        event_bdd_encodings[i] = mgr.bddZero();
        auto current_set = valid_sets->at(i);
        for(const auto& combination: *current_set){
            BDD current = mgr.bddOne();
            for (auto reg: combination) {
                current *= regions_bdd_map[reg];
            }
            event_bdd_encodings[i] += current;
        }
    }

    char const* inames[regions->size()];
    char const* onames[num_events_after_splitting];
    int k=0;
    for(auto reg: *regions){
        auto tmp = new string();
        *tmp = "r";
        tmp->append(to_string(regions_alias_mapping->at(reg)));
        inames[k] = tmp->c_str();
        k++;
    }

    char const *inames_without_r[regions->size()];
    int inames_int[regions->size()];

    //removing from inames r, leaving only the index
    for(int i=0;i < regions->size();++i){
        string tmp = inames[i];
        std::size_t pos = 1;
        auto tmp2 = tmp.substr(pos);
        char* tmp3=new char();
        strcpy(tmp3,tmp2.c_str());
        inames_without_r[i] = tmp3;
        inames_int[i] = stoi(inames_without_r[i]);
    }

    //assign for each value of onames a char array with inside the integer of the position
    for(int i=0; i < num_events_after_splitting; ++i){
        auto tmp = new string();
        tmp->append(to_string(i));
        onames[i] = tmp->c_str();
    }

    DdManager* manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

    //todo: the number assigned to the node is not the same of the region
    vector<DdNode *> inodes;
    for(int counter = 0; counter < regions->size();++counter){
        auto tmp = Cudd_bddIthVar(manager, counter);
        inodes.push_back(tmp);
    }

    auto t = event_bdd_encodings.at(0);
    //DdNode *restrictBy;
    //restrictBy = Cudd_bddAnd(manager, inodes.at(0));
    //Cudd_Ref(restrictBy);

    //todo: provare a fare un esempio con una tavola di verità completa con 2 variabili e poi estendere

    /*
      CODE EXAMPLE:
      x1 x2 | out
      0  0  |  1
      0  1  |  0
      1  0  |  1
      1  1  |  0

      out = (!x1 A !x2) V (x1 A !x2)
      x1 and x2 are inodes[0] e inodes[1]


    DdNode* first = Cudd_bddAnd(manager, Cudd_Not(inodes[0]), Cudd_Not(inodes[1]));
    DdNode* second = Cudd_bddAnd(manager, inodes[0], Cudd_Not(inodes[1]));
    DdNode* out = Cudd_bddOr(manager, first, second);

    DdNode* restrictBy = Cudd_bddAnd(manager, Cudd_Not(inodes[0]), Cudd_Not(inodes[1]));
    auto testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 0, x2 = 0, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

    restrictBy = Cudd_bddAnd(manager, Cudd_Not(inodes[0]), inodes[1]);
    testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 0, x2 = 1, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

    restrictBy = Cudd_bddAnd(manager, inodes[0], Cudd_Not(inodes[1]));
    testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 1, x2 = 0, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

    restrictBy = Cudd_bddAnd(manager, inodes[0], inodes[1]);
    testOut = Cudd_bddRestrict(manager, out, restrictBy);
    cout <<  "x1 = 1, x2 = 1, out = " << (1 - Cudd_IsComplement(testOut)) << endl;

     */

     /*
      * regions_bdd_map mappa le regioni e BDD da qui posso passare al DdNode e creare le restrizioni
      * quando creo le combinazioni di regioni in input il bello è che il restrictBy può essere fatto per ogni evento
      * o forse mi sto sbagliando, basterebbe limitarsi alle pre-regioni di quel evento quindi gli altri sono DC
      * sia nel restrict by che nella creazione delle clausole
      */

    for(int i=0; i < num_events_after_splitting; ++i){
        auto regs = pre_regions->at(i); //regions's set
        auto regs_vector = new vector<Region *>(regs->begin(), regs->end());
        //todo: here I have to create all binary combinations between these regions
        for(int counter = 0; counter < pow(2,regs->size());++counter){ //all possible combinations between [0, 2^(num event pre-regions) - 1]
            //todo: encoding of counter into a vector

            //todo: for each region of the vector encode the restriction and try the final result

            //todo: i the result is 0 then create a clause
        }
    }




    exit(1);

    //todo: data la tavola di verità con i valori 0 creare le clausole cambiando i valori
    //esempio: x1=0,x2=1, out=0 -> (x1 v !x2)
    //ogni riga che porta a 0 è una clausola




    /*
    for(auto str: inames){
        cout << str << endl;
    }

    for(auto str: onames){
        cout << str << endl;
    }*/

    DdNode *Dds[num_events_after_splitting];
    for(int i=0; i < num_events_after_splitting; ++i){
        Dds[i] = event_bdd_encodings.at(i).getNode();
    }
    FILE* fp = fopen("graph.dot", "w");
    Cudd_DumpDot(mgr.getManager(), num_events_after_splitting, Dds, (char**) inames, (char**) onames, fp);

    fclose(fp);
    //cout << "ok" << endl;



    //code for clause creation and dump on file
    string inFile = "BDD_clauses.txt";

    FILE* fpb = fopen("BDD_clauses.txt", "w");
    fclose(fpb);

    for(int i = 0;i < event_bdd_encodings.size();++i) {
        ofstream fon(inFile, std::ios_base::app);
        fon << "-----" << i <<"------" << endl;
        fon.close();
        fpb = fopen("BDD_clauses.txt", "a");
        event_bdd_encodings[i].PrintTwoLiteralClauses((char **) inames_without_r, fpb);
        fclose(fpb);
    }



    //reading elements from file:
    inFile = "BDD_clauses.txt";
    ifstream fin(inFile);
    //TODO: scrivere i file temporanei in memoria e non su disco
    string tmp;
    set<set<int>> *clause_set;
    clause_set = new set<set<int>>();
    set<int> tmp_set;
    vector<string> data;
    while(true){
        if(fin.eof()!=0)
            break;
        fin >> tmp;
        data.push_back(tmp);
        //cout << tmp << endl;
    }


    for(int i=0;i < data.size();++i){
        if(i == data.size()-1){
            tmp_set.insert(stoi(data[i]));
            clause_set->insert(tmp_set);
            //cout << "adding entire clause" << endl;
            break;
        }
        else if(i == 0){
            tmp_set.insert(stoi(data[i]));
            //cout << "adding" << endl;
            //cout << "adding " << data[i] << endl;
        }
        else {
            if(data[i] != "|"){
                if(data[i-1] != "|"){
                    clause_set->insert(tmp_set);
                    tmp_set.clear();
                    tmp_set.insert(stoi(data[i]));
                }
                else{
                    tmp_set.insert(stoi(data[i]));
                }
            }
            else{

            }
        }
    }

    /*
    for(auto s: *clause_set){
        println(s);
    }*/

    //set_of_EC_clauses = clause_set;

    for(auto val: inames_without_r){
        delete val;
    }
}

set<set<int>> *BDD_encoder::get_set_of_EC_clauses(map<Region *, int> *regions_alias_mapping) {
    auto tmp = new set<set<int>>();

    for(auto rec: *map_of_EC_clauses){
        for(auto clause: *rec.second){
            set<int> current_clause;
            for(auto reg: *clause){
                current_clause.insert(regions_alias_mapping->at(reg));
            }
            tmp->insert(current_clause);
        }
    }
    return tmp;
}
