//
// Created by Stas Piter on 27.02.2021.
//

#ifndef FASTCGI_BLOG_GENERATOR_H
#define FASTCGI_BLOG_GENERATOR_H

#include <regex>

#include "Tree.h"

#include <iostream>


class Generator {

public:

    static std::string Generate(Node* currentPage, Node* templatePage, const std::string& templateName, std::vector<std::string>* pages) {

        Node* root = currentPage->getRoot();
        std::string t = currentPage->getRoot()->get(templateName)->getValue();
        std::string result;

        static const std::regex tagRegex(R"(\<\!\-\-\s*(\w+)\((.*?)\)\s*\-\-\>)");
        std::smatch m;
        std::string function;
        std::vector<std::string> params;
        std::string::const_iterator searchStart(t.cbegin());
        int lastPartPos = 0;

        while (regex_search(searchStart, t.cend(), m, tagRegex)) {
            function = m[1];
            params = Utils::Tokenize(m[2]);

            result.append(t.substr(lastPartPos, m.position()));

            lastPartPos += m.position() + m.length();

            if (function == "print" && !params.empty()) {
                Node* p = templatePage;
                if (params[0][0] == '@') {
                    params[0] = params[0].substr(1);
                    p = currentPage;
                }
                auto n = p->get(params[0]);
                if (n)
                    result.append(n->getValue());
            }

            else if (function == "template" && !params.empty()) {

                for (const auto &subTemplateName : params) {
                    std::string pagePath;
                    bool pageFound = false;
                    for (const auto &page : *pages) {
                        pagePath += '/' + page;
                        auto pagePathSplit = Utils::Split(page, '.');
                        if (std::find(pagePathSplit.begin(), pagePathSplit.end(), subTemplateName) != pagePathSplit.end()) {
                            pageFound = true;
                            break;
                        }
                    }

                    if (pageFound) {
                        std::vector<std::string> newPages;
                        auto n = root->get(pagePath, &newPages);
                        std::string subTemplate = Generate(currentPage, n, subTemplateName, &newPages);

                        result.append(subTemplate);
                    }
                }

            }

            else
                result.append(m.str());

            searchStart = m.suffix().first;
        }

        result.append(t.substr(lastPartPos));

        return result;

    }

};


#endif //FASTCGI_BLOG_GENERATOR_H
