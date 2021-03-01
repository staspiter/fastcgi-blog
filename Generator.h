//
// Created by Stas Piter on 27.02.2021.
//

#ifndef FASTCGI_BLOG_GENERATOR_H
#define FASTCGI_BLOG_GENERATOR_H

#include <regex>

#include "Tree.h"
#include "fcgio.h"

class Generator {

private:

    static std::string ProcessVariable(Node* currentPage, Node* templatePage, const std::string& templateName, FCGX_Request* request,
                                const std::string& templatesPath, std::string variableName) {

        if (variableName[0] != '$')
            return "";

        variableName = variableName.substr(1);

        // PATH
        if (variableName == "@FULLPATH" || variableName == "FULLPATH" || variableName == "PATH"
            || variableName == "@PATH") {

            Node* p = templatePage;
            if (variableName[0] == '@')
                p = currentPage;

            std::string nodePath;
            if (variableName == "PATH" || variableName == "@PATH")
                nodePath = p->getKey();
            else
                nodePath = p->getPath();

            auto splitPath = Utils::Split(nodePath, '/');
            std::string resultPath;
            for (const auto &path : splitPath)
                resultPath.append(path.substr(0, path.find_first_of('.')) + '/');

            return resultPath;
        }

        // FCGI variables
        else if (request) {
#ifndef tests
            return FCGX_GetParam(variableName.c_str(), request->envp);
#endif
        }

        return "";

    }

public:

    static std::string Generate(Node* currentPage, Node* templatePage, const std::string& templateName, FCGX_Request* request,
                                const std::string& templatesPath) {

        Node* root = currentPage->getRoot();
        Node* templateNode = root->getFirst(templatesPath + '/' + templateName);

        if (!templateNode)
            return "";

        std::string t = templateNode->getValue();
        std::string result;

        static const std::regex tagRegex(R"(\<\!\-\-\s*(\w+)\((.*?)\)\s*\-\-\>)");
        std::smatch m;
        std::string function;
        std::vector<std::string> params;
        std::string::const_iterator searchStart(t.cbegin());
        unsigned long lastPartPos = 0;
        bool printing = true;

        while (regex_search(searchStart, t.cend(), m, tagRegex)) {
            function = m[1];
            params = Utils::Tokenize(m[2]);

            if (printing)
                result.append(t.substr(lastPartPos, m.position()));

            lastPartPos += m.position() + m.length();

            if (function == "print" && !params.empty() && printing) {

                if (params[0][0] == '$')
                    result.append(ProcessVariable(currentPage, templatePage, templateName, request,
                                                  templatesPath, params[0]));

                else {
                    // Node value or generated template

                    Node* p = templatePage;
                    if (params[0][0] == '@') {
                        params[0] = params[0].substr(1);
                        p = currentPage;
                    }
                    std::string subTemplateName;
                    if (params.size() > 1)
                        subTemplateName = params[1];

                    // Print multiple nodes
                    auto nodes = p->get(params[0]);

                    for (const auto &n : nodes) {

                        if (subTemplateName.empty()) {
                            // Try the template of the node
                            auto splitKey = Utils::Split(n->getKey(), '.');
                            if (splitKey.size() > 1)
                                subTemplateName = splitKey[1];
                            // Check the template of the node exists
                            if (!root->getFirst(subTemplateName))
                                subTemplateName = "";
                        }

                        if (!subTemplateName.empty())
                            result.append(Generate(currentPage, n, subTemplateName, nullptr, templatesPath));
                        else
                            result.append(n->getValue());

                    }
                }

            }

            else if (function == "template" && !params.empty() && printing) {

                auto pages = Utils::Split(currentPage->getPath(), '/');

                for (const auto &subTemplateName : params) {
                    std::string pagePath;
                    bool pageFound = false;
                    for (const auto &page : pages) {
                        pagePath += '/' + page;
                        auto pagePathSplit = Utils::Split(page, '.');
                        if (std::find(pagePathSplit.begin(), pagePathSplit.end(), subTemplateName) != pagePathSplit.end()) {
                            pageFound = true;
                            break;
                        }
                    }

                    if (pageFound) {
                        auto n = root->getFirst(pagePath);
                        std::string subTemplate = Generate(currentPage, n, subTemplateName, nullptr, templatesPath);

                        result.append(subTemplate);
                    }
                }

            }

            else if (function == "if" && params.size() == 2 && printing) {

                std::regex r(params[1]);

                std::string variable = params[0];
                std::string value;
                if (variable[0] == '$')
                    value = ProcessVariable(currentPage, templatePage, templateName, request,
                                            templatesPath, variable);
                else {
                    Node* p = templatePage;
                    if (variable[0] == '@') {
                        variable = variable.substr(1);
                        p = currentPage;
                    }
                    Node* n = p->getFirst(variable);
                    if (n)
                        value = n->getValue();
                }

                if (!std::regex_match(value, r))
                    printing = false;

            }

            else if (function == "endif") {
                printing = true;
            }

            else if (printing)
                result.append(m.str());

            searchStart = m.suffix().first;
        }

        if (printing)
            result.append(t.substr(lastPartPos));

        return result;

    }

};


#endif //FASTCGI_BLOG_GENERATOR_H
