#include <algorithm>
#include <random>
#include "gen.hh"
#include <boost/math/distributions/pareto.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <chrono>
#include <iostream>
#include <math.h>

//The constructor does everything, basically
Generator::Generator(int locality_range, double locality_shift, int size, double size_factor)
{
    locality_range_ = locality_range;
    locality_shift_ = locality_shift;
    current_locality_ = 0;
    total_prob_ = 0;
    int size_used = 0;
    int i = 0;
    //Now we set up the randomization, and the different distributions
    std::random_device rd;
    std::mt19937 rng{rd()};
    std::normal_distribution<double> key_dist(43.0, 9.0);//want a normal dist spanning 15-70, so use this and clamp later
    std::uniform_int_distribution<int> percent_dist(0, 100);
    std::normal_distribution<double> val_dist_low(2.0, 3.0);
    std::uniform_int_distribution<int> val_dist_mid(10, 500);
    std::normal_distribution<double> val_dist_high(0.0, 1500.0);
    while(size_used <= size_factor*size){//we want the cache full and still plenty of pairs left; pointless to test if cache can hold 100% of data
        //First generate a new data tuple based on our distributions
        int key_size = std::clamp((int)key_dist(rng), 15, 70); //should be normal dist between 15 and 70
        int padding = (int)(log10(1000000 - i));//since cache has max memory of ~1mil bytes, should never need more than that many keys
        int filler = key_size - 10; //10 chars needed for "key_000000"; then the rest is filler "f"
        key_type key = "key_" + std::string(padding, '0') + std::to_string(i) + std::string(filler, 'f');//f for filler and f to pay respects to all those wasted bytes
        int val_size;//approximated with 3 distributions, so we select one first

        int p = percent_dist(rng);
        if(p < 40){
            val_size = std::clamp((int)(abs(val_dist_low(rng))), 2, 10);//use the normal dist, but want only positive nums and no greater than 10
        } else if(p < 90){
            val_size = val_dist_mid(rng);
        } else {
            val_size = std::clamp((int)(abs(val_dist_high(rng))), 0, 4500) + 500;//same as low vals, but need to boost by 500 since dist is centered at 0
        }
        size_used += val_size;
        int prob;
        p = percent_dist(rng);
        prob = (int)((1000)/(pow(p+1, 2.3)));
        total_prob_+= prob;
        data_type data_tuple = std::make_tuple(key, val_size, prob);
        data_.push_back(data_tuple);
        i++;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(data_.begin(), data_.end(), std::default_random_engine(seed));
}

Generator::~Generator()
{

}


Request Generator::gen_req(bool print_results, bool set_only)
{
    std::random_device rd;
    std::mt19937 rng{rd()};
    std::normal_distribution<double> key_dist(43.0, 9.0);//want a normal dist spanning 15-70, so use this and clamp later
    std::uniform_int_distribution<int> method_dist(0, 30);
    std::string method;
    if(set_only){
        method = "set";
    } else{
        int p = method_dist(rng);
        if(p < 21){
            method = "get";
        } else if (p < 30){
            method = "del";
            if(method_dist(rng) < 27){//8/9 of deletes should delete a nonexistent key to keep the cache full
                int dummy_key_length = std::clamp((int)key_dist(rng), 15, 70);
                if(print_results){
                    std::cout << std::string(dummy_key_length, 'f') + ", "<< std::to_string(1) + ", "<< method << std::endl;
                }
                return Request(std::string(dummy_key_length, 'f'), 1, method);//this is not a key that will ever be used for "set"
            }
        } else {
            method = "set";
        }
    }
    //now we select a key/value pair at random; the readme will describe how this process works
    std::uniform_int_distribution<int> kv_dist(0, total_prob_/locality_range_);
    int target = kv_dist(rng);
    int current = 0;
    long unsigned int i = current_locality_*(locality_shift_*(data_.size()-1))/locality_range_;//we start some ways into the data vector
    data_type kv_tuple;
    while(current + std::get<2>(data_[i]) < target){//getting the probability of the current data_tuple in the vector
        //iterate through
        current += std::get<2>(data_[i]);
        i+=1;
        if(i >= data_.size()){
            i = 0;
        }
    }
    current_locality_ += 1;
    if(current_locality_ >= locality_range_/locality_shift_){//If we've reach the end of the data vector
        current_locality_ = 0;//reset before the next call happens
    }
    kv_tuple = data_[i];
    key_type key = std::get<0>(kv_tuple);
    int val_size = std::get<1>(kv_tuple);
    if(print_results){
        std::cout << key + ", "<< std::to_string(val_size) + ", " << method << std::endl;
    }
    return Request(key, val_size, method);
}
