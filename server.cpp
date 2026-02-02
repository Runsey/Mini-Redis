#include "server.h"
#include "protocol.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
using namespace std;

    // Define the constructor inside the namespace for simplicity
    RedisServer::RedisServer(int port){
        port_=port;
    }


void RedisServer::start()
{
 //   int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    auto log_fd = open("aof.log", O_APPEND | O_CREAT | O_RDWR, 0644);
    load_history(log_fd);
 int server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
if (server_fd < 0) { perror("socket"); exit(1); }
//store_.taken();
//int opt = 1;
//setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_port = htons(port_);
addr.sin_addr.s_addr = htonl(INADDR_ANY);  // listen on all interfaces

if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
}
    

    if (listen(server_fd, SOMAXCONN) < 0)
    {
        cerr << "Listen failed" << endl;
        exit(1);
    }
    
    cout << "Server sucesfully started on port" << port_ << endl;
    thread cleaner(&RedisServer::clean_expired_keys, this);
    cleaner.detach();

    int epoll_fd = epoll_create(1);

    struct epoll_event ev_listener;
    ev_listener.events=EPOLLIN;
    ev_listener.data.fd=server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev_listener);

    while (true)
    {
        struct epoll_event events[10];
        int fd_count= epoll_wait(epoll_fd, events, 10, -1);
        cout<<"EVTNTS"<<fd_count<<endl;
        for(int i=0;i<fd_count;i++){
            if(events[i].data.fd!=server_fd){
                cout<<"EVEnt on client"<<endl;
                handle_client(events[i].data.fd, log_fd);
            }
            else{
                while(true){
                int client_fd = accept4(server_fd, nullptr, nullptr,SOCK_NONBLOCK);
                cout<<"ACCEPTING"<<endl;
                if (client_fd<0){
                    if(errno==EAGAIN|| errno==EWOULDBLOCK){ break;}
                    perror("accept");
                    break;
                }
                struct epoll_event ev;
                ev.events=EPOLLIN;
                ev.data.fd=client_fd;
                ev.events=EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            }
            }
        }


    }

}

void RedisServer::handle_client(int client_fd, int log_fd)
{   cout<<"HANDLE CLIENT"<<endl;
    char buffer[1024];

        int n=recv(client_fd, buffer, sizeof(buffer), 0);
        if ( n<= 0) {
            close(client_fd);
            return;
        }
        string input(buffer, n);
        char header[6];
        snprintf(header, sizeof(header), "%05zu", (int)input.size()+1);
        string size_padder(header);
        string log_write= size_padder+input;
        vector<string> command = parse_command(input);
        for(string s: command){ cout<<s<<" ";}
        if (command.size() == 0){
            string response = "No command given\n";
            send(client_fd, response.c_str(), response.size(), 0);
            return;
        }
        
        cout<<input.size()<<endl;
        string response = "Failed command\n";
        if (command[0] == "SET"){
            cout<<"s"<<endl;
            write(log_fd, log_write.c_str(), log_write.size());
            if (command.size() == 4){
                int ttl = stoi(command[3]);
                send(client_fd, response.c_str(), response.size(), 0);
                
                if (store_.set(command[1], command[2], ttl))
                {
                    response = "Succesfully set key with TTL\n";
                }
            }
            else if (store_.set(command[1], command[2])){
                            cout<<"fgh"<<endl;

                response = "Succesfully set key\n";
            }
            else cout<<"end"<<endl;
        }
        if (command[0] == "GET")
        {
            string value;
            if (store_.get(command[1], value))
            {
                response = "Value: " + value + "\n";
            }
            else{
                response= "Key not found\n";
            }
        }
        if (command[0] == "DEL")
        {   
            send(client_fd, response.c_str(), response.size(), 0);
            write(log_fd, log_write.c_str(), log_write.size());
            if (store_.del(command[1]))
            {
                response = "Successfully delted key\n";

            }
        }
        send(client_fd,response.c_str(),response.size(),0);

    
}

void RedisServer::clean_expired_keys(){
    auto next= chrono::steady_clock::now();
    while(true){
        next += chrono::milliseconds(100);
        this_thread::sleep_until(next);
        store_.clean_expiry();
    }
}
    

void RedisServer::load_history(int log_fd){
    char buffer[1024];
    int n=read(log_fd, buffer, 5);

    cout<<"Star load history "<<n<<endl;
    while (n>0)
    {   
        int num=stoi(string(buffer, 5));
        read(log_fd, buffer, num);
        string line = string(buffer, num);
        vector<string> command = parse_command(line);
        cout<<command.size()<<endl;
        cout<<line<<command[0]<<endl;
        if (command[0] == "SET")
        {
            cout<<"Load set"<<endl;
            if (command.size() == 4)
            {
                int ttl = stoi(command[3]);
                store_.set(command[1], command[2], ttl);
            }
            else store_.set(command[1], command[2]);
        }
        if (command[0] == "DEL")
        {store_.del(command[1]);
            cout<<"delete load"<<endl;
        }
        n=read(log_fd, buffer, 4);
    }
}
