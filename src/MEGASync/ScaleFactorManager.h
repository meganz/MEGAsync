#pragma once
#include <vector>
#include <string>
#include <QString>

struct ScreenInfo
{
    QString name;
    int availableWidthPixels;
    int availableHeightPixels;
    double dotsPerInch;
    double devicePixelRatio;

    QString toString() const
    {
        return name + QString::fromUtf8(", ") +
               QString::number(availableWidthPixels) + QString::fromUtf8(", ") +
               QString::number(availableHeightPixels) + QString::fromUtf8(", ") +
               QString::number(dotsPerInch) + QString::fromUtf8(", ") +
               QString::number(devicePixelRatio);
    }
};

enum class OsType{LINUX=0, WIN, MACOS};

using ScreensInfo = std::vector<ScreenInfo>;

class ScaleFactorManager
{
public:
    ScaleFactorManager(OsType osType);
    ScaleFactorManager(OsType osType, ScreensInfo screensInfo, std::string osName, std::string desktopName);
    void setScaleFactorEnvironmentVariable();
    std::vector<std::string> getLogMessages() const;

private:
    OsType mOsType;
    std::string mOsName;
    ScreensInfo mScreensInfo;
    mutable std::vector<std::string> mLogMessages;
    std::vector<double> mCalculatedScales;
    std::string mDesktopName;

    bool checkEnvironmentVariables() const;
    bool computeScales();
    double computeScaleLinux(const ScreenInfo& screenInfo) const;
};
