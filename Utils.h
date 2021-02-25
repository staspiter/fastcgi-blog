//
// Created by Stas on 25.02.2021.
//

#ifndef FASTCGI_BLOG_UTILS_H
#define FASTCGI_BLOG_UTILS_H

#include <string>

class Utils {

public:

    static std::vector<std::string> Split(const std::string& s, char delim) {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;
        while (getline(ss, item, delim))
            result.push_back(item);
        return result;
    }

};

#endif //FASTCGI_BLOG_UTILS_H
