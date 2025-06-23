// error_handler.h
#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <csignal>
#include <exception>

namespace error_handler {

    /// Install handlers for fatal signals:
    ///   SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
    void setup_signal_handlers();

    /// Install a std::terminate handler to catch uncaught C++ exceptions.
    void setup_terminate_handler();

    /// Convenience: call both setup_signal_handlers() and setup_terminate_handler().
    inline void initialize_error_handlers() {
        setup_signal_handlers();
        setup_terminate_handler();
    }

} // namespace error_handler

#endif // ERROR_HANDLER_H
