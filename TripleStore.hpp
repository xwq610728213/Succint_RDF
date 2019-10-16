//
// Created by weiqin xu on 2019-08-23.
//

#ifndef TRIPLE_STORE_TRIPLESTORE_HPP
#define TRIPLE_STORE_TRIPLESTORE_HPP

#include <sdsl/wavelet_trees.hpp>
#include <string>
#include "Constant.hpp"

using namespace sdsl;
using namespace std;
typedef wt_int<rrr_vector<63>> WT_TYPE;

class TripleStore {
private:
    TripleStoreType triple_store_type = UNKNOWN;
    WT_TYPE wt_ele0;
    vector<unsigned long long> wt_ele1_bit_map;
    WT_TYPE wt_ele1;
    vector<unsigned long long> wt_ele2_bit_map;
    WT_TYPE wt_ele2;

    unsigned long long cal_index2_begin(unsigned long long index1);
    unsigned long long cal_index2_end(unsigned long long index1);
    unsigned long long cal_index1_begin(unsigned long long index0);
    unsigned long long cal_index1_end(unsigned long long index0);
    unsigned long long rank_vector(vector<unsigned long long> &vec, unsigned long long num);
    //void construct_bit_map(vector<unsigned long long> &vec, string str);

public:
    TripleStore();

    TripleStore(std::vector<ID_TYPE>&  ele0_vector, std::vector<ID_TYPE>& ele1_vector, std::vector<ID_TYPE>&  ele2_vector, TripleStoreType ts_type);

    // ts_type: PSO or POS
    TripleStoreType get_ts_type();
    void store_to_disk(const string &path);
    void back_up_from_disk(const string &path);

    void load(std::vector<ID_TYPE>&  ele0_vector, std::vector<ID_TYPE>& ele1_vector, std::vector<ID_TYPE>&  ele2_vector, TripleStoreType ts_type);
    std::vector<vector<ID_TYPE>> look_up_ele0(ID_TYPE ele1, ID_TYPE ele2);
    unsigned long long look_up_ele0_num(ID_TYPE ele0);
    std::vector<vector<ID_TYPE>> look_up_ele1(ID_TYPE ele0, ID_TYPE ele2);
    std::vector<vector<ID_TYPE>> look_up_ele2(ID_TYPE ele0, ID_TYPE ele1);
    std::vector<vector<ID_TYPE>> look_up_ele01(ID_TYPE ele2);
    std::vector<vector<ID_TYPE>> look_up_ele02(ID_TYPE ele1);
    std::vector<vector<ID_TYPE>> look_up_ele12(ID_TYPE ele0);
    std::vector<vector<ID_TYPE>> look_up_ele012();
    std::vector<vector<ID_TYPE>> verify_triple(ID_TYPE ele0, ID_TYPE ele1, ID_TYPE ele2);
    bool verify_triple_merge_pos2(ID_TYPE ele0, ID_TYPE ele1, ID_TYPE ele2, unsigned long long &index0, unsigned long long &index1, unsigned long long &index2_begin, unsigned long long &index2_end);
    bool verify_triple_merge_pos1(ID_TYPE ele0, ID_TYPE ele1, ID_TYPE ele2, unsigned long long &index0, unsigned long long &index1_begin, unsigned long long &index1_end);
    std::vector<vector<ID_TYPE>> look_up_ele2_merge(ID_TYPE ele0, ID_TYPE ele1, unsigned long long &index0, unsigned long long &index1_begin, unsigned long long &index1_end);


};


#endif //TRIPLE_STORE_TRIPLESTORE_HPP
