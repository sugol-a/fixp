#include <cstdint>
#include <iostream>
#include "fixp.hpp"

using fixed = fixp::fixed<12>;

int
main(int argc, char* argv[])
{
    std::cout << "x,float sin(x),fixed sin(x)" << std::endl;
    for (float x = 0.0f; x < 2.0f * std::numbers::pi; x += 0.001f) {
        fixed value = x;

        const fixed sin_x = fixed::sin(value);
        const float real_sin_x = std::sin(value.to_float());

        std::cout << x << "," << real_sin_x << "," << sin_x.to_float() << std::endl;
    }
    return 0;
}
