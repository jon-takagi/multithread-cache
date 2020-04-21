#include <iostream>
#include "gen.hh"
#include <chrono>
#include <cstring>

//We had some issues with values and keys of length zero being returned from the Generator
//So we used/modified this test to check for that
int main() {
    auto gen = Generator(120, 0.3, 8192, 120);
    double total = 0;
    int zero_length_vals = 0;
    while(total< 100000) {
        Request req = gen.gen_req(false);
        if(req.method_ == "set") {
            if(req.val_size_ == 0) {
                zero_length_vals += 1;
            }
        }
        total++;
    }
    std::cout << zero_length_vals << " of " << total << " values found tested had length 0 (" << std::to_string(zero_length_vals / total * 100)<< "%)" << std::endl;
    return 0;
}
