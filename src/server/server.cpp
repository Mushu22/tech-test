#include <getopt.h>
#include <thread>
#include <csignal>
#include <iostream>
#include <atomic>
#include <string>
#include <zmq.hpp>
#include "../monitor/monitor.h"

namespace {
    // static variable used to sopt the main loop
    volatile std::atomic_bool isRunning{true};
}

void signal_handler(int signal) {
    std::cout << "Signal " << signal << std::endl;
    isRunning = false;
}

// Build and publish msg
void publish_msg(zmq::socket_ref pub, const nlohmann::json& json) {
    std::string strData = to_string(json);
    zmq::message_t msg(std::begin(strData), std::end(strData));

    // send simple message
    auto res = pub.send(msg, zmq::send_flags::none);
    if (!res.has_value()) {
        std::cout << "ERROR: send message failed!" << std::endl;
    }
}

int main(int argc, char **argv) {
    // Signal used to stop server
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
    std::cout << "Init Zmq Publisher on " << ip << ":" << port << std::endl;
    zmq::context_t ctx;
    zmq::socket_t publisher(ctx, zmq::socket_type::pub);
    try {
        publisher.bind("tcp://" + ip + ":" + port);
    }
    catch (const zmq::error_t& e) {
        std::cout << "Failed to open publisher on ip:" << ip << " port:" << port << std::endl;
        std::cout << "Error:" << e.what() << std::endl;
        return 1;
    }

    // main loop with periodic publish
    std::cout << "Start main loop" << std::endl;
    auto nextLoopTime = std::chrono::steady_clock::now();

    // create monitor
    Monitor monitor;
    // ram period bigger than cpu
    int ramCpt{0};

    while (isRunning) {
        // get cpu stat in json and publish it(2s period)
        publish_msg(publisher, monitor.getCPU());

        // get ram stat in json and publish it (10s period)
        if (ramCpt >= 5) {
            publish_msg(publisher, monitor.getRAM());
            ramCpt = 1;
        } else {
            ramCpt += 1;
        }

        // compute next loop timing and wait
        nextLoopTime = nextLoopTime + std::chrono::milliseconds(2000);
        std::this_thread::sleep_until(nextLoopTime);
    }

    // exit server
    std::cout << "Exit Server" << std::endl;

    return 0;
}
