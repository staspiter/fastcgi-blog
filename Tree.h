//
// Created by Stas Piter on 25.02.2021.
//

#ifndef FASTCGI_BLOG_TREE_H
#define FASTCGI_BLOG_TREE_H

#include <vector>
#include <tuple>
#include <string>
#include <filesystem>

#include "Utils.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

class Node {

private:

    std::vector<std::tuple<std::string, Node*>> sub;
    Node* parent;
    std::string value;

public:

    Node(const std::string& value = "", Node* parent = nullptr) {
        this->value = value;
        this->parent = parent;
    }

    ~Node() {
        for (const auto &n : sub)
            delete std::get<1>(n);
    }

    void buildFromJson(rapidjson::Value& json) {

        if (json.IsObject()) {
            for (auto& [key, value] : json.GetObject()) {
                Node* n = new Node("", this);
                sub.emplace_back(key.GetString(), n);
                n->buildFromJson(value);
            }
        }

        else if (json.IsArray()) {
            int i = 0;
            for (auto& item: json.GetArray()) {
                Node* n = new Node("", this);
                sub.emplace_back(std::to_string(i), n);
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

                Node* n = new Node("", this);
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

    Node* get(std::vector<std::string>& pathVector, std::vector<std::string>* pages = nullptr) {

        if (pathVector.empty())
            return this;

        std::string p = pathVector[0];
        pathVector.erase(pathVector.begin());

        Node* nextNode = nullptr;

        if (p.empty())
            nextNode = getRoot();

        else {
            for (const auto &n : sub) {
                auto applicablePage = Utils::IsPageApplicable(p, std::get<0>(n));
                if (!applicablePage.empty()) {
                    if (pages)
                        pages->push_back(applicablePage);
                    nextNode = std::get<1>(n);
                    break;
                }
            }
        }

        if (!nextNode)
            return nullptr;

        return nextNode->get(pathVector, pages);

    }

    Node* get(const std::string& path, std::vector<std::string>* pages = nullptr) {
        auto v = Utils::Split(path, '/');
        return get(v, pages);
    }

    Node* getRoot() {
        if (parent == nullptr)
            return this;
        return parent->getRoot();
    }

    std::string getValue() {
        return value;
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
