//
// Created by ciuchino on 13/04/18.
//

#include "Label_splitting_module.h"
#include "TS_parser.h"

Label_splitting_module::Label_splitting_module(map<int, vector<Region*> *>* pre_regions){
    this->pre_regions=pre_regions;
};

Label_splitting_module::~Label_splitting_module(){
};

bool Label_splitting_module::is_exitation_closed() {

    for(auto item: *pre_regions){
        for(auto item1: *item.second){
            cout << "prova" << item1 <<endl;
        }
    }

}