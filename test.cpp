#include <cstdint>
#include <iostream>
#include "fixp.hpp"

using fixed = fixp::fixed<std::uint32_t, 24>;

int
main(int argc, char* argv[])
{
    fixed a(12,8);

    std::cout << a.as_float() << std::endl;
    return 0;
}
