// Server.hpp
#pragma once

#include "index.hpp"

namespace asio = boost::asio;
namespace http = boost::beast::http;

class Server {
public:
    Server(asio::io_context &io_context, short port);

private:
    void accept();

    asio::ip::tcp::acceptor acceptor_;
    asio::ip::tcp::socket socket_;
    ThreadPool threadPool_;
};
