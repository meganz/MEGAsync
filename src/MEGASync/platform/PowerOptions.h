#ifndef POWEROPTIONS_H
#define POWEROPTIONS_H

#include <memory>
#include <mega/types.h>

class PowerOptionsImpl;
class PowerOptions
{
public:
    PowerOptions();
    ~PowerOptions();

    static bool keepAwake(bool state);

private:
    static std::unique_ptr<PowerOptionsImpl> mPowerOptionsImpl;
};

#endif // POWEROPTIONS_H
