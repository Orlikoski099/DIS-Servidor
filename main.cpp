#include <iostream>
#include <Eigen/Dense>
#include <future>
#include <mutex>
#include <chrono>
#include <winsock2.h>
#include <windows.h>
#include <pdh.h>
#include "CGNE.hpp"
#include "CGNR.hpp"
#include "matrix.hpp"
#include "ImageGeneration.hpp"
#include "json.hpp"
#define BOOST_ASIO_DISABLE_IOCP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/value.hpp>
#include <boost/json.hpp>

namespace asio = boost::asio;
namespace http = boost::beast::http;

double GetCPULoad()
{
  PDH_HQUERY query;
  PDH_HCOUNTER counter;
  PDH_FMT_COUNTERVALUE value;

  PdhOpenQuery(nullptr, 0, &query);
  PdhAddCounter(query, "\\Processor(_Total)\\% Processor Time", 0, &counter);
  PdhCollectQueryData(query);
  PdhCollectQueryData(query);
  PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);

  PdhCloseQuery(query);

  return value.doubleValue;
}

// Função para obter o uso da RAM
MEMORYSTATUSEX GetRAMUsage()
{
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(memInfo);
  GlobalMemoryStatusEx(&memInfo);
  return memInfo;
}

void MonitorCPU()
{
  while (true)
  {
    double cpuUsage = GetCPULoad();
    std::cout << "CPU Usage: " << cpuUsage << "%" << std::endl;
    MEMORYSTATUSEX ramUsage = GetRAMUsage();
    std::cout << "RAM Used: " << (ramUsage.ullTotalPhys - ramUsage.ullAvailPhys) / (1024 * 1024) << " MB" << std::endl;
    Sleep(10000);
  }
}
class Session : public std::enable_shared_from_this<Session>
{
public:
  Session(asio::ip::tcp::socket &&socket)
      : stream_(std::move(socket))
  {
  }
  void enqueueRequest(http::request<http::string_body> request)
  {
    requests_queue_.push(request);
    if (requests_queue_.size() == 1)
    {
      processRequest();
    }
  }
  void start()
  {
    readRequest();
  }

private:
  void readRequest()
  {
    auto self = shared_from_this();
    http::async_read(stream_, buffer_, request_,
                     [self](boost::system::error_code ec, std::size_t bytes_transferred)
                     {
                       self->onRead(ec, bytes_transferred);
                     });
  }

  void onRead(boost::system::error_code ec, std::size_t bytes_transferred)
  {
    if (ec)
    {
      std::cerr << "Error reading request: " << ec.message() << std::endl;
      return;
    }
    auto self = shared_from_this();
    self->enqueueRequest(request_);
  }

  void processRequest()
  {
    // std::future<void> cpuRamThread = std::async(std::launch::async, MonitorCPU);
    if (requests_queue_.empty())
    {
      return;
    }
    http::request<http::string_body> current_request = requests_queue_.front();
    requests_queue_.pop();

    auto self = shared_from_this();

    std::string target(self->request_.target().data(), self->request_.target().size());
    std::string responseBody;

    auto start = std::chrono::steady_clock::now();
    string str;
    for (const auto &part : request_.body())
    {
      str += part;
    }
    try
    {
      nlohmann::json j = nlohmann::json::parse(str);
      std::vector<string> valores = j["vector"].get<std::vector<string>>();
      vector<double> valoresDouble;
      for (const auto &valString : valores)
      {
        try
        {
          double valDouble = std::stod(valString);
          valoresDouble.push_back(valDouble);
        }
        catch (const std::exception &e)
        {
          std::cerr << "Erro ao converter a string para double: " << e.what() << std::endl;
        }
      }
      Eigen::Map<Eigen::VectorXd> eigenVector(valoresDouble.data(), valoresDouble.size());

      ModlMat h1;
      if (j["model"] == false)
      {
        h1.loadMat(*h1.getMat(), "utils\\MatrizesRef\\H-2.csv");
      }
      else
      {
        h1.loadMat(*h1.getMat(), "utils\\MatrizesRef\\H-1.csv");
      }
      if (j["ganho"] == true)
      {
        const int N = 64;
        int S = 436;
        if (j["model"] == 1)
        {
          S = 794;
        }
        for (int c = 0; c < S; ++c)
        {
          for (int l = 0; l < N; ++l)
          {
            double gamma = 100 + (1.0 / 20) * l * sqrt(static_cast<double>(l));
            eigenVector[l * c] *= gamma;
          }
        }
      }
      ConjugateGradienteNR cgnr(*h1.getMat(), eigenVector);
      auto [f, i] = cgnr.solve();
      ImageGeneration::makeImage(f, std::to_string(j["user"].get<int>()));
      auto end = std::chrono::steady_clock::now();
      nlohmann::json responseData = {
          {"bitMapVector", ImageGeneration::ImgVector(f)},
          {"user", j["user"]},
          {"iteracoes", i},
          {"tempo", 5.0}};
      responseBody = responseData.dump();
    }
    catch (const std::exception &e)
    {
      std::cerr << "Erro ao analisar a string JSON: " << e.what() << std::endl;
    }

    response_.version(request_.version());
    response_.keep_alive(false);

    response_.set(http::field::access_control_allow_origin, "*");
    response_.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS, PUT, DELETE, FETCH");
    response_.set(http::field::access_control_allow_headers, "content-type");

    response_.result(http::status::ok);

    response_.body() = std::move(responseBody);

    response_.prepare_payload();

    writeResponse();
    processRequest();
  }

  void writeResponse()
  {
    auto self = shared_from_this();
    http::async_write(stream_, response_,
                      [self](boost::system::error_code ec, std::size_t bytes_transferred)
                      {
                        self->onWrite(ec, bytes_transferred);
                      });
  }

  void onWrite(boost::system::error_code ec, std::size_t bytes_transferred)
  {
    if (ec)
    {
      std::cerr << "Error writing response: " << ec.message() << std::endl;
      return;
    }

    // Close the connection
    stream_.shutdown(asio::ip::tcp::socket::shutdown_send);
  }

private:
  asio::streambuf buffer_;
  asio::ip::tcp::socket stream_;
  http::request<http::string_body> request_;
  http::response<http::string_body> response_;
  std::queue<http::request<http::string_body>> requests_queue_;
};

class Server
{
public:
  Server(asio::io_context &io_context, short port)
      : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
        socket_(io_context)
  {
    accept();
  }

private:
  void accept()
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

private:
  asio::ip::tcp::acceptor acceptor_;
  asio::ip::tcp::socket socket_;
};

int main()
{
  try
  {
    asio::io_context io_context;
    Server server(io_context, 8080);
    io_context.run();
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}