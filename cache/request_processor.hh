#pragma once
#include <iostream>
#include "cache.hh"
#include "kv_json.hh"
#include <boost/beast/version.hpp>
#include <boost/beast/http.hpp>
#include <mutex>
#include <shared_mutex>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

class request_processor {
private:
    mutable std::shared_mutex mutex_;
    //std::shared_lock lock(mutex_);
    //std::unique_lock lock(mutex_);
public:
    void fail(boost::beast::error_code ec, char const* what) const {
        std::cerr << what << ": " << ec.message() << "\n";
    }
    http::response<http::string_body> server_error(http::request<http::string_body> req, std::string message) const {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + message + "'";
        res.prepare_payload();
        return res;
    }
    http::response<http::string_body> bad_request(http::request<http::string_body> req) const {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "Unknown HTTP-method";
        res.prepare_payload();
        return res;
    }
    http::response<http::string_body> not_found(http::request<http::string_body> req, std::string target) const {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + target + "' was not found.\n";
        res.prepare_payload();
        return res;
    }
    http::response<http::string_body> handle_request(http::request<http::string_body> req, Cache* server_cache) const {
        //std::cout << "handling request" << std::endl;
        //std::cout << "\t request method: " << req.method() << std::endl;
        //std::cout << "\t request target: " << req.target() << std::endl;
        if( req.method() != http::verb::get &&
            req.method() != http::verb::put &&
            req.method() != http::verb::delete_ &&
            req.method() != http::verb::head &&
            req.method() != http::verb::post)
            return bad_request(req);

        http::response<http::string_body> res;
        res.version(11);   // HTTP/1.1
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.keep_alive(req.keep_alive());
        if(req.method() == http::verb::get)
        {
          key_type key = std::string(req.target()).substr(1); //make a string and slice off the "/"" from the target
          Cache::size_type size;
          //std::cout << "getting..." << key << std::endl;
          std::unique_lock lock(mutex_);
          Cache::val_type ret_val = server_cache->get(key, size);
          lock.unlock();
          if(ret_val == nullptr){
              //std::cout << "not found" << std::endl;
              return not_found(req, key);
          } else {
              char* val_holder = new char [size];
              std::strncpy (val_holder, ret_val, size);
              val_holder[size-1] = '\0';
              Cache::val_type val = val_holder;
              //std::cout << "cache["<<key<<"]=" << val << std::endl;
              res.result(boost::beast::http::status::ok);
              kv_json kv(key, val);
              std::string json = kv.as_string();
              res.body() = json;
              res.set(boost::beast::http::field::content_type, "json");
              res.content_length(json.size() + 1);
              res.prepare_payload();
              delete[] val_holder;
              return res;
          }
        }

        //Handles a PUT request
        //Maybe add error checking to ensure the things are of the correct types? or not necessary?
        if(req.method() == http::verb::put)
        {
            //First we extract the key and the value from the request target
            std::stringstream target_string(std::string(req.target()).substr(1)); //Slice off the first "/" then make a sstream for further slicing
            std::string key_str;
            std::string val_str;
            std::getline(target_string, key_str, '/');
            std::getline(target_string, val_str, '/');
            //std::cout << "setting " << key_str << " to " << val_str << std::endl;
            //And now we need to convert the value into a char pointer so we can insert into the cache
            key_type key = key_str;
            Cache::val_type val = const_cast<char*>(val_str.c_str());
            bool key_created = false;
            Cache::size_type size = 0;
            //We then check if the key is already in the Cache (need for status code) and then set the value
            std::unique_lock lock(mutex_);
            if(server_cache->get(key_str, size) == nullptr){
                key_created = true;
            }
            lock.unlock();
            size = val_str.length()+1;
            //std::cout << "setting...";
            std::unique_lock write_lock(mutex_);
            server_cache->set(key, val, size);
            write_lock.unlock();
            //std::cout << "done" << std::endl;
            //Now we can create and send the response
            res.set(boost::beast::http::field::content_location, "/" + key_str);
            //set the appropriate status code based on if a new entry was created
            if(key_created){
                res.result(201);
            } else {
                res.result(204);
            }
            res.keep_alive(true);
            res.prepare_payload();
            // std::cout << "cache[" << key << "] now equals: " << server_cache -> get(key, size) << std::endl;
            return res;
        }

        //Will send a response for if key was deleted or not found; same effects either way
        if(req.method() == http::verb::delete_) {
            key_type key = std::string(req.target()).substr(1);
            //std::cout << "deleting " << key << "...";
            std::unique_lock lock(mutex_);
            auto success = server_cache->del(key);
            lock.unlock();
            if(success){
                //std::cout << "done" << std::endl;
                res.result(boost::beast::http::status::ok);
            } else {
                //std::cout << "error: not found" << std::endl;
                return not_found(req, key);
            }
            res.keep_alive(req.keep_alive());
            res.prepare_payload();
            return res;
        }

        if(req.method() == http::verb::head)
        {
            res.result(http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json");
            res.set(boost::beast::http::field::accept, "application/json");
            std::unique_lock lock(mutex_);
            res.insert("Space-Used", server_cache->space_used());
            lock.unlock();
            res.prepare_payload();
            return res;
        }

        if(req.method() == http::verb::post) {
            if(std::string(req.target()) != "/reset"){
                return not_found(req, std::string(req.target()));
            }
            //Resets the cache and sends back a basic response with string body

            std::cout << "resetting the cache";
            std::unique_lock lock(mutex_);
            server_cache->reset();
            lock.unlock();
            //std::cout << "done" << std::endl;
            http::response<boost::beast::http::string_body> res;
            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.body() = "The Cache has been reset";
            res.set(boost::beast::http::field::content_type, "text");
            res.prepare_payload();
            return res;
        }
        return bad_request(req);

    }
};
