#include <cstdint>
#include <iostream>
#include <format>
#include "fixp.hpp"

using fixed = fixp::fixed<12>;

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
    constexpr fixed pi = std::numbers::pi_v<float>;
    constexpr fixed half_pi = 0.5f * std::numbers::pi_v<float>;
    std::cout << fixed::sin(half_pi).to_float() << std::endl;

    return 0;
}
