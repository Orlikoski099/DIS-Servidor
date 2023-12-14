#include "Session.hpp"
#include "ImageGeneration.hpp"
#include "ThreadPool.hpp"

Session::Session(asio::ip::tcp::socket &&socket, ThreadPool &threadPool)
    : stream_(std::move(socket)), threadPool_(threadPool) {}

void Session::enqueueRequest(http::request<http::string_body> request)
{
  auto self = shared_from_this();
  threadPool_.enqueue([self, request]()
                      { self->processRequest(request); });
}

void Session::start()
{
  readRequest();
}

void Session::readRequest()
{
  auto self = shared_from_this();
  http::async_read(stream_, buffer_, request_, [self](boost::system::error_code ec, std::size_t bytes_transferred)
                   { self->onRead(ec, bytes_transferred); });
}

void Session::onRead(boost::system::error_code ec, std::size_t bytes_transferred)
{
  if (ec)
  {
    std::cerr << "Error reading request: " << ec.message() << std::endl;
    return;
  }
  auto self = shared_from_this();
  self->enqueueRequest(request_);
}

void Session::processRequest(http::request<http::string_body> current_request)
{
  // Implementação do método processRequest
  // ...

  response_.prepare_payload();

  writeResponse();
}

void Session::writeResponse()
{
  auto self = shared_from_this();
  http::async_write(stream_, response_,
                    [self](boost::system::error_code ec, std::size_t bytes_transferred)
                    {
                      self->onWrite(ec, bytes_transferred);
                    });
}

void Session::onWrite(boost::system::error_code ec, std::size_t bytes_transferred)
{
  if (ec)
  {
    std::cerr << "Error writing response: " << ec.message() << std::endl;
    return;
  }
}
