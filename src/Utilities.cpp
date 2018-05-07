//
// Created by ciuchino on 18/04/18.
//

#include "../include/Utilities.h"

namespace Utilities {
    //Region = set<int> ->ritorna un insieme di stati
    set<int> *regions_union(vector<Region *> *vec){
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region: *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int> *regions_union(set<Region *> *vec){
        //cout << "region union" << endl;
        auto all_states = new Region();
        int size;
        Region::iterator it;
        for (Region *region: *vec) {
            it = region->begin();
            size = static_cast<int>(region->size());
            for (int i = 0; i < size; ++i) {
                //cout << "Stato: " << *it << endl;
                all_states->insert(*it);
                ++it;
            }
        }
        return all_states;
    }

    set<int> *regions_union(Region *first, Region *second){
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

    map<int, set<int> *> *do_regions_intersection(map<int, vector<Region> *> *regions){

        auto pre_regions_intersection = new map<int, set<int> *>;

        std::vector<Region>::iterator it;
        bool state_in_intersecton=true;

        //per ogni evento
        for (auto item: *regions) {
            (*pre_regions_intersection)[item.first]=new Region();
            for (auto state: (*item.second)[0]) {
                state_in_intersecton=true;
                for(auto set:*item.second){
                    if(set.find(state)==set.end()) {//non l'ho trovato
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
    }

    set<int> *regions_intersection(Region *first, Region *second){
        auto intersection = new set<int>();
        for (auto state: *first) {
            if (second->find(state) != second->end()) {//trovo lo stato (appartiene a entrambe)
                intersection->insert(state);
            }
        }
        return intersection;
    }

    void print(Region &region){
        Region::iterator it;
        int pos=0;
        int size=region.size();
        //for (auto state : region) {
        for(it=region.begin();it!=region.end();++it){
            auto state=*it;
            pos++;
            cout << state;
            if(pos!=size){
                cout<<",";
            }
        }
    }

    void println(Region &region){
        print(region);
        cout << endl;
    }

    void print_place(int pos,Region &region){
        cout<<"r"<<pos<<": { ";
        print(region);
        cout<<" } ";
    }

    void print_transactions(){
        cout<<"Transazioni: "<<endl;
        for(int i=0;i<num_transactions;i++){
            cout<<"t" <<i;
            if(i!=num_transactions-1)
                cout<<",";
        }

    }


    void print_PN(map<int, set<Region*>> * essential_regions,map<int, set<Region*>> * irredundant_regions) {
        int pos=0;

        cout<<"Posti: "<< endl;
        for(auto record:*essential_regions){
            for(auto reg: record.second) {
                pos++;
                print_place(pos,*reg);
            }
        }
        cout<<endl;

        cout<< "Posti ovuti a regioni non essenziali: " << endl;

	    for (auto record:*irredundant_regions) {
		    for (auto reg: record.second){
			    pos++;
			    print_place(pos, *reg);
		    }
	    }

        cout << endl;

	    //todo: nel caso in qui tutte le regioni sono essenziali da seg fault, seg-fault non dovrebbe essere legato al codice precedente
        print_transactions();

        //todo:finire PN con pre e post regioni (pre_essential_region & post_essential_region && pre/post_irredundant)
    }

    set<int> *region_difference(set<int> &first, set<int> &second) {
        auto s = new set<int>();
        for (auto state: first) {
            if (second.find(state) == second.end()) {
                s->insert(state);
            }
        }
        return s;
    }

    vector<Region> *copy_map_to_vector(map<int, vector<Region> *> *map) {
        auto input = new set<Region>();
        for (auto record: *map) {
            for (const auto &region: *record.second) {
                input->insert(region);
            }
        }
        vector<Region> *vec = new vector<Region>(input->size());
        std::copy(input->begin(), input->end(), vec->begin());
        delete input;
        return vec;
    }


    bool is_bigger_than(Region *region, set<int> *region2) {

        if (region->size() > region2->size()) {
            for (auto elem: *region2) {
                //nella regione non trovo un elem
                if (region->find(elem) == region->end()) {
                    cout << "****FALSE ";
                    print(*region),
                            cout << " is not bigger than ";
                    println(*region2);
                    return false;
                }
            }
        } else if (region->size() <= region2->size())
            return false;

        //nella regione trovo tutti gli stati della reg2
        cout << "****TRUE";
        print(*region),
                cout << " is bigger than ";
        println(*region2);
        return true;
    }


    bool are_equal(Region *region1, Region *region2) {


        if (region1->size() != region2->size())
            return false;

        for (auto elem: *region1) {
            if (region2->find(elem) == region2->end()) {
                return false;
            }
        }

        return true;

    }


    bool contains(set<Region *> *set, Region *region) {

        for(auto elem:*set){
            if(are_equal(elem, region) ){
                return true;
            }
        }

        return false;
    }

    bool is_equal_to(ER er,const set<int>* intersection){
        if(er->size()!=intersection->size()) {
            cout<<"if"<<endl;
            return false;
        }
        else {
            cout<<"else"<<endl;
            for (auto state_er : *er) {
                if(intersection->find(state_er) == intersection->end()) {   //non l'ho trovato
                    return false;
                }
            }
        }
        return true;
    }
}

