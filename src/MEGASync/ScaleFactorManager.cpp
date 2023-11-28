#include "ScaleFactorManager.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QDebug>
#include <QProcess>
#include <QScreen>
#include <stdexcept>

double getDpiOnLinux()
{
    //the best cross platform solution found (caveat: screen agnostic)
    QProcess p;
    p.start(QString::fromUtf8("bash"), QStringList() << QString::fromUtf8("-c") << QString::fromUtf8("xrdb -query | grep dpi | awk '{print $2}'"));
    p.waitForFinished(2000);
    QString output = QString::fromUtf8(p.readAllStandardOutput().constData()).trimmed();
    QString e = QString::fromUtf8(p.readAllStandardError().constData());
    if (e.size())
    {
        qDebug() << "Error for \"xrdb -query\" command:" << e;
    }

    return qRound(output.toDouble());
}

double getWindowScalingFactorOnXcfe()
{
    // when zoom is set on Xcfe on Ubuntu 20 the only way to check the value is to query xfconf-query
    // querying xrdb would give 96.0 even when zoom is configured
    auto windowScalingFactor = 1.0;
    QProcess p;
    p.start(QString::fromUtf8("bash"), QStringList() << QString::fromUtf8("-c") << QString::fromUtf8("xfconf-query -c xsettings -p /Gdk/WindowScalingFactor"));

    p.waitForFinished(2000);
    QString output = QString::fromUtf8(p.readAllStandardOutput().constData()).trimmed();
    QString e = QString::fromUtf8(p.readAllStandardError().constData());
    const bool outputCorrect{e.isEmpty() && output.size() == 1};
    if (outputCorrect)
    {
        bool conversionOk;
        const auto scalingFactor = output.toDouble(&conversionOk);
        if(conversionOk)
        {
            windowScalingFactor = scalingFactor;
        }
    }
    return windowScalingFactor;
}

ScreensInfo createScreensInfo(OsType osType, const std::string& desktopName)
{
    if(QSysInfo::prettyProductName().toStdString() == "Deepin 20")
    {
        return {};
    }

    auto linuxDpi = 0.;
    if(osType == OsType::LINUX)
    {
        linuxDpi = getDpiOnLinux();

        if(desktopName == "XFCE")
        {
            linuxDpi *= getWindowScalingFactorOnXcfe();
        }
    }

    int argc = 0;
    QGuiApplication app{argc, nullptr};
    const auto screens = app.screens();
    ScreensInfo screensInfo;
    for (const auto& screen : screens)
    {
        ScreenInfo screenInfo;
        screenInfo.name = screen->name().toStdString();
        screenInfo.availableWidthPixels = screen->availableGeometry().width();
        screenInfo.availableHeightPixels = screen->availableGeometry().height();
        screenInfo.devicePixelRatio = screen->devicePixelRatio();

        const bool isLinuxAndDpiCalculationIsCorrect{osType==OsType::LINUX && linuxDpi > 0};
        screenInfo.dotsPerInch = isLinuxAndDpiCalculationIsCorrect ? linuxDpi : screen->logicalDotsPerInch();

        screensInfo.push_back(screenInfo);
    }
    return screensInfo;
}

std::string getDesktopName()
{
    std::string desktopName;
    const auto xdgCurrentDesktop = getenv("XDG_CURRENT_DESKTOP");
    if (xdgCurrentDesktop)
    {
        desktopName = xdgCurrentDesktop;
    }
    return desktopName;
}

ScaleFactorManager::ScaleFactorManager(OsType osType)
    :ScaleFactorManager{osType, createScreensInfo(osType, getDesktopName()), QSysInfo::prettyProductName().toStdString(), getDesktopName()}
{
}

ScaleFactorManager::ScaleFactorManager(OsType osType, ScreensInfo screensInfo, std::string osName, std::string desktopName)
    :mOsType{osType}, mOsName{osName}, mScreensInfo{screensInfo}, mDesktopName{desktopName}
{
    if(mDesktopName.empty())
    {
        mLogMessages.emplace_back(mOsName);
    }
    else
    {
        mLogMessages.emplace_back(mOsName + " (" + mDesktopName + ")");
    }

    for(const auto& screenInfo : mScreensInfo)
    {
        mLogMessages.emplace_back("Screen detected: "+screenInfo.toString());
    }
}

std::string createScreenScaleFactorsVariable(std::vector<double> calculatedScales)
{
    std::string screenScaleFactorVariable;
    for(const auto calculatedScale : calculatedScales)
    {
        if(!screenScaleFactorVariable.empty())
        {
            screenScaleFactorVariable.append(";");
        }
        screenScaleFactorVariable.append(std::to_string(calculatedScale));
    }
    return screenScaleFactorVariable;
}

void ScaleFactorManager::setScaleFactorEnvironmentVariable()
{
    if(mScreensInfo.empty() && mOsName != "Deepin 20")
    {
        throw std::runtime_error("No screens found");
    }

    if(!checkEnvironmentVariables())
    {
        if(mOsName == "Deepin 20")
        {
            const auto scale = getDpiOnLinux() / 96.0;
            qputenv("QT_SCALE_FACTOR", QString::number(scale).toAscii());
            return;
        }

        const bool needsRescaling{computeScales()};
        if(needsRescaling)
        {
            if(mScreensInfo.size() > 1)
            {
                if(mOsName == "Deepin 20")
                {
                    const auto minCalculatedScale = *std::min_element(mCalculatedScales.begin(), mCalculatedScales.end());
                    const auto minCalculatedScaleString = QString::number(minCalculatedScale).toAscii();
                    qputenv("QT_SCALE_FACTOR", minCalculatedScaleString);
                    mLogMessages.emplace_back("QT_SCALE_FACTOR set to " + minCalculatedScaleString);
                }
                else
                {
                    const auto screenScaleFactorVariable = createScreenScaleFactorsVariable(mCalculatedScales);
                    qputenv("QT_SCREEN_SCALE_FACTORS", screenScaleFactorVariable.c_str());
                    mLogMessages.emplace_back("QT_SCREEN_SCALE_FACTORS set to " + screenScaleFactorVariable);
                }
            }
            else
            {
                const auto scaleString = QString::number(mCalculatedScales.front()).toAscii();
                qputenv("QT_SCALE_FACTOR", scaleString);
                mLogMessages.emplace_back("QT_SCALE_FACTOR set to " + scaleString);
            }
        }
        else
        {
            mLogMessages.emplace_back("Scaling not needed.");
        }
    }
}

std::vector<std::string> ScaleFactorManager::getLogMessages() const
{
    return mLogMessages;
}

bool ScaleFactorManager::checkEnvironmentVariables() const
{
    if (getenv("QT_SCALE_FACTOR"))
    {
        qDebug() << "Not setting scale factors. Using predefined QT_SCALE_FACTOR=" << getenv("QT_SCALE_FACTOR");
        mLogMessages.emplace_back("Scale factor not calculated because QT_SCALE_FACTOR is already set to: "+
                                 std::string(getenv("QT_SCALE_FACTOR")));
        return true;
    }

    if (getenv("QT_SCREEN_SCALE_FACTORS"))
    {
        qDebug() << "Predefined QT_SCREEN_SCALE_FACTORS found:" << getenv("QT_SCREEN_SCALE_FACTORS");

        const auto predefinedScreenScaleFactors = std::string(getenv("QT_SCREEN_SCALE_FACTORS"));
        bool screenScaleFactorsValid = true;
        for (const auto& screenInfo : mScreensInfo)
        {
            const bool textFound{predefinedScreenScaleFactors.find(screenInfo.name) != std::string::npos};
            if (!textFound)
            {
                screenScaleFactorsValid = false;
                qDebug() << "Screen name " << QString::fromStdString(screenInfo.name) << " not found in predefined QT_SCREEN_SCALE_FACTORS: " << getenv("QT_SCREEN_SCALE_FACTORS");
                mLogMessages.emplace_back("Screen name " + screenInfo.name + " not found in predefined QT_SCREEN_SCALE_FACTORS: " +
                               std::string(getenv("QT_SCREEN_SCALE_FACTORS")));
                break;
            }
        }

        if (screenScaleFactorsValid)
        {
            qDebug() << "Not setting scale factors. Using predefined QT_SCREEN_SCALE_FACTORS=" << getenv("QT_SCREEN_SCALE_FACTORS");
            mLogMessages.emplace_back("Scale factor not calculated because QT_SCREEN_SCALE_FACTORS is already set to: "+
                                     std::string(getenv("QT_SCREEN_SCALE_FACTORS")));
            return true;
        }
    }
    return false;
}

double adjustScaleValueToSuitableIncrement(double scale, double maxScale, double pixelRatio)
{
    auto dpiScreensSuitableIncrement = 1. / 6.; // this seems to work fine with 24x24 images at least
    dpiScreensSuitableIncrement /= pixelRatio;
    scale = qRound(scale / dpiScreensSuitableIncrement) * dpiScreensSuitableIncrement;
    if(scale > maxScale)
    {
        scale -= dpiScreensSuitableIncrement;
    }
    return scale;
}

double calculateMaxScale(const ScreenInfo& screenInfo)
{
    constexpr auto minTitleBarHeight = 20; // give some pixels to the tit le bar
    constexpr auto maxDialogHeight = 740;  //This is the height of the biggest dialog in megassync (Settings)

    constexpr auto biggestDialogHeight = minTitleBarHeight + maxDialogHeight;
    return screenInfo.availableHeightPixels * screenInfo.devicePixelRatio / static_cast<double>(biggestDialogHeight);
}

bool ScaleFactorManager::computeScales()
{
    bool needsRescaling = false;
    for(auto& screenInfo : mScreensInfo)
    {
        auto scale = screenInfo.devicePixelRatio;
        if(mOsType == OsType::LINUX)
        {
            scale = computeScaleLinux(screenInfo);
        }
        const auto maxScale = calculateMaxScale(screenInfo);
        scale = std::min(scale, maxScale);
        scale = adjustScaleValueToSuitableIncrement(scale, maxScale, screenInfo.devicePixelRatio);

        const bool hdpiAutoEnabled{screenInfo.devicePixelRatio > 1.0};
        const bool scaleNeedsToBeAdjusted{scale < 1.0 && !hdpiAutoEnabled};
        if(scaleNeedsToBeAdjusted)
        {
            scale = 1.0;
        }

        if(scale != 1.0)
        {
            needsRescaling = true;
        }
        mCalculatedScales.push_back(scale);
    }
    return needsRescaling;
}

double ScaleFactorManager::computeScaleLinux(const ScreenInfo &screenInfo) const
{
    const bool hdpiAutoEnabled{screenInfo.devicePixelRatio > 1.0};
    if(hdpiAutoEnabled)
    {
        return 1.0;
    }

    constexpr auto baseDpi = 96.;
    auto scale = 1.;
    const bool highDpi{screenInfo.dotsPerInch > baseDpi};
    if (highDpi)
    {
        scale = screenInfo.dotsPerInch / baseDpi;
    }
    else
    {
        constexpr auto baseWidthPixels = 1920.;
        constexpr auto baseHeightPixels = 1080.;
        constexpr auto correctionFactor = .75;
        scale = std::min(screenInfo.availableWidthPixels / baseWidthPixels,
                         screenInfo.availableHeightPixels / baseHeightPixels) * correctionFactor;
        scale = std::max(scale, 1.0); // avoid problematic scales lower than 1.0 when no hdpi enabled
    }
    scale = std::min(scale, 3.0); // avoid too big scales
    return scale;
}
