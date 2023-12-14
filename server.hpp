#pragma once
#include "session.hpp"

class Server
{
public:
  Server(asio::io_context &io_context, short port);

private:
  void accept();

private:
  asio::ip::tcp::acceptor acceptor_;
  asio::ip::tcp::socket socket_;
};