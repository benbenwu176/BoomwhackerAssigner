// RandomUtils.hpp
#pragma once
#include <random>
#include <atomic>

namespace random_utils {

  // ——— Global seed state ———————————————————————————————
  // Must call seed() before any thread uses engine().
  static std::atomic<bool>  g_seeded{false};
  static std::atomic<uint32_t> g_seed{0};

  inline void seed(uint32_t s) {
    g_seed.store(s, std::memory_order_relaxed);
    g_seeded.store(true, std::memory_order_relaxed);
  }

  // ——— Per-thread engine —————————————————————————————
  inline std::mt19937& engine() {
    thread_local std::mt19937 eng{ [] {
      if (g_seeded.load(std::memory_order_relaxed)) {
        return g_seed.load(std::memory_order_relaxed);
      } else {
        return std::random_device{}();
      }
    }() };
    return eng;
  }

  // ——— Utility generators ————————————————————————————
  inline int randInt(int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(engine());
  }

  inline double randReal(double lo, double hi) {
    std::uniform_real_distribution<double> dist(lo, hi);
    return dist(engine());
  }

}
