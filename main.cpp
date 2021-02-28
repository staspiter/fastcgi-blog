//
// Created by Stas Piter on 24.02.2021.
//

#include <iostream>
#include "fcgio.h"

int main() {

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

        std::cout << "Content-type: text/html\r\n"
             << "\r\n"
             << "<html>\n"
             << "  <head>\n"
             << "    <title>Hello, World!</title>\n"
             << "  </head>\n"
             << "  <body>\n"
             << "    <h1>Hello, World from FastCGI!</h1>\n"
             << "URI: " << FCGX_GetParam("REQUEST_URI", request.envp) << "\n"
             << "  </body>\n"
             << "</html>\n";
    }

    std::cin.rdbuf(cin_streambuf);
    std::cout.rdbuf(cout_streambuf);
    std::cerr.rdbuf(cerr_streambuf);

    return 0;
}
