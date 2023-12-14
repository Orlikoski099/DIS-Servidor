#include "Monitor.hpp"
#include "Session.hpp"
#include "Server.hpp"

namespace asio = boost::asio;
namespace http = boost::beast::http;



int main()
{
  // std::thread resources (MonitorCPU);
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

  // resources.join();

  return 0;
}
