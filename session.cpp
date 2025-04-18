#include "session.hpp"

using namespace ImageGeneration;

Session::Session(asio::ip::tcp::socket &&socket)
    : stream_(std::move(socket))
{
}
void Session::enqueueRequest(http::request<http::string_body> request)
{
  requests_queue_.push(request);
  if (requests_queue_.size() == 1)
  {
    processRequest();
  }
}
void Session::start()
{
  readRequest();
}

void Session::readRequest()
{
  auto self = shared_from_this();
  http::async_read(stream_, buffer_, request_,
                   [self](boost::system::error_code ec, std::size_t bytes_transferred)
                   {
                     self->onRead(ec, bytes_transferred);
                   });
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

void Session::processRequest()
{
  clock_t start, end;
  start = clock() / CLOCKS_PER_SEC;
  if (requests_queue_.empty())
  {
    return;
  }
  http::request<http::string_body> current_request = requests_queue_.front();
  requests_queue_.pop();

  auto self = shared_from_this();

  std::string target(self->request_.target().data(), self->request_.target().size());
  std::string responseBody;
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
    makeImage(f, std::to_string(j["user"].get<int>()));
    end = clock() / CLOCKS_PER_SEC;
    nlohmann::json responseData = {
        {"imageVector", ImgVector(f)},
        {"user", j["user"]},
        {"iteracoes", i},
        {"startTime", start},
        {"endTime", end}};
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

  if (requests_queue_.empty())
    stream_.shutdown(asio::ip::tcp::socket::shutdown_send);
  processRequest();
}