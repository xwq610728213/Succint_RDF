//
// Created by weiqin xu on 2019-08-23.
//

#include "TripleStore.hpp"

TripleStore::TripleStore(){}

TripleStore::TripleStore(std::vector<ID_TYPE>& ele0_vector, std::vector<ID_TYPE>& ele1_vector, std::vector<ID_TYPE>& ele2_vecto, TripleStoreType ts_type){
    load(ele0_vector, ele1_vector, ele2_vecto, ts_type);
}


// ele0 ele1 ele2 have been sorted
void TripleStore::load(std::vector<ID_TYPE>& ele0_vector, std::vector<ID_TYPE>& ele1_vector, std::vector<ID_TYPE>& ele2_vector, TripleStoreType ts_type) {
    triple_store_type = ts_type;
    wt_ele1_bit_map.clear();
    wt_ele2_bit_map.clear();

    ID_TYPE last_ele0 = -1, last_ele1 = -1;
    string ele0_str, ele1_str, ele1_bit_map_str, ele2_str, ele2_bit_map_str;
    unsigned long long index0 = 0, index1 = 0, index2 = 0;


    // Construct the string of ele0, ele1, ele2 and bit_maps
    auto start = clock();
    for(unsigned long long i = 0; i < ele0_vector.size(); ++i){

        ele2_str += to_string(ele2_vector[i]);
        ele2_str += " ";

        if(ele1_vector[i] != last_ele1){
            last_ele1 = ele1_vector[i];
            ele1_str += to_string(ele1_vector[i]);
            ele1_str += " ";
            /*
            ele2_bit_map_str += to_string(1);
            ele2_bit_map_str += " ";
            */
            wt_ele2_bit_map.push_back(index2);


            if(ele0_vector[i] != last_ele0){
                last_ele0 = ele0_vector[i];
                ele0_str += to_string(ele0_vector[i]);
                ele0_str += " ";
                /*
                ele1_bit_map_str += to_string(1);
                ele1_bit_map_str += " ";
                */
                wt_ele1_bit_map.push_back(index1);
                ++index0;
            }
            else{
                /*
                ele1_bit_map_str += to_string(0);
                ele1_bit_map_str += " ";
                */
            }
            ++index1;
        }
        else{
            /*
            ele2_bit_map_str += to_string(0);
            ele2_bit_map_str += " ";
            */
        }
        ++index2;
    }
    cout << "  |    -Construct the string of ele0, ele1, ele2 and bit_maps: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;

    start = clock();
    construct_im(wt_ele0, ele0_str, 'd');
    cout << "  |   | Call construct_im to create ele0 wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    //construct_im(wt_ele1_bit_map, ele1_bit_map_str, 'd');
    //construct_bit_map(wt_ele1_bit_map, ele1_bit_map_str);
    cout << "  |   | Call construct_im to create ele01 bit map wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    construct_im(wt_ele1, ele1_str, 'd');
    cout << "  |   | Call construct_im to create ele1 wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    //construct_im(wt_ele2_bit_map, ele2_bit_map_str, 'd');
    //construct_bit_map(wt_ele1_bit_map, ele1_bit_map_str);
    cout << "  |   | Call construct_im to create ele12 bit map wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;
    start = clock();
    construct_im(wt_ele2, ele2_str, 'd');
    cout << "  |   | Call construct_im to create ele2 wt: " << (double)(clock() - start)/CLOCKS_PER_SEC*1000 << endl;

    //cout << ele0_str << "\n" << ele1_bit_map_str << "\n" << ele1_str << "\n" << ele2_bit_map_str << "\n" << ele2_str << "\n" << "\n" << "\n" ;

}


// For triple pattern: ?ele0 ele1 ele2
std::vector<vector<ID_TYPE>> TripleStore::look_up_ele0(ID_TYPE ele1, ID_TYPE ele2){
    std::vector<vector<ID_TYPE>> results;
    for(unsigned long long i = 0; i < wt_ele1.rank(wt_ele1.size(), ele1); ++i){

        unsigned long long index1 = wt_ele1.select(i + 1, ele1);
        unsigned long long index2_begin = cal_index2_begin(index1);
        unsigned long long index2_end = cal_index2_end(index1);

        if(wt_ele2.range_search_2d(index2_begin, index2_end - 1, ele2, ele2).first == 0){
            continue;
        }

        //unsigned long long index0 = wt_ele1_bit_map.rank(index1 + 1, 1) - 1;
        unsigned long long index0 = rank_vector(wt_ele1_bit_map, index1);

        vector<ID_TYPE> tmp_result;
        tmp_result.push_back(wt_ele0[index0]);

        results.push_back(tmp_result);

    }

    return results;
}

// For triple pattern: ele0 ?ele1 ele2
std::vector<vector<ID_TYPE>> TripleStore::look_up_ele1(ID_TYPE ele0, ID_TYPE ele2){
    std::vector<vector<ID_TYPE>> results;
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = cal_index1_begin(index0);
    unsigned long long index1_end = cal_index1_end(index0);

    //cout << index1_begin << " " << index1_end << "\n";
    unsigned long long index2_begin = cal_index2_begin(index1_begin);
    unsigned long long index2_end = cal_index2_end(index1_end - 1);

    //cout << index2_begin << " " << index2_end << "\n";
    auto temp = wt_ele2.range_search_2d(index2_begin, index2_end - 1, ele2, ele2);
    //cout << temp.first << endl;
    if(temp.first == 0){
        return results;
    }

    for(auto i:temp.second){
        //cout << i.first << " ";
        unsigned long long index1 = rank_vector(wt_ele2_bit_map, i.first);

        vector<ID_TYPE> tmp_result;
        tmp_result.push_back(wt_ele1[index1]);

        results.push_back(tmp_result);
    }

    return results;
}


// For triple pattern: ele0 ele1 ?ele2
std::vector<vector<ID_TYPE>> TripleStore::look_up_ele2(ID_TYPE ele0, ID_TYPE ele1){
    std::vector<vector<ID_TYPE>> results;
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }


    unsigned long long index1_begin = cal_index1_begin(index0);
    unsigned long long index1_end = cal_index1_end(index0);

    auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
    if(temp.first == 0){
        return results;
    }

    unsigned long long index1 = temp.second[0].first;

    unsigned long long index2_begin = cal_index2_begin(index1);
    unsigned long long index2_end = cal_index2_end(index1);

    for(unsigned long long i = index2_begin; i < index2_end; ++i){

        vector<ID_TYPE> tmp_result;
        tmp_result.push_back(wt_ele2[i]);

        results.push_back(tmp_result);
    }

    return results;
}

std::vector<vector<ID_TYPE>> TripleStore::look_up_ele01(ID_TYPE ele2) {
    std::vector<vector<ID_TYPE>> results;
    for(unsigned long long i = 0; i < wt_ele2.rank(wt_ele2.size(), ele2); ++i){
        unsigned long long index2 = wt_ele2.select(i + 1, ele2);
        unsigned long long index1 = rank_vector(wt_ele2_bit_map, index2);
        unsigned long long index0 = rank_vector(wt_ele1_bit_map, index1);

        vector<ID_TYPE> tmp_result;
        tmp_result.push_back(wt_ele0[index0]);
        tmp_result.push_back(wt_ele1[index1]);

        results.push_back(tmp_result);
    }

    return results;
}

std::vector<vector<ID_TYPE>> TripleStore::look_up_ele02(ID_TYPE ele1) {
    std::vector<vector<ID_TYPE>> results;
    for(unsigned long long i = 0; i < wt_ele1.rank(wt_ele1.size(), ele1); ++i){
        unsigned long long index1 = wt_ele1.select(i + 1, ele1);
        unsigned long long index0 = rank_vector(wt_ele1_bit_map, index1);
        unsigned long long index2_begin = cal_index2_begin(index1);
        unsigned long long index2_end = cal_index2_end(index1);

        for(unsigned long long index2 = index2_begin; index2 < index2_end; ++index2){
            vector<ID_TYPE> tmp_result;
            tmp_result.push_back(wt_ele0[index0]);
            tmp_result.push_back(wt_ele2[index2]);

            results.push_back(tmp_result);
        }
    }

    return results;
}

std::vector<vector<ID_TYPE>> TripleStore::look_up_ele12(ID_TYPE ele0) {
    std::vector<vector<ID_TYPE>> results;
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = cal_index1_begin(index0);
    unsigned long long index1_end = cal_index1_end(index0);

    for(unsigned long long index1 = index1_begin; index1 < index1_end; ++index1) {
        unsigned long long index2_begin = cal_index2_begin(index1);
        unsigned long long index2_end = cal_index2_end(index1);

        for (unsigned long long index2 = index2_begin; index2 < index2_end; ++index2) {

            vector<ID_TYPE> tmp_result;
            tmp_result.push_back(wt_ele1[index1]);
            tmp_result.push_back(wt_ele2[index2]);

            results.push_back(tmp_result);
        }
    }

    return results;
}


std::vector<vector<ID_TYPE>> TripleStore::look_up_ele012() {
    std::vector<vector<ID_TYPE>> results;
    for(unsigned long long index0 = 0; index0 < wt_ele0.size(); ++index0){
        unsigned long long index1_begin = cal_index1_begin(index0);
        unsigned long long index1_end = cal_index1_end(index0);

        for(unsigned long long index1 = index1_begin; index1 < index1_end; ++index1){
            unsigned long long index2_begin = cal_index2_begin(index1);
            unsigned long long index2_end = cal_index2_end(index1);

            for(unsigned long long index2 = index2_begin; index2 < index2_end; ++index2){
                vector<ID_TYPE> tmp_result;
                tmp_result.push_back(wt_ele0[index0]);
                tmp_result.push_back(wt_ele1[index1]);
                tmp_result.push_back(wt_ele2[index2]);

                results.push_back(tmp_result);
            }
        }
    }

    return results;
}

TripleStoreType TripleStore::get_ts_type() {
    return triple_store_type;
}

unsigned long long TripleStore::look_up_ele0_num(ID_TYPE ele0) {
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return 0;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = cal_index1_begin(index0);
    unsigned long long index1_end = cal_index1_end(index0);

    unsigned long long index2_begin = cal_index2_begin(index1_begin);
    unsigned long long index2_end = cal_index2_end(index1_end - 1);

    return (index2_end - index2_begin);

}

std::vector<vector<ID_TYPE>> TripleStore::verify_triple(ID_TYPE ele0, ID_TYPE ele1, ID_TYPE ele2) {
    std::vector<vector<ID_TYPE>> results;
    unsigned long long index0;
    if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
        return results;
    }else{
        index0 = wt_ele0.select(1,ele0);
    }

    unsigned long long index1_begin = cal_index1_begin(index0);
    unsigned long long index1_end = cal_index1_end(index0);

    auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
    if(temp.first == 0){
        return results;
    }

    unsigned long long index1 = temp.second[0].first;

    unsigned long long index2_begin = cal_index2_begin(index1);
    unsigned long long index2_end = cal_index2_end(index1);

    auto temp2 = wt_ele2.range_search_2d(index2_begin, index2_end - 1, ele2, ele2);
    if(temp2.first == 0){
        return results;
    }

    vector<ID_TYPE> tmp_result;
    tmp_result.push_back(ele0);
    tmp_result.push_back(ele1);
    tmp_result.push_back(ele2);
    results.push_back(tmp_result);
    return results;

}

bool TripleStore::verify_triple_merge_pos2(ID_TYPE ele0, ID_TYPE ele1, ID_TYPE ele2, unsigned long long &index0,
                                      unsigned long long &index1, unsigned long long &index2_begin,
                                      unsigned long long &index2_end) {
    if(index2_begin > index2_end){
        if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
            return false;
        }else{
            index0 = wt_ele0.select(1,ele0);
        }

        unsigned long long index1_begin = cal_index1_begin(index0);
        unsigned long long index1_end = cal_index1_end(index0);

        auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
        if(temp.first == 0){
            return false;
        }

        index1 = temp.second[0].first;

        index2_begin = cal_index2_begin(index1);
        index2_end = cal_index2_end(index1);

        auto temp2 = wt_ele2.range_search_2d(index2_begin, index2_end - 1, ele2, ele2);
        if(temp2.first == 0){
            return false;
        }

        index2_begin = temp2.second[0].first;

        return true;

    }else{
        auto temp2 = wt_ele2.range_search_2d(index2_begin, index2_end - 1, ele2, ele2);
        if(temp2.first == 0){
            return false;
        }

        index2_begin = temp2.second[0].first;
        return true;
    }
}

bool TripleStore::verify_triple_merge_pos1(ID_TYPE ele0, ID_TYPE ele1, ID_TYPE ele2, unsigned long long &index0,
                                           unsigned long long &index1_begin,
                                           unsigned long long &index1_end) {
    if(index1_begin > index1_end){
        if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
            return false;
        }else{
            index0 = wt_ele0.select(1,ele0);
        }

        index1_begin = cal_index1_begin(index0);
        index1_end = cal_index1_end(index0);

        auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
        if(temp.first == 0){
            return false;
        }

        unsigned long long index1 = temp.second[0].first;

        index1_begin = temp.second[0].first;

        unsigned long long index2_begin = cal_index2_begin(index1);
        unsigned long long index2_end = cal_index2_end(index1);

        auto temp2 = wt_ele2.range_search_2d(index2_begin, index2_end - 1, ele2, ele2);
        if(temp2.first == 0){
            return false;
        }

        return true;

    }else{
        auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
        if(temp.first == 0){
            return false;
        }

        unsigned long long index1 = temp.second[0].first;

        index1_begin = temp.second[0].first;

        unsigned long long index2_begin = cal_index2_begin(index1);
        unsigned long long index2_end = cal_index2_end(index1);

        auto temp2 = wt_ele2.range_search_2d(index2_begin, index2_end - 1, ele2, ele2);
        if(temp2.first == 0){
            return false;
        }

        return true;
    }
}

std::vector<vector<ID_TYPE>> TripleStore::look_up_ele2_merge(ID_TYPE ele0, ID_TYPE ele1, unsigned long long &index0,
                                                             unsigned long long &index1_begin,
                                                             unsigned long long &index1_end) {
    vector<vector<ID_TYPE>> res;
    if(index1_begin > index1_end){
        if(wt_ele0.rank(wt_ele0.size(), ele0) == 0){
            return res;
        }else{
            index0 = wt_ele0.select(1,ele0);
        }

        index1_begin = cal_index1_begin(index0);
        index1_end = cal_index1_end(index0);

        auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
        if(temp.first == 0){
            return res;
        }

        unsigned long long index1 = temp.second[0].first;

        index1_begin = temp.second[0].first;

        unsigned long long index2_begin = cal_index2_begin(index1);
        unsigned long long index2_end = cal_index2_end(index1);

        for(unsigned long long i = index2_begin; i < index2_end; ++i){

            vector<ID_TYPE> tmp_result;
            tmp_result.push_back(wt_ele2[i]);

            res.push_back(tmp_result);
        }

        return res;
    }else{
        auto temp = wt_ele1.range_search_2d(index1_begin, index1_end - 1, ele1, ele1);
        if(temp.first == 0){
            return res;
        }

        unsigned long long index1 = temp.second[0].first;

        index1_begin = temp.second[0].first;

        unsigned long long index2_begin = cal_index2_begin(index1);
        unsigned long long index2_end = cal_index2_end(index1);

        for(unsigned long long i = index2_begin; i < index2_end; ++i){

            vector<ID_TYPE> tmp_result;
            tmp_result.push_back(wt_ele2[i]);

            res.push_back(tmp_result);
        }

        return res;
    }
}

void TripleStore::store_to_disk(const string &path) {
    string directory_path;

    if(path[path.length() - 1] == '/'){
        directory_path = path;
    }else{
        directory_path = path + "/";
    }

    string ele0_path = directory_path + "ele0.st";
    string ele01_bitmap_path = directory_path + "ele01_bitmap.st";
    string ele1_path = directory_path + "ele1.st";
    string ele12_bitmap_path = directory_path + "ele12_bitmap.st";
    string ele2_path = directory_path + "ele2.st";
    sdsl::store_to_file(wt_ele0, ele0_path);
    //sdsl::store_to_file(wt_ele1_bit_map, ele01_bitmap_path);
    sdsl::store_to_file(wt_ele1, ele1_path);
    //sdsl::store_to_file(wt_ele1_bit_map, ele12_bitmap_path);
    sdsl::store_to_file(wt_ele2, ele2_path);
    ofstream base_type(directory_path + "base_type.st");
    base_type << triple_store_type;
    base_type.close();
}


void TripleStore::back_up_from_disk(const string &path) {
    string directory_path;

    if(path[path.length() - 1] == '/'){
        directory_path = path;
    }else{
        directory_path = path + "/";
    }

    string ele0_path = directory_path + "ele0.st";
    string ele01_bitmap_path = directory_path + "ele01_bitmap.st";
    string ele1_path = directory_path + "ele1.st";
    string ele12_bitmap_path = directory_path + "ele12_bitmap.st";
    string ele2_path = directory_path + "ele2.st";

    ifstream base_type(directory_path + "base_type.st");
    string type;
    base_type >> type;
    this->triple_store_type = (TripleStoreType)atoi(type.c_str());
    base_type.close();
}

unsigned long long TripleStore::cal_index2_begin(unsigned long long index1) {
    return wt_ele2_bit_map[index1];
}

unsigned long long TripleStore::cal_index2_end(unsigned long long index1) {
    return index1 >= (wt_ele2_bit_map.size() - 1)? wt_ele2.size(): wt_ele2_bit_map[index1 + 1];
}

unsigned long long TripleStore::cal_index1_begin(unsigned long long index0) {
    return wt_ele1_bit_map[index0];
}

unsigned long long TripleStore::cal_index1_end(unsigned long long index0) {
    return index0 >= (wt_ele1_bit_map.size() - 1)? wt_ele1.size(): wt_ele1_bit_map[index0 + 1];
}

unsigned long long TripleStore::rank_vector(vector<unsigned long long> &vec, unsigned long long num) {
    unsigned long long begin = 0, end = vec.size();
    //cout << begin << " " << end << " index: " << num << endl;
    if(vec[begin] == num)
        return begin;
    else if(vec[end - 1] == num)
        return end - 1;

    while(begin < end && end - begin > 1){
        unsigned long long index_mid = (end + begin)/2;
        if(vec[index_mid] == num)
            return index_mid;
        else if(vec[index_mid] > num)
            end = index_mid;
        else if(vec[index_mid] < num)
            begin = index_mid;
    }

    return begin;
}