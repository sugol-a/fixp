#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <format>
#include <cmath>

#include "fixp.hpp"
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

using namespace ankerl;

using fixed = fixp::fixed<16, std::int32_t, std::int64_t>;

int main(int argc, char *argv[])
{
    nanobench::Bench().run("fixed sqrt", [] {
        fixed f = rand();
        fixed result = fixed::sqrt(f);

        nanobench::doNotOptimizeAway(result);
    });

    return 0;
}


// #define PICOBENCH_IMPLEMENT_WITH_MAIN
// #include <picobench/picobench.hpp>

// #include "fixp.hpp"

// using fixed = fixp::fixed<8, std::int32_t, std::int64_t>;

// template <typename T>
// std::string
// as_bin(T x)
// {
//     std::string result;
//     constexpr std::size_t n_bits = sizeof(T) * 8;

//     for (std::size_t i = 0; i < n_bits; i++) {
//         result += (x & (1 << (n_bits - i - 1))) ? '1' : '0';
//     }

//     return result;
// }

// void
// bench_fixed_sqrt(picobench::state& s)
// {
//     for (const auto& _ : s) {
//         fixed f = rand();
//         fixed result = fixed::from_raw(fixed::sqrt<3>(f).to_raw());

//         s.set_result(result.to_raw());
//     }
// }
// PICOBENCH(bench_fixed_sqrt).samples(1000);

// void
// bench_float_sqrt(picobench::state& s)
// {
//     for (const auto& _ : s) {
//         float x = rand();
//         float result = std::sqrt(x);

//         s.set_result(result);
//     }
// }
// PICOBENCH(bench_float_sqrt).samples(1000);

// int
// main(int argc, char* argv[])
// {
//     std::cout << "x,fixed sq(x),real sq(x)" << std::endl;

//     for (fixed x = 0.0f; x < 64.0f; x += 0.01f) {
//         std::cout << x.to_string() << "," << fixed::sqrt<32>(x).to_string() << std::endl;
//     }

//     return 0;
// }
