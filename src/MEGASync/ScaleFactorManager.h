#pragma once
#include <vector>
#include <memory>

struct ScreenInfo
{
    std::string name;
    int availableWidthPixels;
    int availableHeightPixels;
    int dotsPerInch;
    double devicePixelRatio;

    std::string toString() const
    {
        return name + ", " + std::to_string(availableWidthPixels) + ", " + std::to_string(availableHeightPixels)
                + ", " + std::to_string(dotsPerInch) + ", " + std::to_string(devicePixelRatio);
    }
};

enum class OsType{LINUX=0, WIN, MACOS};

using ScreensInfo = std::vector<ScreenInfo>;

class ScaleFactorManager
{
public:
    ScaleFactorManager(OsType osType);
    ScaleFactorManager(OsType osType, ScreensInfo screensInfo);
    void setScaleFactorEnvironmentVariable() const;
    std::vector<std::string> getLogMessages() const;

private:
    OsType mOsType;
    ScreensInfo mScreensInfo;
    mutable std::vector<std::string> logMessages;

    bool checkEnvirontmentVariables() const;
    double computeScale() const;
    double computeScaleLinux(const ScreenInfo& screenInfo) const;
    double getDpiOnLinux() const;
    ScreensInfo createScreensInfo() const;
};
