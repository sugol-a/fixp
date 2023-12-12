#include <cstdint>
#include <iostream>
#include <format>
#include <cmath>
#include "fixp.hpp"

using fixed = fixp::fixed<14, std::int32_t, std::int64_t>;

template <typename T>
std::string
as_bin(T x)
{
    std::string result;
    constexpr std::size_t n_bits = sizeof(T) * 8;

    for (std::size_t i = 0; i < n_bits; i++) {
        result += (x & (1 << (n_bits - i - 1))) ? '1' : '0';
    }

    return result;
}

int
main(int argc, char* argv[])
{
    std::cout << "x,fixed sqrt(x),real sqrt(x)" << std::endl;
    for (float x = 0; x < 3.0f; x += 0.01) {
        fixed fx_sqrt = fixed::sqrt<1024>(x);

        std::cout << x << "," << fx_sqrt.to_float() << "," << std::sqrt(x) << std::endl;
    }
    
    return 0;
}
