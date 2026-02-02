#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
using namespace std;

class KeyValueStore {
public:
    bool set(const std::string& key, const std::string& value, int ttl_secons=-1);
    bool get(const std::string& key, std::string& out);
    bool del(const std::string& key);
    void taken();
    void clean_expiry();
    

private:
    std::unordered_map<std::string, std::string> data_;
    std::unordered_map<std::string, chrono::steady_clock::time_point> expiry; 
    std::mutex mtx_;
};
