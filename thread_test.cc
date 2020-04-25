#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <future>
#include <algorithm>
#include <fstream>
#include "gen.hh"
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
    std::vector<double> results(nreq, -1.0);
    for(int i = 0; i < nreq; i++ ) {
        results[i] = time_single_request(gen_, cache_);
    }
    promObj->set_value(results);
}

int main()
{
    const int CACHE_SIZE = 8192;
    const int TRIALS = 1000000;
    const int THREADS = 4;
    Generator gen = Generator(8, 0.2, CACHE_SIZE, 8);
    std::vector<std::thread> threads;
    std::vector<Cache*> clients(THREADS, 0x0);
    std::vector<std::promise<std::vector<double>>> promises(THREADS);
    std::vector<std::future<std::vector<double>>> futures(THREADS);
    std::vector<std::vector<double>> results(THREADS, std::vector<double>(TRIALS));
    for(int i = 0; i < THREADS; i++){
        futures[i] = promises[i].get_future();
        clients[i] = new Cache("127.0.0.1", "42069");
        warm(gen, clients[i], CACHE_SIZE/THREADS);
        threads.push_back(std::thread(do_nreq_requests, gen, clients[i], TRIALS, &(promises[i])));
    }
    for(int i = 0; i < THREADS; i++) {
        threads[i].join();
    }
    for(int i = 0; i < THREADS; i++ ) {
        results[i] = futures[i].get();
    }

    std::vector<double> big_results(THREADS * TRIALS, 0.0);
    for(int i = 0; i < THREADS; i++) {
        for(int j = 0; j < TRIALS; j++) {
            big_results[i * TRIALS + j] = results[i][j];
        }
    }
    double percentile = big_results[.95 *  TRIALS * THREADS];
    std::cout << "95th percentile: " << percentile << "ms" << std::endl;
    return 0;
}


// double lower_bound = 10;
// double upper_bound = 500;
// std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
// std::default_random_engine re;
// double a_random_double = unif(re);
// a_random_double = unif(re);
// results[i] = a_random_double;
