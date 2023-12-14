#include "server.hpp"

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