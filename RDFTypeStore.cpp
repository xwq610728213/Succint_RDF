//
// Created by weiqin xu on 28/08/2019.
//

#include "RDFTypeStore.hpp"
#include "Util.hpp"

RDFTypeStore::RDFTypeStore(){}

void RDFTypeStore::add(ID_TYPE s, ID_TYPE concept, bool force){
    if(force == true){
        unsigned long index = 0;
        for(auto i : concept2s[s2concept[s]]){
            if(i == s){
                concept2s[s2concept[s]].erase(concept2s[s2concept[s]].begin() + index);
                concept2s[concept].push_back(s);
                s2concept[s] = concept;
                return;
            }
            ++index;
        }
        s2concept[s] = concept;
        concept2s[concept].push_back(s);
    }else if(s2concept.count(s) == 0){
        s2concept[s] = concept;
        concept2s[concept].push_back(s);
    }else{
       std::cerr << "Triple already exists!" << endl;
    }
}

void RDFTypeStore::remove(ID_TYPE s, ID_TYPE concept){
    std::cerr << "Not implemented yet!" << endl;
}

bool RDFTypeStore::contains(ID_TYPE s, ID_TYPE concept){
    if(s2concept.count(s) == 0){
        return false;
    }else if(s2concept[s] != concept){
        cerr << "Instance belongs to another concept, please input the most precise concept!" << endl;
        throw RDFTypeException("Instance belongs to another concept");
    }else{
        return true;
    }
}

vector<ID_TYPE> RDFTypeStore::look_up_concept(ID_TYPE concept) {
    if(concept2s.count(concept) == 0){
        throw RDFTypeException("Research concept doesn't have registrations!");
    }else{
        return concept2s[concept];
    }
}

ID_TYPE RDFTypeStore::look_up_subject(ID_TYPE subject) {
    if(s2concept.count(subject) == 0){
        throw RDFTypeException("Research subject doesn't match any concept!");
    }else{
        return s2concept[subject];
    }
}
