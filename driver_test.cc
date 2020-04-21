#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "driver.hh"
#include <iostream>
#include <fstream>
#include <cmath>
#include "cache/fifo_evictor.h"

const int CACHE_SIZE = 8192;
const int TRIALS = 1000000;
Generator gen = Generator(8, 0.2, CACHE_SIZE, 8);

auto test_cache = Cache("127.0.0.1", "42069"); //Add the appropriate params here once chosen

// FifoEvictor fifo_evictor = FifoEvictor();
// Evictor* evictor = &fifo_evictor;
// auto test_cache = Cache(CACHE_SIZE, 0.75, evictor); //Add the appropriate params here once chosen

Cache::size_type size;
auto driver = Driver(&test_cache, gen);

//Tests to ensure the driver has a connection to the server/cache
//And that all types of basic requests are actually working and getting a response
// TEST_CASE("Connection")
// {
//
//     SECTION("SET/GET"){//can't test set without using get
//         driver.set_request("key_one", "value_1", 8);
//         Cache::val_type val = "value_2";
//         REQUIRE(*driver.get_request("key_one") == *val);
//     }
//
//     SECTION("DELETE"){
//         REQUIRE(driver.del_request("key_one") == true);
//         REQUIRE(driver.get_request("key_one") == nullptr);
//     }
//
//     driver.reset();//resets cache, hitrate, and ... ?
// }


TEST_CASE("warm")
{
    SECTION("Warm"){//adds new values to cache summing to given size
        driver.warm(CACHE_SIZE);
        REQUIRE(driver.head_request() > 0.9 * CACHE_SIZE);//fix this
    }

    driver.reset();
}



TEST_CASE("Hitrate")
{
    SECTION("Hitrate at ~80%"){
        int hits = 0;
        driver.warm(CACHE_SIZE);
        int gets = 0;
        while (gets < TRIALS) {
            auto req = gen.gen_req(false);
            std::string method = req.method_;
            if(method == "get") {
                gets += 1;
                Cache::val_type res = driver.get_request(req.key_);
                if(res != nullptr){
                    hits += 1;
                }
            }
        }
        std::cout<< "hits : " + std::to_string(hits) << " out of " + std::to_string(TRIALS) << std::endl;
        REQUIRE(hits > TRIALS * 0.75);
        REQUIRE(hits < TRIALS * 0.85);
    }

    driver.reset();
}
TEST_CASE("prep_data") {
    SECTION("graph") {
        driver.warm(CACHE_SIZE);
        auto latencies = driver.baseline_latencies(TRIALS);
        std::sort(latencies.begin(), latencies.end());
        std::ofstream output;
        int num_bins = 100;
        std::vector<int> bins(num_bins, 0);

        output.open("latencies.dat");
        for(int i = 0; i < num_bins; i++) {
            output << latencies[i * TRIALS / 100] << "\t" << i << std::endl;
        }
        output.close();
    }
    driver.reset();
}

TEST_CASE("performance") {
    SECTION("part a") {
        driver.warm(CACHE_SIZE);
        auto results = driver.baseline_performance(TRIALS);
        std::cout << "95th percentile latency: " << results.first << "ms"<< std::endl;
        std::cout << "mean throughput: " << results.second << "req/s" << std::endl;
    }
    driver.reset();
}
