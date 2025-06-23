#include <csignal>
#include <cstdlib>
#include <iostream>
#include <vector>

void crash_handler(int sig) {
    std::cerr << "Fatal error: signal " << sig << " (access violation)\n";
    std::exit(EXIT_FAILURE);
}

int main() {
    // Install our handler before any bad pointer use
    std::signal(SIGSEGV, crash_handler);

    std::vector<int> nums;
    nums[-1] = 0;
    int* p = nullptr;
    *p = 42;   // → raises SIGSEGV, lands in crash_handler()

    return 0;
}
