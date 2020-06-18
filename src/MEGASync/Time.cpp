#include "Time.h"

std::chrono::system_clock::time_point Time::now() const
{
    return std::chrono::system_clock::now();
}
