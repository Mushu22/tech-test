#ifndef SRC_MONITOR_MONITOR_H_
#define SRC_MONITOR_MONITOR_H_

#include <chrono>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class Monitor {
 public:
    Monitor();
    nlohmann::json getRAM();
    nlohmann::json getCPU();

 private:
    // Time start from monitor init
    std::chrono::time_point<std::chrono::steady_clock> startTime;

    // Used to convert cpu ticks to percent consuption
    int64_t scClkTck;  // clock ticks per second
    std::chrono::time_point<std::chrono::steady_clock> lastCpuTime;
    std::vector<std::vector<uint64_t>> lastCpuValues;
    // Internal function used to read /proc/stat file
    // Double vector used because cpu core number and label number not fix
    std::vector<std::vector<uint64_t>> dumpStat();

    // Labes used
    std::vector<std::string> listRamLabel{
        "MemTotal",
        "MemFree",
        "MemAvailable",
        "Buffers",
        "Cached",
        "SwapCached",
        "Active",
        "Inactive",
        "SwapTotal",
        "SwapFree"};
    static constexpr std::array<std::string, 11> listCpuLabel{{
        "user",
        "nice",
        "system",
        "idle",
        "iowait",
        "irq",
        "softirq",
        "steal",
        "guest",
        "guest_nice",
        "UNKNOWN"}};

    // get duration between two measures
    inline int64_t getElapsedMicro();
};

#endif  // SRC_MONITOR_MONITOR_H_
