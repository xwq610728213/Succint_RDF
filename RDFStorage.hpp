//
// Created by weiqin xu on 26/08/2019.
//

#ifndef TRIPLE_STORE_RDFSTORAGE_HPP
#define TRIPLE_STORE_RDFSTORAGE_HPP

#include "TripleStore.hpp"
#include "Dictionary.hpp"
#include "RDFTypeStore.hpp"
#include <iostream>
#include <fstream>
#include "Constant.hpp"

using namespace std;


class RDFStorage {
private:
    TripleStore triple_store;
    // Storage for: s rdf:type concept
    RDFTypeStore type_store;
    Dictionary dict_instances;
    Dictionary dict_properties;   // Can be optimized with LiteMat
    Dictionary dict_concepts;  // Can be optimized with LiteMat

    unsigned long long num_predicate_triples(string predicate);
    bool more_selective(vector<string>& first, vector<string>& second);
    vector<string> find_first_triple_pattern(vector<vector<string>>& query_table);
    vector<string> find_next_triple_pattern(vector<string>& variables, vector<vector<string>>& query_table);
    tuple<vector<string>, vector<vector<ID_TYPE>>> join(vector<string>& variables, vector<vector<ID_TYPE>>& table_a, const vector<string> triple_pattern);
    tuple<vector<string>, vector<vector<ID_TYPE>>> merge_join(vector<string>& variables, vector<vector<ID_TYPE>>& table_a, const vector<string> triple_pattern);
    bool has_binding(vector<string>& variables, vector<string>& tp);
    int selectivity(vector<string>& t);
    bool merge_join_condition(const vector<string>& variables, const vector<string>& triple_pattern);
public:
    RDFStorage();
    RDFStorage(ifstream& abox_file, TripleStoreType triple_store_type);
    void store_to_disk(const string &path);
    void back_up_from_disk(const string &path);
    void load_facts(ifstream& abox_file, TripleStoreType triple_store_type);
    tuple<vector<string>, vector<vector<ID_TYPE>>> query_triple_pattern(string s, string p, string o);
    tuple<vector<string>, vector<vector<ID_TYPE>>> query_graph_pattern(vector<string> triple_string_vector);
    string look_up_id_instance(ID_TYPE id){
        return dict_instances.look_up_id(id);
    }
};


#endif //TRIPLE_STORE_RDFSTORAGE_HPP
