//
// Created by Stas Piter on 24.02.2021.
//

#include <iostream>
#include <fstream>
#include <filesystem>

#include "fcgio.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "Tree.h"
#include "Generator.h"

int main() {

    // Load config

    std::string dir = std::filesystem::current_path().string();
    std::string templatesPath = "/";
    std::string templateHome = "home";
    std::string template404 = "404";

    if (std::filesystem::exists("config.json")) {
        std::ifstream t("config.json");
        std::string configJsonStr((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        rapidjson::Document json;
        rapidjson::ParseResult result = json.Parse(configJsonStr.c_str());
        if (result) {
            dir = json.GetObject().FindMember("dir")->value.GetString();
            templatesPath = json.GetObject().FindMember("templatesPath")->value.GetString();
            templateHome = json.GetObject().FindMember("templateHome")->value.GetString();
            template404 = json.GetObject().FindMember("template404")->value.GetString();
        }
    }

    // Load tree

    Tree t(std::filesystem::current_path().string() + dir);
    t.build();

    std::streambuf * cin_streambuf  = std::cin.rdbuf();
    std::streambuf * cout_streambuf = std::cout.rdbuf();
    std::streambuf * cerr_streambuf = std::cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0) {
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        std::cin.rdbuf(&cin_fcgi_streambuf);
        std::cout.rdbuf(&cout_fcgi_streambuf);
        std::cerr.rdbuf(&cerr_fcgi_streambuf);

        std::string uri = FCGX_GetParam("REQUEST_URI", request.envp);

        std::string currentTemplate = templateHome;
        std::string status = "Status: 200 OK";
        auto n = t.getRoot()->getFirst(uri);
        if (!n) {
            n = t.getRoot();
            currentTemplate = template404;
            status = "Status: 404 Not Found";
        }

        std::string result = Generator::Generate(n, t.getRoot(), currentTemplate, &request, templatesPath);
        std::cout << status << "\r\n"
                  << "Content-type: text/html\r\n"
                  << "\r\n"
                  << result;
    }

    std::cin.rdbuf(cin_streambuf);
    std::cout.rdbuf(cout_streambuf);
    std::cerr.rdbuf(cerr_streambuf);

    return 0;
}
