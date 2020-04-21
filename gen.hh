#include "request.hh"
#include <vector>

//The generator produces a set of data that emulates the workload of the ETC cache
//It also can generate a single request from said data based on the appropriate distributions
class Generator {

    public:
        using data_type = std::tuple<key_type, int, int>; //holds key, value size, and probability
        using req_type = Request;
    private:
        std::vector<data_type> data_;//data vector, which holds all the kv pairs and their data that will be used for this driver
        int total_prob_; //tracks the sum total of the probability entries in the data vector, as this is fixed after one warm call
        int locality_range_;
        double locality_shift_;
        int current_locality_;

    public:
        Generator(int locality_range, //how much of the data vec we draw from at once
                  double locality_shift, //how much we shift our "temporal zone" with each call
                  int size, //the size of the cache we're using this for
                  double size_factor); //adjustable factor of how much data we should produce; is multiplied by size

        ~Generator();

        //Generates a new request for the cache; the size, frequency, values, etc. are chosen at random based on the distributions in the driver's private data
        Request gen_req(bool print_results = false, bool set_only = false); //for testing purposes; will print the requests and reponses in gen_req

};
