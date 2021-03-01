//
// Created by Stas Piter on 25.02.2021.
//

#ifndef FASTCGI_BLOG_TREE_H
#define FASTCGI_BLOG_TREE_H

#include <vector>
#include <tuple>
#include <string>
#include <filesystem>
#include <regex>

#include "Utils.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

class Node {

private:

    std::vector<std::tuple<std::string, Node*>> sub;
    Node* parent;
    std::string key;
    std::string value;

public:

    Node(const std::string& value = "", const std::string& key = "", Node* parent = nullptr) {
        this->value = value;
        this->parent = parent;
        this->key = key;
    }

    ~Node() {
        for (const auto &n : sub)
            delete std::get<1>(n);
    }

    void buildFromJson(rapidjson::Value& json) {

        if (json.IsObject()) {
            for (auto& [key, value] : json.GetObject()) {
                auto keyStr = key.GetString();
                Node* n = new Node("", keyStr, this);
                sub.emplace_back(keyStr, n);
                n->buildFromJson(value);
            }
        }

        else if (json.IsArray()) {
            int i = 0;
            for (auto& item: json.GetArray()) {
                auto keyStr = std::to_string(i);
                Node* n = new Node("", keyStr, this);
                sub.emplace_back(keyStr, n);
                n->buildFromJson(item);
                i++;
            }

        }

        else if (json.IsString()) {
            value = json.GetString();
        }

        else if (json.IsInt()) {
            value = std::to_string(json.GetInt());
        }

        else if (json.IsDouble()) {
            value = std::to_string(json.GetDouble());
        }

        else if (json.IsInt64()) {
            value = std::to_string(json.IsInt64());
        }

        else if (json.IsBool()) {
            value = json.GetBool() ? "true" : "false";
        }

    }

    void build(const std::string& path) {

        if (std::filesystem::is_directory(path)) {
            // Iterate through the files and folders

            for (const auto & item : std::filesystem::directory_iterator(path)) {

                std::string itemStr = item.path().filename().c_str();
                if (itemStr.empty() || itemStr[0] == '.')
                    continue;

                // Process txt and json files only
                if (std::filesystem::is_character_file(item)) {
                    if (itemStr.find('.') == std::string::npos)
                        continue;
                    std::string ext = itemStr.substr(itemStr.find_last_of('.') + 1);
                    if (ext != "txt" && ext != "json")
                        continue;
                }

                Node* n = new Node("", itemStr, this);
                sub.emplace_back(itemStr, n);
                n->build(path + '/' + itemStr);

            }
            
        } else {
            // Load the file

            std::string ext = path.substr(path.find_last_of('.') + 1);

            if (ext == "txt" || ext == "json" || ext == "html") {

                std::ifstream t(path);
                std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

                if (ext == "json") {
                    // Parse json and build nodes

                    rapidjson::Document json;
                    rapidjson::ParseResult result = json.Parse(str.c_str());
                    if (result)
                        buildFromJson(json);

                }
                else
                    value = str;
            }

        }
    }

    std::vector<Node*> get(std::vector<std::string>& pathVector) {

        if (pathVector.empty())
            return {};

        std::vector<Node*> result;
        std::vector<Node*> nodes = {this};
        std::vector<Node*> newNodes;

        for (int i = 0; i < pathVector.size(); i++) {
            std::string& s = pathVector[i];

            if (s.empty())
                newNodes = {getRoot()};

            else {

                std::regex sRegex(s);

                for (const auto &n : nodes)
                    for (const auto &n1pair: n->sub) {
                        Node* n1 = std::get<1>(n1pair);

                        if (
                            // path = 'file.template.json', node = 'file.template.json'
                            (n1->key == s) ||

                            // path = 'file', node = 'file.template.json'
                            (n1->key.length() > s.length() && n1->key.compare(0, s.length(), s) == 0
                             && n1->key[s.length()] == '.') ||

                            // path = regular expression
                            std::regex_match(n1->key, sRegex)
                            )

                            newNodes.push_back(n1);
                    }
            }

            if (i == pathVector.size() - 1)
                for (const auto &n : newNodes)
                    result.push_back(n);

            nodes = newNodes;
            newNodes.clear();

        }

        return result;

    }

    std::vector<Node*> get(const std::string& path) {
        auto v = Utils::Split(path, '/');
        return get(v);
    }

    Node* getFirst(const std::string& path) {
        auto v = Utils::Split(path, '/');
        auto r = get(v);
        if (r.empty())
            return nullptr;
        return r[0];
    }

    Node* getRoot() {
        if (!parent)
            return this;
        return parent->getRoot();
    }

    const std::string& getValue() {
        return value;
    }

    const std::string& getKey() {
        return key;
    }

    std::string getPath() {
        if (parent)
            return parent->getPath() + '/' + key;
        return key;
    }

};

class Tree {

private:

    Node* root = nullptr;
    std::string path;

public:

    explicit Tree(const std::string& path) {
        this->path = path;
    }

    ~Tree() {
        delete root;
    }

    void build() {
        delete root;
        root = new Node();
        root->build(path);
    }

    Node* getRoot() {
        return root;
    }

};


#endif //FASTCGI_BLOG_TREE_H
