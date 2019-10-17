//
// Created by weiqin xu on 26/08/2019.
//

#include "RDFStorage.hpp"
#include "RankBaseElement.hpp"
#include <sstream>
#include "Util.hpp"
#include "Constant.hpp"
#include "quickSortTS.hpp"



using namespace std;

RDFStorage::RDFStorage(){}

RDFStorage::RDFStorage(ifstream& abox_file, TripleStoreType ts_type){
    load_facts(abox_file, ts_type);
}

void RDFStorage::load_facts(ifstream& abox_file, TripleStoreType ts_type) {

    //to comment while using rdftype store
    dict_properties.add("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>", 0);

    ID_TYPE last_instance_id = 0;
    ID_TYPE last_property_id = 0;
    ID_TYPE last_concept_id = 0;

    vector<ID_TYPE> s_vector;
    vector<ID_TYPE> p_vector;
    vector<ID_TYPE> o_vector;

    string str;
    vector<RankBaseElement<3>> triple_vector;

    // For each fact, if instance doesn't exist, distribute an id_instance
    //                if concept doesn't exist, remind to check the TBOX
    //                if property doesn't exist, distribute an id_property
    auto start = clock();
    while(getline(abox_file, str, '\n')){
        //stringstream input(str);
        string subject, predicate, object;
        tie(subject, predicate, object) = triple_string_parser(str);
        ID_TYPE id_s, id_p, id_o;

        // Update dictionary, search id_s
        try {
            id_s = dict_instances.look_up_instance(subject);
        }catch(DictException& excep){
            ++last_instance_id;
            dict_instances.add(subject, last_instance_id);
            id_s = last_instance_id;
        }

        // Can be optimized with LiteMat, search id_p
        if(predicate != RDF_TYPE) {
            try {
                id_p = dict_properties.look_up_instance(predicate);
            } catch (DictException &excep) {
                ++last_property_id;
                dict_properties.add(predicate, last_property_id);
                id_p = last_property_id;
            }
        }


        // If predicate == rdf:type, object is a concept, otherwise object is an instance. search id_o
        if(true /*predicate != RDF_TYPE*/){
            try {
                id_o = dict_instances.look_up_instance(object);
            }catch(DictException &excep){
                ++last_instance_id;
                dict_instances.add(object, last_instance_id);
                id_o = last_instance_id;
            }
            // In order of PSO or POS
            ID_TYPE tmp[3];
            if(ts_type == PSO){
                tmp[0] = id_p;
                tmp[1] = id_s;
                tmp[2] = id_o;
            }else if(ts_type == POS){
                tmp[0] = id_p;
                tmp[1] = id_o;
                tmp[2] = id_s;
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }


            //cout << "new triple: " << id_s << " " << id_p << " " << id_o << endl;
            triple_vector.push_back(RankBaseElement<3>(tmp));
        }else{ //Will be optimized with LiteMat
            try {
                id_o = dict_concepts.look_up_instance(object);
                try {
                    if (!type_store.contains(id_s, id_o)) {
                        type_store.add(id_s, id_o);
                    }
                }catch(RDFTypeException& except){
                    cout << "Is " << object << " the most precise concept of " << subject << "? Y/N" << endl;
                    char answer;
                    cin >> answer;
                    while(answer != 'Y' && answer != 'N' && answer != 'y' && answer != 'n'){
                        cout << "Please enter Y/N!" << endl;
                        cin >> answer;
                    }
                    if(answer == 'Y' || answer == 'y'){
                        type_store.add(id_s, id_o, true);
                    }
                }
            }catch(DictException& excep){
                // Will be optimized with LiteMat
                //cerr << "Concept doesn't exist! Check the TBOX!" << endl;
                ++last_concept_id;
                dict_concepts.add(object, last_concept_id);
                id_o = last_concept_id;
                type_store.add(id_s, id_o);
            }
        }
    }
    cout << "Parse each line to creat dictionary and store triples in a vector time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    cout << "Last instance: " << last_instance_id << endl;
    cout << "Last proprety: " << last_property_id << endl;

    if(triple_vector.size() <= 0){
        cerr << "Triple vector size is 0!\n";
        exit(1);
    }
    start = clock();
    quickSortTS(triple_vector, 0, triple_vector.size() - 1);
    cout << "Qsort vector of triples time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;



    // Creat wt structure storage, In order of PSO or POS
    start = clock();
    if(ts_type == PSO){
        auto innerstart = clock();
        for(unsigned long i = 0; i < triple_vector.size(); ++i){
            p_vector.push_back(triple_vector[i][0]);
            s_vector.push_back(triple_vector[i][1]);
            o_vector.push_back(triple_vector[i][2]);
        }
        cout << "   -Push each line into P,S,O vectors: " << (double)(clock() - innerstart)/CLOCKS_PER_SEC*1000 << endl;
        innerstart =clock();
        triple_store = TripleStore(p_vector, s_vector, o_vector, ts_type);
        cout << "  | Creat wt structure with separate P,S,O vectors: " << (double)(clock() - innerstart)/CLOCKS_PER_SEC*1000 << endl;
    }else if(ts_type == POS){
        for(unsigned long i = 0; i < triple_vector.size(); ++i){
            p_vector.push_back(triple_vector[i][0]);
            o_vector.push_back(triple_vector[i][1]);
            s_vector.push_back(triple_vector[i][2]);
        }
        triple_store = TripleStore(p_vector, o_vector, s_vector, ts_type);
    }else{
        cerr << "Triple store type not defined!" << endl;
        exit(1);
    }
    cout << "Creat triple store time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
}

tuple<vector<string>, vector<vector<ID_TYPE>>> RDFStorage::query_triple_pattern(string s, string p, string o) {
    vector<vector<ID_TYPE>> results;
    vector<string> variables;
    if(true /*p != RDF_TYPE*/) { //Optimized with LiteMate

        if (s[0] == '?' && p[0] == '?' && o[0] == '?') {
            results = triple_store.look_up_ele012();

            //cout << "s" << " " << "p" << " " << "o" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id(i[1]) << " "
                         << dict_properties.look_up_id(i[0])
                         << " " << dict_instances.look_up_id(i[2]) << "\n";
                }*/
                variables.push_back(p);
                variables.push_back(s);
                variables.push_back(o);
            }else if(triple_store.get_ts_type() == POS){
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id(i[2]) << " "
                         << dict_properties.look_up_id(i[0])
                         << " " << dict_instances.look_up_id(i[1]) << "\n";
                }*/
                variables.push_back(p);
                variables.push_back(o);
                variables.push_back(s);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }

            //cout << '\n' << "\n";
        } else if (s[0] == '?' && p[0] == '?') {
            // In order of P S O
            //cout << "s" << " " << "p" << "\n";


            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele01(dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[1] ) << " "
                         << dict_properties.look_up_id( i[0] )
                         << "\n";
                }*/
                variables.push_back(p);
                variables.push_back(s);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele02(dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[1] ) << " "
                         << dict_properties.look_up_id( i[0] )
                         << "\n";
                }*/
                variables.push_back(p);
                variables.push_back(s);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (s[0] == '?' && o[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "s" << " " << "o" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele12(dict_properties.look_up_instance(p));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << " "
                         << dict_instances.look_up_id( i[1] )
                         << "\n";
                }*/
                variables.push_back(s);
                variables.push_back(o);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele12(dict_properties.look_up_instance(p));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[1] ) << " "
                         << dict_instances.look_up_id( i[0] )
                         << "\n";
                }*/
                variables.push_back(o);
                variables.push_back(s);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (p[0] == '?' && o[0] == '?') {
            // In order of P S O
            //cout << "p" << " " << "o" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele02(dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << " "
                         << dict_instances.look_up_id( i[1] )
                         << "\n";
                }*/
                variables.push_back(p);
                variables.push_back(o);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele01(dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << " "
                         << dict_instances.look_up_id( i[1] )
                         << "\n";
                }*/
                variables.push_back(p);
                variables.push_back(o);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (p[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "p" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele0(dict_instances.look_up_instance(s),
                                                        dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }/*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << endl;
                }*/
                variables.push_back(p);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele0(dict_instances.look_up_instance(o),
                                                        dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_properties.look_up_id( i[0] ) << endl;
                }*/
                variables.push_back(p);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (s[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "s" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele1(dict_properties.look_up_instance(p), dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                variables.push_back(s);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele2(dict_properties.look_up_instance(p), dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }/*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                variables.push_back(s);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else if (o[0] == '?') {
            // In order of P S O
            // Consider rdf:type
            //cout << "o" << "\n";
            if(triple_store.get_ts_type() == PSO) {
                try {
                    results = triple_store.look_up_ele2(dict_properties.look_up_instance(p), dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                variables.push_back(o);
            }else if(triple_store.get_ts_type() == POS){
                try {
                    results = triple_store.look_up_ele1(dict_properties.look_up_instance(p), dict_instances.look_up_instance(s));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
                /*
                for (auto i:results) {
                    cout << dict_instances.look_up_id( i[0] ) << endl;
                }*/
                variables.push_back(o);
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
            //cout << '\n' << "\n";
        } else {
            if(triple_store.get_ts_type() == PSO) {
                try{
                    results = triple_store.verify_triple(dict_properties.look_up_instance(p), dict_instances.look_up_instance(s), dict_instances.look_up_instance(o));
                } catch (DictException &except) {
                    return make_tuple(variables, results);
                }
            }else if(triple_store.get_ts_type() == POS){
                try{
                    results = triple_store.verify_triple(dict_properties.look_up_instance(p), dict_instances.look_up_instance(o), dict_instances.look_up_instance(s));
                }catch (DictException &except) {
                    return make_tuple(variables, results);
                }
            }else{
                cerr << "Triple store type not defined!" << endl;
                exit(1);
            }
        }
        return make_tuple(variables, results);
    }else{
        cerr << "Reasoner doesn't exist!" << endl;
        return make_tuple(variables, results);
    }
}

int RDFStorage::selectivity(vector<string> &t) {
    if(t[0][0] == '?' && t[1][0] == '?' && t[2][0] == '?')
        return 8;
    else if(t[0][0] == '?' && t[2][0] == '?')
        return 7;
    else if(t[1][0] == '?' && t[2][0] == '?')
        return 6;
    else if(t[0][0] == '?' && t[1][0] == '?')
        return 5;
    else if(t[2][0] == '?')
        return 4;
    else if(t[0][0] == '?')
        return 3;
    else if(t[1][0] == '?')
        return 2;
    else
        return 1;
}

unsigned long long RDFStorage::num_predicate_triples(string predicate){
    try{
        return triple_store.look_up_ele0_num(dict_properties.look_up_instance(predicate));
    }catch(DictException& excep){
        return 0;
    }
}

bool RDFStorage::more_selective(vector<string>& first, vector<string>& second){
    if(selectivity(first) < selectivity(second)){
        return true;
    }else if(selectivity(first) > selectivity(second)){
        return false;
    }else if(first[1][0] != '?' && second[1][0] != '?'){
        if(num_predicate_triples(first[1]) < num_predicate_triples(second[1]))
            return true;
        else
            return false;
    }else{
        return false;
    }
}

vector<string> RDFStorage::find_first_triple_pattern(vector<vector<string>>& query_table){
    vector<string> first_tp = query_table[0];
    unsigned long id = 0;
    for(unsigned long i = 1; i < query_table.size(); ++i){
        if(!more_selective(first_tp, query_table[i])){
            first_tp = query_table[i];
            id = i;
        }
    }
    query_table.erase(query_table.begin() + id);
    return first_tp;
}

bool RDFStorage::has_binding(vector<string>& variables, vector<string>& tp){
    for(auto i:tp)
        for(auto j:variables){
            if(i == j)
                return true;
        }
    return false;
}

vector<string> RDFStorage::find_next_triple_pattern(vector<string>& variables, vector<vector<string>>& query_table){
    vector<string> next_tp = query_table[0];
    unsigned long id = 0;
    for(unsigned long i = 1; i < query_table.size(); ++i){
        if(has_binding(variables, query_table[i]) && !more_selective(next_tp, query_table[i])){
            next_tp = query_table[i];
            id = i;
        }
    }

    cout << "Number of triples in dataset with this predicate: " << num_predicate_triples(next_tp[1]) << endl;

    query_table.erase(query_table.begin() + id);

    return next_tp;
}

tuple<vector<string>, vector<vector<ID_TYPE>>> RDFStorage::join(vector<string>& variables, vector<vector<ID_TYPE>>& table_a, const vector<string> triple_pattern){
    vector<string> new_variables = variables;
    vector<vector<ID_TYPE>> new_results_table;
    vector<vector<unsigned long>> binding_table;
    vector<unsigned long> tp_new_mapping;


    for(unsigned long index_triple_pattern = 0; index_triple_pattern < triple_pattern.size(); ++index_triple_pattern){
        if(triple_pattern[index_triple_pattern][0] == '?'){
            bool binding_trigger = false;
            for(unsigned long index_variables = 0; index_variables < variables.size(); ++index_variables){
                if(triple_pattern[index_triple_pattern] == variables[index_variables]){
                    vector<unsigned long> tmp;
                    tmp.push_back(index_triple_pattern);
                    tmp.push_back(index_variables);
                    binding_table.push_back(tmp);
                    binding_trigger = true;
                    break;
                }
            }
            if(!binding_trigger){
                new_variables.push_back(triple_pattern[index_triple_pattern]);
                tp_new_mapping.push_back(index_triple_pattern);
            }
        }
    }

    for(auto line: table_a){
        vector<string> tmp_query_triple = triple_pattern;
        vector<string> tmp_variables;
        vector<vector<ID_TYPE>> tmp_table;

        for(auto replace_index: binding_table){
            if(replace_index[0] != 1){
                try{
                    tmp_query_triple[replace_index[0]] = dict_instances.look_up_id(line[replace_index[1]]);
                } catch (DictException &except) {
                    cout <<"Instance " << line[replace_index[1]] << " not found" << endl;
                    vector<vector<ID_TYPE>> empty_results;
                    return make_tuple(variables, empty_results);
                }
            }else{
                try{
                    tmp_query_triple[replace_index[0]] = dict_properties.look_up_id(line[replace_index[1]]);
                } catch (DictException &except) {
                    cout <<"Property " << line[replace_index[1]] << " not found " << endl;
                    vector<vector<ID_TYPE>> empty_results;
                    return make_tuple(variables, empty_results);
                }
            }
        }
        tie(tmp_variables, tmp_table) = this -> query_triple_pattern(tmp_query_triple[0], tmp_query_triple[1], tmp_query_triple[2]);

        // If SPO are all constants, query_triple_pattern only verify the triple in tripleStore, in which case, tmp_variables.size() == 0 && tmp_table.size() > 0
        if(tmp_variables.size() == 0 && tmp_table.size() > 0){
            for(auto result_line: tmp_table){
                new_results_table.push_back(line);
            }
        }else{
            for(auto result_line: tmp_table){
                vector<ID_TYPE> new_line = line;
                for(auto mapping_ele: result_line){
                    new_line.push_back(mapping_ele);
                }
                new_results_table.push_back(new_line);
            }
        }

    }

    return make_tuple(new_variables, new_results_table);
}

bool RDFStorage::merge_join_condition(const vector<string> &variables, const vector<string> &triple_pattern) {
    // number of variables are included in this join
    int num_join_var = 0;
    for(auto i:triple_pattern){
        for(auto j:variables){
            if(i == j){
                ++num_join_var;
                break;
            }
        }
    }
    if(num_join_var == 1){
        if(triple_store.get_ts_type() == PSO){
            if(triple_pattern[0] == variables[0] && triple_pattern[1][0] != '?'){
                return true;
            }else if(triple_pattern[2] == variables[0] && triple_pattern[1][0] != '?' && triple_pattern[0][0] != '?'){
                return true;
            }else{
                return false;
            }
        }else if(triple_store.get_ts_type() == POS){
            if(triple_pattern[2] == variables[0] && triple_pattern[1][0] != '?'){
                return true;
            }else if(triple_pattern[0] == variables[0] && triple_pattern[1][0] != '?' && triple_pattern[2][0] != '?'){
                return true;
            }else{
                return false;
            }
        }else{
            cerr << "Triple store type not defined!" << endl;
            exit(1);
        }

    }else{
        return false;
    }
}

tuple<vector<string>, vector<vector<ID_TYPE>>> RDFStorage::query_graph_pattern(vector<string> triple_string_vector) {
    vector<vector<string>> query_table;
    for(string triple_pattern:triple_string_vector){
        //stringstream input(triple_pattern);
        string query_subject, query_predicate, query_object;
        tie(query_subject, query_predicate, query_object) = triple_string_parser(triple_pattern);
        vector<string> tmp;
        tmp.push_back(query_subject);
        tmp.push_back(query_predicate);
        tmp.push_back(query_object);
        query_table.push_back(tmp);
    }


    vector<string> variables;
    vector<vector<ID_TYPE>> results_table;

    vector<string> current_triple_pattern = find_first_triple_pattern(query_table);

    cout << current_triple_pattern[0] << " " << current_triple_pattern[1] << " " << current_triple_pattern[2] << endl;

    auto start = clock();
    tie(variables, results_table) = this -> query_triple_pattern(current_triple_pattern[0], current_triple_pattern[1], current_triple_pattern[2]);
    cout << "time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    cout << "number of results: " << results_table.size() << endl;

    while(query_table.size() > 0 && results_table.size() > 0){
        current_triple_pattern = find_next_triple_pattern(variables, query_table);

        cout << current_triple_pattern[0] << " " << current_triple_pattern[1] << " " << current_triple_pattern[2] << endl;
        // The first column is always in order, we can profit from merge_join
        auto start = clock();
        if(merge_join_condition(variables, current_triple_pattern)){
            tie(variables, results_table) = merge_join(variables, results_table, current_triple_pattern);
        }else{
            tie(variables, results_table) = join(variables, results_table, current_triple_pattern);
        }
        cout << "time: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
        cout << "number of results: " << results_table.size() << endl;

    }
    /*
    JoinResults res;
    // Naive join order
    for(int i = 0; i < query_table.size(); ++i){
        vector<vector<ID_TYPE>> tmp_table;
        vector<string> tmp_varibales;
        clock_t start, end;
        start = clock();
        tie(tmp_varibales, tmp_table) = this -> query_triple_pattern(query_table[i][0], query_table[i][1], query_table[i][2]);
        end = clock();
        cout << "Triple patter search time: " << (double)(end - start)/CLOCKS_PER_SEC*1000 << " ms" << endl;
        if(i == 0){
            res = JoinResults(tmp_table, tmp_varibales);
        }else{
            res.join(tmp_table, tmp_varibales);
        }
    }*/

    return make_tuple(variables, results_table);
}

tuple<vector<string>, vector<vector<ID_TYPE>>> RDFStorage::merge_join(vector<string> &variables,
                                                                      vector<vector<ID_TYPE>> &table_a,
                                                                      const vector<string> triple_pattern) {
    vector<string> new_variables = variables;
    vector<vector<ID_TYPE>> new_results_table;

    if(triple_store.get_ts_type() == PSO){
        if(variables[0] == triple_pattern[0]){
            unsigned long long index0, index1_begin = 1, index1_end = 0;
            if(triple_pattern[2][0] != '?'){
                for(auto line: table_a){
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[0] = dict_instances.look_up_id(line[0]);
                    if(triple_store.verify_triple_merge_pos1(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[0]), dict_instances.look_up_instance(tmp_query_triple[2]), index0, index1_begin, index1_end)){
                        new_results_table.push_back(line);
                        if(index1_begin == index1_end){
                            break;
                        }
                    }

                }

            }else{
                new_variables.push_back(triple_pattern[2]);
                for (auto line: table_a) {
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[0] = dict_instances.look_up_id(line[0]);
                    vector<vector<ID_TYPE>> triple_results = triple_store.look_up_ele2_merge(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[0]), index0, index1_begin, index1_end);

                    for(auto res_line: triple_results){
                        vector<ID_TYPE> new_line = line;
                        for(auto ele: res_line){
                            new_line.push_back(ele);
                        }
                        new_results_table.push_back(new_line);
                    }
                }
            }

        }else if(variables[0] == triple_pattern[2]){
            unsigned long long index0, index1, index2_begin = 1, index2_end = 0;
            for(auto line: table_a){
                vector<string> tmp_query_triple = triple_pattern;
                tmp_query_triple[2] = dict_instances.look_up_id(line[0]);
                if(triple_store.verify_triple_merge_pos2(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[0]), dict_instances.look_up_instance(tmp_query_triple[2]), index0, index1, index2_begin, index2_end)){
                    new_results_table.push_back(line);
                    if(index2_begin == index2_end){
                        break;
                    }
                }

            }
        }
    }else if(triple_store.get_ts_type() == POS){
        if(variables[0] == triple_pattern[2]){

            unsigned long long index0, index1_begin = 1, index1_end = 0;
            if(triple_pattern[0][0] != '?'){
                for(auto line: table_a){
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[2] = dict_instances.look_up_id(line[0]);
                    if(triple_store.verify_triple_merge_pos1(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[2]), dict_instances.look_up_instance(tmp_query_triple[0]), index0, index1_begin, index1_end)){
                        new_results_table.push_back(line);
                        if(index1_begin == index1_end){
                            break;
                        }
                    }

                }
            }else{
                new_variables.push_back(triple_pattern[0]);
                for (auto line: table_a) {
                    vector<string> tmp_query_triple = triple_pattern;
                    tmp_query_triple[2] = dict_instances.look_up_id(line[0]);
                    vector<vector<ID_TYPE>> triple_results = triple_store.look_up_ele2_merge(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[2]), index0, index1_begin, index1_end);

                    for(auto res_line: triple_results){
                        vector<ID_TYPE> new_line = line;
                        for(auto ele: res_line){
                            new_line.push_back(ele);
                        }
                        new_results_table.push_back(new_line);
                    }
                }
            }

        }else if(variables[0] == triple_pattern[0]){
            unsigned long long index0, index1, index2_begin = 1, index2_end = 0;
            for(auto line: table_a){
                vector<string> tmp_query_triple = triple_pattern;
                tmp_query_triple[0] = dict_instances.look_up_id(line[0]);
                if(triple_store.verify_triple_merge_pos2(dict_properties.look_up_instance(tmp_query_triple[1]), dict_instances.look_up_instance(tmp_query_triple[2]), dict_instances.look_up_instance(tmp_query_triple[0]), index0, index1, index2_begin, index2_end)){
                    new_results_table.push_back(line);
                    if(index2_begin == index2_end){
                        break;
                    }
                }

            }
        }
    }else{
        cerr << "Triple store type not defined!" << endl;
        exit(1);
    }
    return make_tuple(new_variables, new_results_table);
}

void RDFStorage::store_to_disk(const string &path) {
    triple_store.store_to_disk(path);
    dict_instances.store_to_disk(path, "dict_instances");
    dict_properties.store_to_disk(path, "dict_properties");
    dict_concepts.store_to_disk(path, "dict_concepts");
}

void RDFStorage::back_up_from_disk(const string &path){
    triple_store.back_up_from_disk(path);
}

tuple<string,string,string> RDFStorage::triple_string_parser(string &str) {
    unsigned long index_begin = 0, triple_index = 0;
    string triple[3];
    char last_sym = '0';
    for(unsigned long i = 0; i < str.size(); ++i){
        if((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')){
            continue;
        }else if(str[i] == '?' && last_sym == '0'){
            index_begin = i;
            last_sym = '?';
        }else if(str[i] == '<' && last_sym == '0'){
            index_begin = i;
            last_sym = '<';
        }else if(str[i] == '\"' && last_sym != '\"'){
            index_begin = i;
            last_sym = '\"';
        }else if(last_sym != '0' && str[i] >= '0' && str[i] <= '9'){
            index_begin = i;
            last_sym = '1';
        }else if(str[i] == '>' && last_sym == '<'){
            triple[triple_index] = str.substr(index_begin, i - index_begin + 1);
            ++triple_index;
            last_sym = '0';
        }else if(str[i] == '\"' && last_sym == '\"'){
            // in case of "qs dfza"^^xsd:string
            last_sym = 'e';
        }else if(str[i] == ' ' && (last_sym == 'e' || last_sym == '?' || last_sym == '1')){
            triple[triple_index] = str.substr(index_begin, i - index_begin);
            ++triple_index;
            last_sym = '0';
        }
        if(triple_index >=3)
            break;
    }
    return make_tuple(triple[0], triple[1], triple[2]);
}