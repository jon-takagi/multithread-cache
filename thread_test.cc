#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <future>
#include <algorithm>

#include "gen.hh"


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

// int main() {
//     const int CACHE_SIZE = 8192;
//     const int TRIALS = 10000;
//     // const int THREADS = 1;
//     Generator gen = Generator(8, 0.2, CACHE_SIZE, 8);
//     auto test_cache = Cache("127.0.0.1", "42069");
//
//
//
//     std::promise<std::vector<double>> promise_0;
//     // std::promise<std::vector<double>> promise_1;
//     std::future <std::vector<double>> future_0;
//     // std::future <std::vector<double>> future_1;
//     std::vector<double> result_0(TRIALS);
//     // std::vector<double> result_1(nreq);
//
//     future_0 = promise_0.get_future();
//     // future_1 = promise_1.get_future();
//
//     std::cout << "creating thread 0" << std::endl;
//     std::thread thread_0(do_nreq_requests, gen, &test_cache, TRIALS, &promise_0);
//     // std::thread thread_1(do_nreq_requests, nreq, &promise_1);
//     thread_0.join();
//     std::cout << "back in main" << std::endl;
//     // thread_1.join();
//     result_0 = future_0.get();
//     // result_1 = future_1.get();
//
//     std::cout << result_0[0] << std::endl;
//     // std::cout << result_1[0] << std::endl;
//     return 0;
// }
    // driver.warm(CACHE_SIZE);
    // std::promise<std::vector<double>> promise;
    // std::thread(do_nreq_requests, gen, &test_cache, TRIALS, &(promises[i]));
    // // auto results = driver.threaded_performance(THREADS, TRIALS);
    // std::cout << "95th percentile latency: " << results.first << "ms"<< std::endl;
    // std::cout << "mean throughput: " << results.second << "req/s" << std::endl;
    // return 0;
    //
int main()
{
    const int CACHE_SIZE = 8192;
    const int TRIALS = 10000;
    const int THREADS = 8;
    Generator gen = Generator(8, 0.2, CACHE_SIZE, 8);
    auto test_cache = Cache("127.0.0.1", "42069");

    std::vector<std::thread> threads;
    std::vector<std::promise<std::vector<double>>> promises(THREADS);
    std::vector<std::future<std::vector<double>>> futures(THREADS);
    std::vector<std::vector<double>> results(THREADS, std::vector<double>(TRIALS));
    for(int i = 0; i < THREADS; i++){
        futures[i] = promises[i].get_future();
        threads.push_back(std::thread(do_nreq_requests, gen, &test_cache, TRIALS, &(promises[i])));
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
