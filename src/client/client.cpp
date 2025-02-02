#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>
#include <zmq.hpp>

namespace {
    //static variable used to sopt the main loop
    volatile std::atomic_bool isRunning{true};
}

void signal_handler(int signal) {
    std::cout << "Signal " << signal << std::endl;
    isRunning = false;
}

int main() {
    // Signal used to stop server
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Init zmq
    std::cout << "Init Zmq Subscribe" << std::endl;
    zmq::context_t ctx;
    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://127.0.0.1:4444");
    subscriber.set(zmq::sockopt::subscribe, "");

    // periodic main loop
    std::cout << "Start main loop" << std::endl;

    while (isRunning) {
        zmq::message_t msg;
        auto res = subscriber.recv(msg, zmq::recv_flags::dontwait);
        if (res.has_value()) {
            std::cout << "Receive message:" << msg.to_string() << ", size:"<< res.value() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // exit server
    std::cout << "Exit Client" << std::endl;

    return 0;
}


