#include <iostream>
#include <libwebsockets.h>
#include <Eigen/Dense>
#include <thread>
#include <mutex>
#include <windows.h>
#include <winsock2.h>
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
#include <iostream>

namespace asio = boost::asio;
namespace http = boost::beast::http;

class Session : public std::enable_shared_from_this<Session>
{
public:
  Session(asio::ip::tcp::socket &&socket)
      : stream_(std::move(socket))
  {
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
    processRequest();
  }

  void processRequest()
  {
    // Obtém o valor do campo 'user' da solicitação
    // TA ERRADO, ACHAR COMO PEGAR E SALVAR NO REQUEST
    // auto jsonBody = nlohmann::json::parse(request_.body());

    // for (auto it = jsonBody.begin(); it != jsonBody.end(); it++)
    // {
    //   cout << it.key() << endl;
    // }
    // long userValue = request_.body()[1];
    // Cria um objeto JSON com os dados desejados
    string str;
    for (const auto &part : request_.body())
    {
      str += part;
    }
    try
    {
      // Criando um objeto JSON a partir da string
      nlohmann::json j = nlohmann::json::parse(str);
      std::vector<string> valores = j["vector"].get<std::vector<string>>();

      // Convertendo o std::vector para Eigen::VectorXd
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

      std::cout << eigenVector
                << "Objeto JSON criado a partir da string:\n";
      // std::cout << j.dump(4) << std::endl; // Saída formatada do JSON
      ModlMat h1;
      if (j["model"] == false)
      {
        nlohmann::json responseData;
        h1.loadMat(*h1.getMat(), "C:\\Users\\Cetaphil\\Desktop\\ultrassom\\DIS-Servidor\\utils\\MatrizesRef\\H-1.csv");
        ConjugateGradientNE cgne(*h1.getMat(), eigenVector);
        auto [f, i] = cgne.solve();
        responseData["bitMapVector"] = f;
        responseData["iteracao"] = i;
        responseData["user"] = j["user"];
        std::string responseBody = responseData.dump();

        response_.version(request_.version());
        response_.keep_alive(false);

        // Habilita o CORS para todas as origens
        response_.set(http::field::access_control_allow_origin, "*");
        response_.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS, PUT, DELETE");
        response_.set(http::field::access_control_allow_headers, "content-type");

        response_.result(http::status::ok);

        // Configura o corpo da resposta com a string JSON
        response_.body() = responseBody;

        response_.prepare_payload();

        writeResponse();
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << "Erro ao analisar a string JSON: " << e.what() << std::endl;
    }
    nlohmann::json responseData = {
        {"bitMapVector", {1, 2, 3}}, // Substitua pelos seus dados reais
        {"user", 5},
        {"iteracoes", 10},
        {"tempo", 5.0}};

    // Converte o objeto JSON para uma string
    std::string responseBody = responseData.dump();

    response_.version(request_.version());
    response_.keep_alive(false);

    // Habilita o CORS para todas as origens
    response_.set(http::field::access_control_allow_origin, "*");
    response_.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS, PUT, DELETE");
    response_.set(http::field::access_control_allow_headers, "content-type");

    response_.result(http::status::ok);

    // Configura o corpo da resposta com a string JSON
    response_.body() = responseBody;

    response_.prepare_payload();

    writeResponse();
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