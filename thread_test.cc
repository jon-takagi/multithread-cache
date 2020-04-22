#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <future>
#include <algorithm>


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
class Requestor {
public:
    static void do_nreq_requests(int nreq, std::promise<std::vector<double>> *promObj)
    {
        std::vector<double> results(nreq, -1.0);
        for(int i = 0; i < nreq; i++ ) {
            results[i] = random(10,500);
        }
        promObj->set_value(results);
    }
};


// int main() {
//     int nreq = 100;
//     std::promise<std::vector<double>> promise_0;
//     std::promise<std::vector<double>> promise_1;
//     std::future <std::vector<double>> future_0;
//     std::future <std::vector<double>> future_1;
//     std::vector<double> result_0(nreq);
//     std::vector<double> result_1(nreq);
//
//     future_0 = promise_0.get_future();
//     future_1 = promise_1.get_future();
//     std::thread thread_0(do_nreq_requests, nreq, &promise_0);
//     std::thread thread_1(do_nreq_requests, nreq, &promise_1);
//     thread_0.join();
//     thread_1.join();
//     result_0 = future_0.get();
//     result_1 = future_1.get();
//
//     std::cout << result_0[0] << std::endl;
//     std::cout << result_1[0] << std::endl;
//     return 0;
// }

int main()
{
    int nreq = 100;
    int num_threads = 2;
    std::vector<std::thread> threads;
    std::vector<std::promise<std::vector<double>>> promises(num_threads);
    std::vector<std::future<std::vector<double>>> futures(num_threads);
    std::vector<std::vector<double>> results(num_threads, std::vector<double>(nreq));
    for(int i = 0; i < num_threads; i++){
        futures[i] = promises[i].get_future();
        threads.push_back(std::thread(Requestor::do_nreq_requests, nreq, &(promises[i])));
        threads[i].join();
    }

    for(int i = 0; i < num_threads; i++ ) {
        results[i] = futures[i].get();
    }

    std::vector<double> big_results(num_threads * nreq, 0.0);
    for(int i = 0; i < num_threads; i++) {
        for(int j = 0; j < nreq; j++) {
            big_results[i * nreq + j] = results[i][j];
        }
    }
    double percentile = big_results[.95 *  nreq * num_threads];
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
