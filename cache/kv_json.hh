#include "cache.hh"
#include "evictor.hh"
#include <sstream>
#include <exception>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using key_type = std::string;
struct kv_json {
    key_type key_;
    Cache::val_type value_;

    kv_json(std::string arg) {
        std::vector<std::string> cont;
        std::size_t current, previous = 0;
        std::string delim = "\"";
        current = arg.find(delim);
        while (current != std::string::npos) {
            cont.push_back(arg.substr(previous, current - previous));
            previous = current + 1;
            current = arg.find(delim, previous);
        }
        cont.push_back(arg.substr(previous, current - previous));
        key_ = cont[3];
        value_ = cont[7].c_str();
    }
    kv_json(key_type k, Cache::val_type val) {
        key_ = k;
        value_ = val;
    }
    std::string as_string() {
        return "{ \"key\": \"" + key_ + "\", \"value\": \"" + value_ + "\"}";
    }
};
