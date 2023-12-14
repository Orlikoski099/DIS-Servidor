// Session.hpp
#pragma once

#include "index.hpp"

namespace asio = boost::asio;
namespace http = boost::beast::http;

class Session : public std::enable_shared_from_this<Session>
{
public:
  Session(asio::ip::tcp::socket &&socket, ThreadPool &threadPool);
  void enqueueRequest(http::request<http::string_body> request);
  void start();

private:
  ThreadPool &threadPool_;
  void readRequest();
  void onRead(boost::system::error_code ec, std::size_t bytes_transferred);
  void processRequest(http::request<http::string_body> current_request);
  void writeResponse();
  void onWrite(boost::system::error_code ec, std::size_t bytes_transferred);

private:
  asio::streambuf buffer_;
  asio::ip::tcp::socket stream_;
  http::request<http::string_body> request_;
  http::response<http::string_body> response_;
  std::queue<http::request<http::string_body>> requests_queue_;
};