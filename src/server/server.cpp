#include <thread>
#include <csignal>
#include <iostream>
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

int main() {
    // Signal used to stop server
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Init zmq
    std::cout << "Init Zmq Publish" << std::endl;
    zmq::context_t ctx;
    zmq::socket_t publisher(ctx, zmq::socket_type::pub);
    publisher.bind("tcp://127.0.0.1:4444");

    // main loop with periodic publish
    std::cout << "Start main loop" << std::endl;
    auto nextLoopTime = std::chrono::steady_clock::now();

    // create monitor
    Monitor monitor;
    // ram period bigger than cpu
    uint8_t ramCpt{0};

    while (isRunning) {
        // get cpu stat in json and publish it(2s period)
        nlohmann::json cpuJson = monitor.getCPU();
        publish_msg(publisher, cpuJson);

        // get ram stat in json and publish it (10s period)
        if (ramCpt >= 5) {
            nlohmann::json ramJson = monitor.getRAM();
            publish_msg(publisher, ramJson);
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
