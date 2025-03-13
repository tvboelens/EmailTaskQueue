#include "../include/randomhex.h"
#include <random>


std::string generateHex(size_t length)
{
    static const char hex_chars[] = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());                         // Mersenne Twister PRNG
    std::uniform_int_distribution<> distrib(0, 15); // Values from 0 to 15 (hex range)

    std::stringstream ss;
    for (size_t i = 0; i < length; ++i)
    {
        ss << hex_chars[distrib(gen)];
    }
    return ss.str();
}
