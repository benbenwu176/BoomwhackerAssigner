// error_handler.cpp
#include "error_handler.hpp"

#include <iostream>
#include <cstdlib>
#include <csignal>
#include <string>

namespace {
    // Called on fatal POSIX‐style signals.
    void signal_handler(int sig) {
        const char* desc = nullptr;
        switch (sig) {
            case SIGABRT: desc = "SIGABRT (abort)"; break;
            case SIGFPE:  desc = "SIGFPE (floating-point exception)"; break;
            case SIGILL:  desc = "SIGILL (illegal instruction)"; break;
            case SIGINT:  desc = "SIGINT (interrupt)"; break;
            case SIGSEGV: desc = "SIGSEGV (segmentation fault)"; break;
            case SIGTERM: desc = "SIGTERM (termination request)"; break;
            default:      desc = "Unknown signal"; break;
        }
        std::cerr << "Error: Caught signal " << desc
                  << " (signal number " << sig << ")\n";
        std::exit(EXIT_FAILURE);
    }

    // Called if an exception escapes main() or std::terminate() is invoked.
    void on_terminate() {
        std::exception_ptr ex = std::current_exception();
        if (ex) {
            try { std::rethrow_exception(ex); }
            catch (const std::exception& e) {
                std::cerr << "Error: Uncaught C++ exception: "
                          << e.what() << "\n";
            }
            catch (...) {
                std::cerr << "Error: Uncaught non-std::exception type\n";
            }
        } else {
            std::cerr << "Error: std::terminate() called without an active exception\n";
        }
        std::abort();  // ensure abnormal termination
    }
}

namespace error_handler {

    void setup_signal_handlers() {
        std::signal(SIGABRT, signal_handler);
        std::signal(SIGFPE,  signal_handler);
        std::signal(SIGILL,  signal_handler);
        std::signal(SIGINT,  signal_handler);
        std::signal(SIGSEGV, signal_handler);
        std::signal(SIGTERM, signal_handler);
    }

    void setup_terminate_handler() {
        std::set_terminate(on_terminate);
    }

} // namespace error_handler
