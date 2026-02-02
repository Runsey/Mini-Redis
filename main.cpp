#include <iostream>
#include <cstdlib>
#include "server.h"

int main(int argc, char** argv){
cout<<"Mini-Reids Startign"<<endl;
 int port =atoi(argv[1]);
 RedisServer server(port);
 server.start();
 return 0;   
}