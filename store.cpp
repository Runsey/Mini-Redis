#include "store.h"
#include <string>
#include <mutex>
#include <chrono>
#include<shared_mutex>
#include<unordered_map>
#include<iostream>
#include<thread>
using namespace std;
void KeyValueStore::taken(){
        cout << "InSET tid=" << std::this_thread::get_id() << endl;

std::unique_lock<std::mutex> lock(mtx_, std::try_to_lock);
if (!lock.owns_lock()) {
    cout << "FAILED TO LOCK mtx_ (someone else holds it)" << endl;
    return;
}
}

bool KeyValueStore::set(const string& key, const string& value, int ttl_seconds){
    cout << "InSET tid=" << std::this_thread::get_id() << endl;
    lock_guard<mutex> lock(mtx_);
    cout<<"lock"<<endl;
    data_[key]=value;
    if(ttl_seconds>0){
        auto now= chrono::steady_clock::now();
        cout<<"between "<<endl;
        expiry[key]=now +chrono::seconds(ttl_seconds);
    }
    return true;
}

bool KeyValueStore::get(const std::string& key, std::string& out){
    lock_guard<mutex> lock(mtx_);
    cout<<"1"<<this_thread::get_id()<<endl;
    auto it=data_.find(key);
    if(it==data_.end()){
        return false;
    }
    if(expiry.find(key)!=expiry.end()){
        if(chrono::steady_clock::now()>expiry[key]){
            cout<<"4"<<this_thread::get_id()<<endl;
            data_.erase(it);
            expiry.erase(key);
            return false;
        }
    }
    out=it->second;
    return true;
}

bool KeyValueStore::del(const std::string& key){
    lock_guard<mutex> lock(mtx_);
    auto it=data_.find(key);
    if(it==data_.end()){
        return false;
    }
    data_.erase(it);
    return true; 

}

void KeyValueStore::clean_expiry(){
    auto now =chrono::steady_clock::now();
    lock_guard<mutex> lock(mtx_);
    for(auto it=expiry.begin(); it!=expiry.end();){
        if(now>it->second){
            data_.erase(it->first);
            it=expiry.erase(it);
        }
        else{
            ++it;
        }
    }

}