#include <cstdint>
#include <iostream>
#include <bitset>
#include <format>
#include "fixp.hpp"

using fixed = fixp::fixed<12>;

int
main(int argc, char* argv[])
{
    for (float x = 0.0f; x < 1.2; x += 0.01f) {
        fixed value = x;

        fixed sin_x = fixed::sin(value);
        fixed cos_x = fixed::cos(value);

        float real_sin_x = std::sin(value.to_float());
        float real_cos_x = std::cos(value.to_float());

        std::cout << std::format("sin({}) = {}", value.to_float(), sin_x.to_float()) << std::endl;;
        std::cout << std::format("real sin({}) = {}", value.to_float(), real_sin_x) << std::endl;;
        std::cout << std::format("cos({}) = {}", value.to_float(), cos_x.to_float()) << std::endl;
        std::cout << std::format("real cos({}) = {}", value.to_float(), real_cos_x) << std::endl;
    }
    return 0;
}
