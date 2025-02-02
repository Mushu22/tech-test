#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <csignal>
#include <iostream>
#include <zmq.hpp>

namespace {
    //static variable used to sopt the main loop
    volatile std::atomic_bool isRunning{true};
}

void signal_handler(int signal) {
    std::cout << "Signal " << signal << std::endl;
    isRunning = false;
}

// Build and publish msg
void publish_msg(zmq::socket_ref pub, const uint32_t cpt) {
    std::string strData{"Hello:" + std::to_string(cpt)};
    zmq::message_t msg(strData.begin(), strData.end());

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
    uint32_t cpt{0};

    while (isRunning) {
        publish_msg(publisher, cpt);
        cpt += 1;

        // compute next loop timing and wait
        nextLoopTime = nextLoopTime + std::chrono::milliseconds(2000);
        std::this_thread::sleep_until(nextLoopTime);
    }

    // exit server
    std::cout << "Exit Server" << std::endl;

    return 0;
}
