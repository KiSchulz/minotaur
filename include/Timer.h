#ifndef TIMER_H
#define TIMER_H

#include <chrono>

template <typename unit = std::chrono::microseconds>
struct Timer {
  std::chrono::high_resolution_clock::time_point start;

  Timer() {
    start = std::chrono::high_resolution_clock::now();
  }

  unit stop() {

    std::chrono::high_resolution_clock::time_point end =
        std::chrono::high_resolution_clock::now();

    auto t = std::chrono::duration_cast<unit>(end - start);

    start = std::chrono::high_resolution_clock::now();

    t.count();

    return t;
  }
};

#endif // TIMER_H
