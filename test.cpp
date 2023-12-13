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
using gentype = std::mt19937;

namespace benches {
    template<fixp::is_fixed T>
    void fixed_sqrt(gentype& gen) {
        std::uniform_real_distribution<float> rng(0.0, 100.0f);
        T f = rng(gen);
        T result = T::sqrt(f);

        nanobench::doNotOptimizeAway(result);
    }

    void float_sqrt(gentype& gen) {
        std::uniform_real_distribution<float> rng(0.0, 100.0f);
        float f = rng(gen);
        float result = std::sqrt(f);
        nanobench::doNotOptimizeAway(result);
    }

    template<fixp::is_fixed T>
    void fixed_sin(gentype& gen) {
        std::uniform_real_distribution<float> rng(0.0, 2.0f * std::numbers::pi);

        T f = rng(gen);
        T result = T::sin(f);
        nanobench::doNotOptimizeAway(result);
    }

    void float_sin(gentype& gen) {
        std::uniform_real_distribution<float> rng(0.0, 2.0f * std::numbers::pi);

        float f = rng(gen);
        float result = std::sin(f);
        nanobench::doNotOptimizeAway(result);
    }

    template<fixp::is_fixed T>
    void fixed_cos(gentype& gen) {
        std::uniform_real_distribution<float> rng(0.0, 2.0f * std::numbers::pi);

        T f = rng(gen);
        T result = T::cos(f);
        nanobench::doNotOptimizeAway(result);
    }

    void float_cos(gentype& gen) {
        std::uniform_real_distribution<float> rng(0.0, 2.0f * std::numbers::pi);

        float f = rng(gen);
        float result = std::cos(f);
        nanobench::doNotOptimizeAway(result);
    }
}

struct bench_case {
    const char* name;
    std::function<void(gentype&)> func;

    bench_case(const char* name, std::function<void(gentype&)> func) {
        this->name = name;
        this->func = func;
    }
};

using fixed_q16_16 = fixp::fixed<16, std::int32_t, std::int64_t>;
using fixed_q4_12 = fixp::fixed<12, std::int16_t, std::int32_t>;
using fixed_q8_8 = fixp::fixed<8, std::int16_t, std::int32_t>;

static const bench_case cases[] = {
    { "float sqrt", benches::float_sqrt },
    { "fixed sqrt Q16.16", benches::fixed_sqrt<fixed_q16_16> },
    { "fixed sqrt Q4.12", benches::fixed_sqrt<fixed_q4_12> },
    { "fixed sqrt Q8.8", benches::fixed_sqrt<fixed_q8_8> },
    { "float sin", benches::float_sin },
    { "fixed sin Q16.16", benches::fixed_sin<fixed_q16_16> },
    { "fixed sin Q4.12", benches::fixed_sin<fixed_q4_12> },
    { "fixed sin Q8.8", benches::fixed_sin<fixed_q8_8> },
};

int main(int argc, char *argv[])
{
    std::random_device rd;
    std::mt19937 gen(rd());

    for (const auto& bc : cases) {
        nanobench::Bench().run(bc.name, [&]() {
            bc.func(gen);
        });
    }

    return 0;
}
