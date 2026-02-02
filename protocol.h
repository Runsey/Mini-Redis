#pragma once
#include <string>
#include <vector>

std::string resp(int client_fd);

std::vector<std::string> parse_command(const std::string& line);
