#include <cmath>
#include <iostream>
#include <fixp.hpp>

using fixed_q4_12 = fixp::fixed<12>;

int main(int argc, char *argv[])
{
    using fixed = fixed_q4_12;

    for (float x = 0; x < 8.0f; x += 0.001f) {
        std::cout << x << ","
                  << fixed::sqrt(x).to_float() << ","
                  << std::sqrt(x) << ","
                  << std::endl;
    }
    
    return 0;
}
