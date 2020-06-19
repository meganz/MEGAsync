#ifndef TIME_H
#define TIME_H
#include <chrono>

class Time
{
public:
    virtual std::chrono::system_clock::time_point now() const;
};

#endif // TIME_H
