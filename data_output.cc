#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <future>
#include <algorithm>
#include <fstream>
#include "gen.hh"
#include <stdio.h>
#include <string.h>

void warm(Generator gen_, Cache* cache_, int size)
{
    int sets = 0;
    while(sets < size) {
        Request req = gen_.gen_req(false, true);
        if(req.method_ == "set") {
            std::string val_str = std::string(req.val_size_, 'B');
            //Cache::val_type val = val_str.c_str();
            char* val = new char [req.val_size_+1];
            strcpy (val, val_str.c_str());
            cache_->set(req.key_, val, req.val_size_+1);
            delete[] val;
            sets += 1;
        }
    }
}

template<typename Numeric, typename Generator = std::mt19937>
Numeric random(Numeric from, Numeric to)
{
    thread_local static Generator gen(std::random_device{}());

    using dist_type = typename std::conditional
    <
        std::is_integral<Numeric>::value
        , std::uniform_int_distribution<Numeric>
        , std::uniform_real_distribution<Numeric>
    >::type;

    thread_local static dist_type dist;

    return dist(gen, typename dist_type::param_type{from, to});
}


double time_single_request(Generator gen_, Cache* cache_) {
    std::chrono::time_point<std::chrono::high_resolution_clock> t1;
    std::chrono::time_point<std::chrono::high_resolution_clock> t2;
    Request req = gen_.gen_req(false);
    Cache::size_type size = 0;
    std::string val_str = std::string(req.val_size_, 'B');
    Cache::val_type val = val_str.c_str();
    if(req.method_ =="get") {
        t1 = std::chrono::high_resolution_clock::now();
        cache_->get(req.key_, size);
    // std::cout << std::get<2>(req) << " [key: " << std::get<0>(req) << ", val: " << std::get<1>(req) <<"]"<< std::endl;
        t2 = std::chrono::high_resolution_clock::now();
    } else if (req.method_ == "set") {
        t1 = std::chrono::high_resolution_clock::now();
        cache_->set(req.key_, val, req.val_size_+1);
    // std::cout << std::get<2>(req) << " [key: " << std::get<0>(req) << ", val: " << std::get<1>(req) <<"]"<< std::endl;
        t2 = std::chrono::high_resolution_clock::now();
    } else if (req.method_ == "del") {
        t1 = std::chrono::high_resolution_clock::now();
        cache_->del(req.key_);
    // std::cout << std::get<2>(req) << " [key: " << std::get<0>(req) << ", val: " << std::get<1>(req) <<"]"<< std::endl;
        t2 = std::chrono::high_resolution_clock::now();
    }
    std::chrono::duration<double, std::milli> elapsed = std::chrono::duration_cast<std::chrono::duration<double, std::milli>> (t2-t1);
    return elapsed.count();
}

void do_nreq_requests(Generator gen_, Cache* cache_, int nreq, std::promise<std::vector<double>> *promObj)
{
    std::cout << "timing " << nreq << " requests" << std::endl;
    std::vector<double> results(nreq, -1.0);
    for(int i = 0; i < nreq; i++ ) {
        results[i] = time_single_request(gen_, cache_);
        if(i % (nreq / 100) == 0) {
            std::cout << ".";
        }
    }
    std::cout << std::endl << "returning" << std::endl;
    promObj->set_value(results);
}

int main()
{
    // for num_threads in 1-8:
        // copy code from thread_test.cc
        // output << num_threads << "\t" << percentile << "\t" << latency << std::endl;
    return 0;
}
