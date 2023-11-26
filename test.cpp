#include <cstdint>
#include <iostream>
#include <bitset>
#include "fixp.hpp"

using fixed = fixp::fixed<12>;

int
main(int argc, char* argv[])
{
    fixed a = 5.0f;
    fixed b = 3.0f;

    float result = (a / b).to_float();

    std::cout << result << std::endl;
    
    return 0;
}
