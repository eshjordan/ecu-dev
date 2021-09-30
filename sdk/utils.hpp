#pragma once
#include "RTOS.hpp"
#include <sstream>
#include <string>
#include <vector>

inline std::vector<std::string> &split(const std::string &str, char delimiter)
{
    std::vector<std::string> *tokens = new std::vector<std::string>();
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delimiter))
    {
        if (!token.empty())
        {

            auto r_pos = token.find("\r");
            if (r_pos != std::string::npos) { token.replace(r_pos, 1, ""); }

            auto n_pos = token.find("\n");
            if (n_pos != std::string::npos) { token.replace(n_pos, 1, ""); }

            tokens->push_back(token);
        }
    }
    return *tokens;
}
