#include "protocol.h"
#include <sstream>

std::string resp(int client_fd){
    char first[1];
    int n=recv(client_fd, first, 1, 0)
    if(n<=0)return "";
    
}

std::vector<std::string> parse_command(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;

    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}
