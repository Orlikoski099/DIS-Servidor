#include "server.hpp"

Server::Server(asio::io_context &io_context, short port)
    : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      socket_(io_context)
{
  accept();
}

void Server::accept()
{
  acceptor_.async_accept(socket_,
                         [this](boost::system::error_code ec)
                         {
                           if (!ec)
                           {
                             std::make_shared<Session>(std::move(socket_))->start();
                           }
                           accept();
                         });
}
