#pragma once
#include "store.h"

class RedisServer {
public:
    RedisServer(int port);
    void start();
    int get_port() const{ return port_;}

private:
    int port_;
    int server_fd;
    KeyValueStore store_;

    void load_history(int log_fd);
    void handle_client(int client_fd, int log_fd);
    void clean_expired_keys();
};


