#pragma once
#include <QList>
#include <QScreen>
#include <QString>
#include <QVector>
#include <vector>

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
    ScaleFactorManager(OsType osType, const QList<QScreen*>& screens);
    ScaleFactorManager(OsType osType,
                       ScreensInfo screensInfo,
                       const QString& osName,
                       const QString& desktopName);
    void setScaleFactorEnvironmentVariable();
    QVector<QString> getLogMessages() const;

private:
    OsType mOsType;
    QString mOsName;
    ScreensInfo mScreensInfo;
    mutable QVector<QString> mLogMessages;
    std::vector<double> mCalculatedScales;
    QString mDesktopName;

    bool checkEnvironmentVariables() const;
    bool computeScales();
    double computeScaleLinux(const ScreenInfo& screenInfo) const;
};
