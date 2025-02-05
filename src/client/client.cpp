#include <getopt.h>
#include <csignal>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <zmq.hpp>
#include <nlohmann/json.hpp>

namespace {
    // static variable used to sopt the main loop
    volatile std::atomic_bool isRunning{true};
}

void signal_handler(int signal) {
    std::cout << "Signal " << signal << std::endl;
    isRunning = false;
}

// get time since monitor start in second
inline uint64_t simpleTimeSec(float timeMicro) {
    return static_cast<uint64_t>(std::trunc(timeMicro/1000000));
}

// reset/create new csv for RAM with header
void initRamCsv() {
    std::ofstream ramCsv{"RAM.csv", std::ios_base::trunc};
    if (ramCsv.is_open()) {
        ramCsv << "Time(us)" << ";";
        ramCsv << "MemTotal" << ";";
        ramCsv << "MemFree" << ";";
        ramCsv << "MemAvailable" << ";";
        ramCsv << "Buffers" << ";";
        ramCsv << "Cached" << ";";
        ramCsv << "SwapCached" << ";";
        ramCsv << "Active" << ";";
        ramCsv << "Inactive" << ";";
        ramCsv << "SwapTotal" << ";";
        ramCsv << "SwapFree" << ";";
        ramCsv << std::endl;
    }
}

// add one raw in RAM csv file
void updateRamCsv(const nlohmann::json& json) {
    try {
        const nlohmann::json& jsonR = json["RAM"];
        std::ofstream ramCsv{"RAM.csv", std::ios_base::app};
        if (ramCsv.is_open()) {
            ramCsv << simpleTimeSec(jsonR["timeMicro"]) << ";";
            ramCsv << jsonR["MemTotal"]["val"] << ";";
            ramCsv << jsonR["MemFree"]["val"] << ";";
            ramCsv << jsonR["MemAvailable"]["val"] << ";";
            ramCsv << jsonR["Buffers"]["val"] << ";";
            ramCsv << jsonR["Cached"]["val"] << ";";
            ramCsv << jsonR["SwapCached"]["val"] << ";";
            ramCsv << jsonR["Active"]["val"] << ";";
            ramCsv << jsonR["Inactive"]["val"] << ";";
            ramCsv << jsonR["SwapTotal"]["val"] << ";";
            ramCsv << jsonR["SwapFree"]["val"] << ";";
            ramCsv << std::endl;
        }
    }
    catch(const nlohmann::json::exception& e) {
        std::cout << "Json Exception:" << e.what() << std::endl;
    }
}

// reset/create new csv for CPU with header
void initCpuCsv() {
    std::ofstream cpuCsv{"CPU.csv", std::ios_base::trunc};
    if (cpuCsv.is_open()) {
        cpuCsv << "Time(us)" << ";";
        cpuCsv << "user" << ";";
        cpuCsv << "nice" << ";";
        cpuCsv << "system" << ";";
        cpuCsv << "idle" << ";";
        cpuCsv << "iowait" << ";";
        cpuCsv << "irq" << ";";
        cpuCsv << "softirq" << ";";
        cpuCsv << "steal" << ";";
        cpuCsv << "guest" << ";";
        cpuCsv << "guest_nice" << ";";
        cpuCsv << std::endl;
    }
}

// add one raw in CPU csv file
void updateCpuCsv(const nlohmann::json& json) {
    try {
        const nlohmann::json& jsonC = json["CPU"]["Global"];
        std::ofstream cpuCsv{"CPU.csv", std::ios_base::app};
        if (cpuCsv.is_open()) {
            cpuCsv << simpleTimeSec(json["CPU"]["timeMicro"]) << ";";
            cpuCsv << std::fixed << std::setprecision(1);
            cpuCsv << float(jsonC["user"]) << ";";
            cpuCsv << float(jsonC["nice"]) << ";";
            cpuCsv << float(jsonC["system"]) << ";";
            cpuCsv << float(jsonC["idle"]) << ";";
            cpuCsv << float(jsonC["iowait"]) << ";";
            cpuCsv << float(jsonC["irq"]) << ";";
            cpuCsv << float(jsonC["softirq"]) << ";";
            cpuCsv << float(jsonC["steal"]) << ";";
            cpuCsv << float(jsonC["guest"]) << ";";
            cpuCsv << float(jsonC["guest_nice"]) << ";";
            cpuCsv << std::endl;
        }
    }
    catch(const nlohmann::json::exception& e) {
        std::cout << "Json Exception:" << e.what() << std::endl;
    }
}

// display in stdout global CPU information
void logCpu(const nlohmann::json& json) {
    try {
        const nlohmann::json& jsonC = json["CPU"]["Global"];
        uint64_t nbCpu = json["CPU"].size() - 3;
        // simplify less important values in one
        float otherCpu{};
        if (jsonC.contains("nice"))
            otherCpu += static_cast<float>(jsonC["nice"]);
        if (jsonC.contains("iowait"))
             otherCpu += static_cast<float>(jsonC["iowait"]);
        if (jsonC.contains("irq"))
             otherCpu += static_cast<float>(jsonC["irq"]);
        if (jsonC.contains("softirq"))
             otherCpu += static_cast<float>(jsonC["softirq"]);
        if (jsonC.contains("steal"))
             otherCpu += static_cast<float>(jsonC["steal"]);
        if (jsonC.contains("guest"))
             otherCpu += static_cast<float>(jsonC["guest"]);
        if (jsonC.contains("guest_nice"))
             otherCpu += static_cast<float>(jsonC["guest_nice"]);

        std::cout << simpleTimeSec(json["CPU"]["timeMicro"]) << "s -> CPU:";
        std::cout.precision(2);
        std::cout << " idle(" << float(jsonC["idle"])/float(nbCpu) << "%) + (";
        std::cout << float(jsonC["user"])/float(nbCpu) << "% / ";
        std::cout << float(jsonC["system"])/float(nbCpu) << "% / ";
        std::cout << otherCpu/static_cast<float>(nbCpu) << "%) (user / system / others)";
        std::cout << std::endl;
    }
    catch(const nlohmann::json::exception& e) {
        std::cout << "Json Exception:" << e.what() << std::endl;
    }
}

// convert value as percent of total
inline float toPercent(float val, float total) {
    return val*100/total;
}

// display in stdout global RAM information
void logRam(const nlohmann::json& json) {
    try {
        const nlohmann::json& jsonR = json["RAM"];
        float total = static_cast<float>(jsonR["MemTotal"]["val"]);

        std::cout << simpleTimeSec(jsonR["timeMicro"]) << "s -> RAM:";
        std::cout << " Total:" << jsonR["MemTotal"]["val"] << jsonR["MemTotal"]["unit"].get<nlohmann::json::string_t>();
        std::cout.precision(3);
        std::cout << " Available:"<< toPercent(jsonR["MemAvailable"]["val"], total) << "% (";
        std::cout << "Free:"<< toPercent(jsonR["MemFree"]["val"], total) << "% / ";
        std::cout << "Buffers:"<< toPercent(jsonR["Buffers"]["val"], total) << "% / ";
        std::cout << "Cached:"<< toPercent(jsonR["Cached"]["val"], total) << "%)";
        std::cout << std::endl;
    }
    catch(const nlohmann::json::exception& e) {
        std::cout << "Json Exception:" << e.what() << std::endl;
    }
}

int main(int argc, char **argv) {
    // Signal used to stop client
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string ip = "127.0.0.1";
    std::string port = "4444";

    const struct option long_options[] = {
        {"ip", required_argument, nullptr, 'i'},
        {"port", required_argument, nullptr, 'p'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "i:p:h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'i':
                ip = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'h':
                std::cout << "Usage : " << argv[0] << " [OPTIONS]\n"
                          << "  -i, --ip    Select IP address (default: 127.0.0.1)\n"
                          << "  -p, --port  Select port (default: 4444)\n"
                          << "  -h, --help  Display help\n";
                return 0;
            default:
                std::cout << "Error in getopt!" << std::endl;
                return 1;
        }
    }

    // Init zmq
    std::cout << "Init Zmq Subscriber on " << ip << ":" << port << std::endl;
    zmq::context_t ctx;
    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    try {
        subscriber.connect("tcp://" + ip + ":" + port);
    }
    catch (const zmq::error_t& e) {
        std::cout << "Failed to open subscriber on ip:" << ip << " port:" << port << std::endl;
        std::cout << "Error:" << e.what() << std::endl;
        return 1;
    }

    subscriber.set(zmq::sockopt::subscribe, "");

    // init CSV files with header
    initCpuCsv();
    initRamCsv();

    // periodic main loop
    std::cout << "Start main loop" << std::endl;

    while (isRunning) {
        zmq::message_t msg;
        // check for new message received (dontwait used to be able to receive signal)
        auto res = subscriber.recv(msg, zmq::recv_flags::dontwait);
        if (res.has_value()) {
            /*std::cout << "Receive message:" << msg.to_string() << ", size:"<< res.value() << std::endl;*/

            // parse json and log/store RAM and CPU data
            const nlohmann::json json = nlohmann::json::parse(msg.to_string());
            if (json.contains("RAM")) {
                logRam(json);
                updateRamCsv(json);
            }
            if (json.contains("CPU")) {
                logCpu(json);
                updateCpuCsv(json);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // exit server
    std::cout << "Exit Client" << std::endl;

    return 0;
}


