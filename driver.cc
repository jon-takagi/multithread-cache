#include <algorithm>
#include <random>
#include "driver.hh"
#include <boost/math/distributions/pareto.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <iostream>
#include <vector>
#include <tuple>
#include <cmath>
#include <cstring>

const int PERCENTILE_METRIC = 95;
Driver::Driver(Cache* cache, Generator gen):gen_(gen)
{
    cache_ = cache;
}

Driver::~Driver()
{
    //reset the cache? I forgot what this does
}

//warm generates new data and adds to vector until cache has gotten at least size total sets
//It creates tuples of key, val, size, and probability that are drawn from distributions designed to mimic the ETC workload
void Driver::warm(int size)
{
    int sets = 0;
    while(sets < size) {
        Request req = gen_.gen_req(false, true);
        if(req.method_ == "set") {
            std::string val_str = std::string(req.val_size_, 'B');
            //Cache::val_type val = val_str.c_str();
            char * val = new char [req.val_size_+1];
            std::strcpy (val, val_str.c_str());
            cache_->set(req.key_, val, req.val_size_+1);
            delete[] val;
            sets += 1;
        }
    }
}

void Driver::set_request(key_type key, Cache::val_type val, Cache::size_type size)
{
    cache_->set(key, val, size);
}

Cache::val_type Driver::get_request(key_type key)
{
    Cache::size_type size;
    return cache_->get(key, size);
}

bool Driver::del_request(key_type key)
{
    return cache_->del(key);
}

int Driver::head_request()
{
    return cache_->space_used();
}
//Delete driver data and reset cache as well
void Driver::reset()
{
    cache_->reset();
// warm generates new data and adds to vector until cache has gotten at least size total data
}

double Driver::time_single_request() {
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


// param: number of requests to make
// return: vector containing the time for each measurement
std::vector<double> Driver::baseline_latencies(int nreq) {
    std::vector<double> results(nreq);
    for(int i = 0; i < nreq; i++) {
        results[i] = time_single_request();
    }
    return results;
}

std::pair<double, double> Driver::baseline_performance(int nreq) {
    std::vector<double> latencies = baseline_latencies(nreq);
    int index = PERCENTILE_METRIC * nreq / 100;
    std::sort(latencies.begin(), latencies.end());
    double percentile = latencies[index];
    double total_latency = std::accumulate(latencies.begin(), latencies.end(), 0);
    double throughput = nreq / total_latency * std::milli::den;
    return std::make_pair(percentile, throughput);
}

void  Driver::do_nreq_requests(int nreq, std::promise<std::vector<double>> *promObj)
{
    std::vector<double> results(nreq, -1.0);
    for(int i = 0; i < nreq; i++ ) {
        results[i] = time_single_request();
    }
    promObj->set_value(results);
}

std::pair<double, double> Driver::threaded_performance(int nthreads, int nreq) {
    std::vector<std::thread> threads(nreq);
    std::vector<std::promise<std::vector<double>>> promises(nthreads);
    std::vector<std::future<std::vector<double>>> futures(nthreads);
    std::vector<std::vector<double>> results(nthreads, std::vector<double>(nreq));
    for(int i = 0; i < nthreads; i++){
        futures[i] = promises[i].get_future();
        threads.push_back(std::thread(&Driver::do_nreq_requests, this, nreq, &(promises[i])));
        threads[i].join();
    }

    for(int i = 0; i < nthreads; i++ ) {
        results[i] = futures[i].get();
    }

    std::vector<double> big_results(nthreads * nreq, 0.0);
    for(int i = 0; i < nthreads; i++) {
        for(int j = 0; j < nreq; j++) {
            big_results[i * nreq + j] = results[i][j];
        }
    }
    double percentile = big_results[.95 *  nreq * nthreads];
    double total_latency = std::accumulate(big_results.begin(), big_results.end(), 0);
    double throughput = (nreq * nthreads) / total_latency * std::milli::den;
    return std::make_pair(percentile, throughput);

}
