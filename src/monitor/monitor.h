#ifndef SRC_MONITOR_MONITOR_H_
#define SRC_MONITOR_MONITOR_H_

#include <chrono>
#include <string>
#include <string_view>
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

    // Internal function used to read /proc/stat file
    std::vector<std::vector<uint64_t>> dumpStat();
    // Double vector used because cpu core count and number of labels not const
    std::vector<std::vector<uint64_t>> lastCpuValues;

    // Label used
    // RAM label not const because we can add/remove elements for configration
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
    // CPU labes const because list is exhaustive
    static constexpr std::array<std::string_view, 11> listCpuLabel = {{
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

    // get duration between two measurements
    inline int64_t getElapsedMicro();
};

#endif  // SRC_MONITOR_MONITOR_H_
