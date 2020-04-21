#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <future>


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

void do_nreq_requests(int nreq, std::promise<std::vector<double>> *promObj)
{
    std::vector<double> results(nreq, -1.0);
    for(int i = 0; i < nreq; i++ ) {
        results[i] = random(10,500);
    }
    promObj->set_value(results);
}

int main()
{
    int nreq = 100;
    int num_threads = 1;
    std::vector<std::thread> threads(num_threads);
    std::vector<std::promise<std::vector<double>>> promises(nreq);
    std::vector<std::future<std::vector<double>>> futures(nreq);
    std::vector<std::vector<double>> results(num_threads, std::vector<double>(nreq));
    for(int i = 0; i < num_threads; i++){
        futures[i] = promises[i].get_future();
        threads[i](do_nreq_requests, nreq, &promises[i]);
        threads[i].join();
    }

    for(int i = 0; i < num_threads; i++ ) {
        results[i] = futures[i].get();
    }

    std::vector<double> result = results[0];
    std::cout << "95th percentile: " << result[95] << std::endl;

    std::cout<<"Exit of Main function"<<std::endl;
    return 0;
}


// double lower_bound = 10;
// double upper_bound = 500;
// std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
// std::default_random_engine re;
// double a_random_double = unif(re);
// a_random_double = unif(re);
// results[i] = a_random_double;
