#include <fstream>
#include <iostream>
#include <utility>
#include "monitor.h"

Monitor::Monitor() {
    // get initialisation time for compute period
    startTime = std::chrono::steady_clock::now();
    lastCpuTime = startTime;

    // get system cpu tick per second
    scClkTck = sysconf(_SC_CLK_TCK);
    if (scClkTck <= 0) {
        std::cout << "Error, failed to read _SC_CLK_TCK" << std::endl;
        scClkTck = 100;
    }

    // first measure of cpu in order to compute diff after
    lastCpuValues = dumpStat();
}

// compute elapsed time since last measure
inline int64_t Monitor::getElapsedMicro() {
    return std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::steady_clock::now() - startTime).count();
}

// Read CPU values form /proc/stat file
std::vector<std::vector<uint64_t>> Monitor::dumpStat() {
    std::vector<std::vector<uint64_t>> cpuValues{};

    // check man proc_stat
    std::ifstream stat{"/proc/stat"};
    if (!stat) {
        std::cout << "ERROR, failed to open stat" << std::endl;
        return cpuValues;
    }

    std::string line;
    // iterate over all line. One global line and one line per core
    while (std::getline(stat, line)) {
        std::vector<uint64_t> singleCpuValues;

        std::istringstream ss(line);
        std::string label;
        ss >> label;

        // ignore line that missmatch
        if (label.rfind("cpu", 0) != 0) {
            continue;
        }

        // store all the line values
        uint64_t value;
        while ( ss >> value ) {
            singleCpuValues.push_back(value);
        }

        // feed main vector
        cpuValues.push_back(singleCpuValues);
    }

    return cpuValues;  // NRVO should be ok
}

// read RAM values from /proc/meminfo and store it in json
nlohmann::json Monitor::getRAM() {
    nlohmann::json ramJson;
    nlohmann::json& json = ramJson["RAM"];
    json["timeMicro"] = getElapsedMicro();

    // check man proc_meminfo
    std::ifstream meminfo{"/proc/meminfo"};
    if (!meminfo) {
        std::cout << "ERROR, failed to open meminfo" << std::endl;
        return "{ \"RAM\": \"Open meminfo failed\" }"_json;
    }

    std::string line;
    while (std::getline(meminfo, line)) {
        std::string id{};
        uint64_t size{};
        std::string unit{};

        std::istringstream ss(line);
        ss >> id >> size >> unit;
        // remove ':' char
        id.pop_back();

        // check for wanted label only
        if (std::find(std::begin(listRamLabel), std::end(listRamLabel), id) !=
                std::end(listRamLabel)) {
            json[id]["val"] = size;
            json[id]["unit"] = unit;
        }
    }

    return ramJson;
}

// compute CPU diff value in percent and store them in json
nlohmann::json Monitor::getCPU() {
    nlohmann::json cpuJson;
    nlohmann::json& json = cpuJson["CPU"];

    json["timeMicro"] = getElapsedMicro();

    // cpu slot duration used to compute cpu load in percent from tick number
    int64_t slotDurationMicro = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - lastCpuTime).count();
    json["slotDurationMicro"] = slotDurationMicro;

    lastCpuTime = std::chrono::steady_clock::now();

    // get the new cpu tick values
    auto newCpuValues = dumpStat();

    // compute difference between tick and then cpu load
    if (newCpuValues.size() == lastCpuValues.size()) {
        for (uint64_t i = 0; i < newCpuValues.size(); ++i) {
            // first line is global cpu, nex line are for each core
            std::string cpuName;
            if (i == 0) {
                cpuName = "Global";
            } else {
                cpuName = std::to_string(i-1);
            }

            if (newCpuValues[i].size() == lastCpuValues[i].size()) {
                for (uint64_t j = 0; j < newCpuValues[i].size(); ++j) {
                    // Compute cpu percent from ticks
                    // [ (Tick Period Diff) * 100 ] / [ (Tick Per Sec) * (Period Sec) ]
                    float val = (
                        static_cast<float>(newCpuValues[i][j] - lastCpuValues[i][j]) * 100
                        /
                        (static_cast<float>((uint64_t)scClkTck * (uint64_t)slotDurationMicro) / 1000000));
                    json[cpuName][listCpuLabel.at(j)] = val;
                }
            } else {
                std::cout << "ERROR, CPU Values size mismatch !" << std::endl;
            }
        }
    } else {
        std::cout << "ERROR, CPU Values size mismatch !" << std::endl;
    }

    // replace last cpu value for next iteration
    lastCpuValues = std::move(newCpuValues);

    return cpuJson;
}

