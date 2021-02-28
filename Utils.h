//
// Created by Stas Piter on 25.02.2021.
//

#ifndef FASTCGI_BLOG_UTILS_H
#define FASTCGI_BLOG_UTILS_H

#include <string>
#include <vector>

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

    static std::string IsPageApplicable(const std::string& name, const std::string& fullName) {
        if (name == fullName)
            return fullName;

        // pagename -> pagename.category.json
        if (fullName.length() > name.length() && fullName.compare(0, name.length(), name) == 0
            && fullName[name.length()] == '.') {
            return fullName;
        }

        return "";
    }

    static std::vector<std::string> Tokenize(const std::string& str) {
        std::vector<std::string> v;

        std::istringstream iss(str);
        std::string s;

        while (iss >> std::quoted(s))
            v.push_back(s);

        return v;
    }

};

#endif //FASTCGI_BLOG_UTILS_H
