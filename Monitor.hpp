// Monitor.hpp
#pragma once

#include "index.hpp"

class Monitor {
public:
    static void MonitorCPU();
    static double GetCPULoad();
    static MEMORYSTATUSEX GetRAMUsage();
};
