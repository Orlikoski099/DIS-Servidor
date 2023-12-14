#include "server.hpp"
#include "Monitor.hpp"

double GetCPULoad()
{
  PDH_HQUERY cpuQuery;
  PDH_HCOUNTER cpuTotal;

  PdhOpenQuery(nullptr, NULL, &cpuQuery);
  PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
  PdhCollectQueryData(cpuQuery);
  Sleep(10000);
  PdhCollectQueryData(cpuQuery);

  PDH_FMT_COUNTERVALUE counterVal;
  PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

  std::cout << "Carga da CPU: " << counterVal.doubleValue << "%" << std::endl;

  PdhCloseQuery(cpuQuery);

  return 0;
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