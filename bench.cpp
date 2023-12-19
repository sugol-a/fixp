#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <format>
#include <cmath>
#include <functional>

#include "fixp.hpp"
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

using namespace ankerl;

static nanobench::Rng rng;
static constexpr float two_pi = 2.0f * std::numbers::pi_v<float>;

namespace benches {
    namespace transcendental {
        template<fixp::is_fixed T>
        void fixed_sqrt() {
            T f = rng.uniform01() * 100.0f;
            T result = T::sqrt(f);

            nanobench::doNotOptimizeAway(result);
        }

        void float_sqrt() {
            float f = rng.uniform01() * 100.0f;
            float result = std::sqrt(f);
            nanobench::doNotOptimizeAway(result);
        }

        template<fixp::is_fixed T>
        void fixed_sin() {
            T f = 2.0f * (rng.uniform01() - 0.5f) * two_pi; // [-2pi,2pi]
            T result = T::sin(f);
            nanobench::doNotOptimizeAway(result);
        }

        void float_sin() {
            float f = 2.0f * (rng.uniform01() - 0.5f) * two_pi;
            float result = std::sin(f);
            nanobench::doNotOptimizeAway(result);
        }

        template<fixp::is_fixed T>
        void fixed_cos() {
            T f = 2.0f * (rng.uniform01() - 0.5f) * two_pi;
            T result = T::cos(f);
            nanobench::doNotOptimizeAway(result);
        }

        void float_cos() {
            float f = 2.0f * (rng.uniform01() - 0.5f) * two_pi;
            float result = std::cos(f);
            nanobench::doNotOptimizeAway(result);
        }
    }

    namespace arithmetic {
        template<fixp::is_fixed T>
        void fixed_add() {
            T a = rng.uniform01();
            T b = rng.uniform01();

            T result = a + b;
            nanobench::doNotOptimizeAway(result);
        }

        void float_add() {
            float a = rng.uniform01();
            float b = rng.uniform01();

            float result = a + b;
            nanobench::doNotOptimizeAway(result);
        }

        template<fixp::is_fixed T>
        void fixed_sub() {
            T a = rng.uniform01();
            T b = rng.uniform01();

            T result = a - b;
            nanobench::doNotOptimizeAway(result);
        }

        void float_sub() {
            float a = rng.uniform01();
            float b = rng.uniform01();

            float result = a - b;
            nanobench::doNotOptimizeAway(result);
        }

        template<fixp::is_fixed T>
        void fixed_mul() {
            T a = rng.uniform01();
            T b = rng.uniform01();

            T result = a * b;
            nanobench::doNotOptimizeAway(result);
        }

        void float_mul() {
            float a = rng.uniform01();
            float b = rng.uniform01();

            float result = a * b;
            nanobench::doNotOptimizeAway(result);
        }

        template<fixp::is_fixed T>
        void fixed_div() {
            T a = rng.uniform01();
            T b = (rng.uniform01() + 1.0f);

            T result = a / b;
            nanobench::doNotOptimizeAway(result);
        }

        void float_div() {
            float a = rng.uniform01();
            float b = (rng.uniform01() + 1.0f);

            float result = a / b;
            nanobench::doNotOptimizeAway(result);
        }
    }

    namespace util {
        template<fixp::is_fixed T>
        void fixed_to_string() {
            T f = (rng.uniform01() - 0.5f) * 2.0f;
            std::string result = f.to_string();

            nanobench::doNotOptimizeAway(result);
        }
    }
}

struct bench_case {
    const char* name;
    std::function<void(void)> func;

    bench_case(const char* name, std::function<void(void)> func) {
        this->name = name;
        this->func = func;
    }
};

using fixed_q16_16 = fixp::fixed<16, std::int32_t, std::int64_t>;
using fixed_q24_8 = fixp::fixed<24, std::int32_t, std::int64_t>;
using fixed_q4_12 = fixp::fixed<12, std::int16_t, std::int32_t>;
using fixed_q8_8 = fixp::fixed<8, std::int16_t, std::int32_t>;

static const bench_case cases[] = {
    { "float sqrt",        benches::transcendental::float_sqrt              },
    { "fixed sqrt Q16.16", benches::transcendental::fixed_sqrt<fixed_q16_16>},
    { "fixed sqrt Q4.12",  benches::transcendental::fixed_sqrt<fixed_q4_12> },
    { "fixed sqrt Q8.8",   benches::transcendental::fixed_sqrt<fixed_q8_8>  },
    { "float sin",         benches::transcendental::float_sin               },
    { "fixed sin Q16.16",  benches::transcendental::fixed_sin<fixed_q16_16> },
    { "fixed sin Q4.12",   benches::transcendental::fixed_sin<fixed_q4_12>  },
    { "fixed sin Q8.8",    benches::transcendental::fixed_sin<fixed_q8_8>   },
    { "float cos",         benches::transcendental::float_cos               },
    { "fixed cos Q16.16",  benches::transcendental::fixed_cos<fixed_q16_16> },
    { "fixed cos Q4.12",   benches::transcendental::fixed_cos<fixed_q4_12>  },
    { "fixed cos Q8.8",    benches::transcendental::fixed_cos<fixed_q8_8>   },

    { "float add",         benches::arithmetic::float_add                   },
    { "fixed add Q16.16",  benches::arithmetic::fixed_add<fixed_q16_16>     },
    { "fixed add Q4.12",   benches::arithmetic::fixed_add<fixed_q4_12>      },
    { "fixed add Q8.8",    benches::arithmetic::fixed_add<fixed_q8_8>       },
    { "float sub"        , benches::arithmetic::float_sub },
    { "fixed sub Q16.16" , benches::arithmetic::fixed_sub<fixed_q16_16> },
    { "fixed sub Q4.12"  , benches::arithmetic::fixed_sub<fixed_q4_12> },
    { "fixed sub Q8.8"   , benches::arithmetic::fixed_sub<fixed_q8_8> },
    { "float mul"        , benches::arithmetic::float_mul },
    { "fixed mul Q16.16" , benches::arithmetic::fixed_mul<fixed_q16_16> },
    { "fixed mul Q4.12"  , benches::arithmetic::fixed_mul<fixed_q4_12> },
    { "fixed mul Q8.8"   , benches::arithmetic::fixed_mul<fixed_q8_8> },
    { "float div"        , benches::arithmetic::float_div },
    { "fixed div Q16.16" , benches::arithmetic::fixed_div<fixed_q16_16> },
    { "fixed div Q4.12"  , benches::arithmetic::fixed_div<fixed_q4_12> },
    { "fixed div Q8.8"   , benches::arithmetic::fixed_div<fixed_q8_8> },
    
    { "to string Q16.16"  , benches::util::fixed_to_string<fixed_q16_16> },
    { "to string Q4.12"   , benches::util::fixed_to_string<fixed_q4_12> },
    { "to string Q8.8"    , benches::util::fixed_to_string<fixed_q8_8> },
};

int main(int argc, char *argv[])
{
    for (const auto& bc : cases) {
        nanobench::Bench().run(bc.name, bc.func);
    }

    return 0;
}
