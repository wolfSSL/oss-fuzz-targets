#pragma once

#include <optional>
#include <random>
#include <cstdint>
#include <cstdlib>

namespace fuzzing {
namespace util {

#define PRNG std::minstd_rand0

class Random : public PRNG {
 public:
  Random(std::optional<uint32_t> seed = std::nullopt) : PRNG(seed ? *seed : 0 /* TODO time */) {}
  result_type operator()() { return this->PRNG::operator()(); }
  size_t Get(void) {
      return operator()();
  }

  size_t Get(size_t n) {
      return n ? Get() % n : 0;
  }

  intptr_t Get(intptr_t From, intptr_t To) {
      intptr_t RangeSize = To - From + 1;
      return Get(RangeSize) + From;
  }

  size_t RandBool() { return Get() % 2; }
};

#undef PRNG

} /* namespace util */
} /* namespace fuzzing */
