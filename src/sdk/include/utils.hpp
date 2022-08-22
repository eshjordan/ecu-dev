#pragma once

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// #include "RTOS.hpp"
// #include <sstream>
// #include <string>
// #include <vector>

// inline std::vector<const char *> &split(const char * &str, char delimiter)
// {
//     std::vector<const char *> *tokens = new std::vector<const char *>();
//     const char * token;
//     std::istringstream tokenStream(str);

//     while (getline(tokenStream, token, delimiter))
//     {
//         if (!token.empty())
//         {

//             auto r_pos = token.find("\r");
//             if (r_pos != std::string::npos) { token.replace(r_pos, 1, ""); }

//             auto n_pos = token.find("\n");
//             if (n_pos != std::string::npos) { token.replace(n_pos, 1, ""); }

//             tokens->push_back(token);
//         }
//     }
//     return *tokens;
// }
