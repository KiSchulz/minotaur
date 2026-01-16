#pragma once

#include <stddef.h>

namespace minotaur {

struct CacheStats {
  size_t hits = 0;
  size_t misses = 0;
  size_t sols = 0;
  size_t solver_calls = 0;
  size_t total_solver_time = 0;
  size_t timeouts = 0;
  size_t canon_time = 0;
};

void cache_stats_add_solver_time(size_t t);
void cache_stats_inc_timeouts();
}
