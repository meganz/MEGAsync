#include "ScaleFactorManager.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QDebug>
#include <QProcess>

#if ( defined(WIN32) && QT_VERSION >= 0x050000 ) || (defined(Q_OS_LINUX) && QT_VERSION >= 0x050600)
#include <QScreen>
#endif

ScaleFactorManager::ScaleFactorManager(OsType osType)
    :ScaleFactorManager{osType, createScreensInfo(), QSysInfo::prettyProductName().toStdString()}
{
}

ScaleFactorManager::ScaleFactorManager(OsType osType, ScreensInfo screensInfo, std::string osName)
    :mOsType{osType}, mScreensInfo{screensInfo}
{
    const auto autoScreenScaleFactor{getenv("QT_AUTO_SCREEN_SCALE_FACTOR")};
    if (autoScreenScaleFactor)
    {
        logMessages.emplace_back("QT_AUTO_SCREEN_SCALE_FACTOR is set to: " + std::string(autoScreenScaleFactor));
    }
    logMessages.emplace_back(osName);

    for(const auto& screenInfo : mScreensInfo)
    {
        logMessages.emplace_back("Screen detected: "+screenInfo.toString());
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
    if(mScreensInfo.empty())
    {
        throw std::runtime_error("No screens found");
    }

    if(!checkEnvirontmentVariables())
    {
        computeScales();

        if(mScreensInfo.size() > 1)
        {
            const auto screenScaleFactorVariable{createScreenScaleFactorsVariable(calculatedScales)};
            qputenv("QT_SCREEN_SCALE_FACTORS", screenScaleFactorVariable.c_str());
            logMessages.emplace_back("QT_SCREEN_SCALE_FACTORS set to "+screenScaleFactorVariable);
        }
        else
        {
            const auto scaleString{QString::number(calculatedScales.front()).toAscii()};
            qputenv("QT_SCALE_FACTOR", scaleString);
            logMessages.emplace_back("QT_SCALE_FACTOR set to "+scaleString);
        }
    }
}

std::vector<std::string> ScaleFactorManager::getLogMessages() const
{
    return logMessages;
}

bool ScaleFactorManager::checkEnvirontmentVariables() const
{
    if (getenv("QT_SCALE_FACTOR"))
    {
        qDebug() << "Not setting scale factors. Using predefined QT_SCALE_FACTOR=" << getenv("QT_SCALE_FACTOR");
        logMessages.emplace_back("Scale factor not calculated because QT_SCALE_FACTOR is already set to: "+
                                 std::string(getenv("QT_SCALE_FACTOR")));
        return true;
    }

    if (getenv("QT_SCREEN_SCALE_FACTORS"))
    {
        qDebug() << "Predefined QT_SCREEN_SCALE_FACTORS found:" << getenv("QT_SCREEN_SCALE_FACTORS");

        const auto predefinedScreenScaleFactors{std::string(getenv("QT_SCREEN_SCALE_FACTORS"))};
        auto screenScaleFactorsValid{true};
        for (const auto& screenInfo : mScreensInfo)
        {
            const auto textFound{predefinedScreenScaleFactors.find(screenInfo.name) != std::string::npos};
            if (!textFound)
            {
                screenScaleFactorsValid = false;
                qDebug() << "Screen name " << QString::fromStdString(screenInfo.name) << " not found in predefined QT_SCREEN_SCALE_FACTORS: " << getenv("QT_SCREEN_SCALE_FACTORS");
                logMessages.emplace_back("Screen name " + screenInfo.name + " not found in predefined QT_SCREEN_SCALE_FACTORS: " +
                               std::string(getenv("QT_SCREEN_SCALE_FACTORS")));
                break;
            }
        }

        if (screenScaleFactorsValid)
        {
            qDebug() << "Not setting scale factors. Using predefined QT_SCREEN_SCALE_FACTORS=" << getenv("QT_SCREEN_SCALE_FACTORS");
            logMessages.emplace_back("Scale factor not calculated because QT_SCREEN_SCALE_FACTORS is already set to: "+
                                     std::string(getenv("QT_SCREEN_SCALE_FACTORS")));
            return true;
        }
    }
    return false;
}

double adjustScaleValueToSuitableIncrement(double scale)
{
    constexpr auto dpiScreensSuitableIncrement = 1. / 6.; // this seems to work fine with 24x24 images at least
    return qRound(scale / dpiScreensSuitableIncrement) * dpiScreensSuitableIncrement;
}

double adjustScaleByMaxMin(double scale)
{
    constexpr auto maxScale = 3.;
    scale = std::min(maxScale, scale);

    constexpr auto minScale = 1.;
    scale = std::max(minScale, scale);

    return scale;
}

double adjustScaleByMaxHeight(double scale, const ScreenInfo& screenInfo)
{
    constexpr auto minTitleBarHeight = 20; // give some pixels to the tittle bar
    constexpr auto biggestDialogHeight = minTitleBarHeight + 600; //This is the height of the biggest dialog in megassync (Settings)
    const auto maxScale = screenInfo.availableHeightPixels / static_cast<double>(biggestDialogHeight);
    return std::min(maxScale, scale);
}

void ScaleFactorManager::computeScales()
{
    for(auto& screenInfo : mScreensInfo)
    {
        auto scale = 1.;
        if(mOsType==OsType::LINUX)
        {
            scale = computeScaleLinux(screenInfo);
        }
        scale = adjustScaleByMaxHeight(scale, screenInfo);
        scale = adjustScaleValueToSuitableIncrement(scale);
        scale = adjustScaleByMaxMin(scale);
        calculatedScales.push_back(scale);
    }
}

double ScaleFactorManager::computeScaleLinux(const ScreenInfo &screenInfo) const
{
    const auto forceCalculateScaleVarible{getenv("FORCE_CALCULATE_SCALE")};
    auto forceCalculateScale{false};
    if(forceCalculateScaleVarible)
    {
        logMessages.emplace_back("FORCE_CALCULATE_SCALE set to " + std::string(forceCalculateScaleVarible));
        forceCalculateScale = (forceCalculateScaleVarible == std::string("1"));
    }

    const auto hdpiAutoEnabled{screenInfo.devicePixelRatio > 2.0};
    if(hdpiAutoEnabled && !forceCalculateScale)
    {
        return 1.0;
    }

    constexpr auto baseDpi{96.};
    auto scale = 1.;
    const auto highDpi{screenInfo.dotsPerInch > baseDpi};
    if (highDpi)
    {
        scale = screenInfo.dotsPerInch / baseDpi;
    }
    else
    {
        constexpr auto baseWidthPixels{1920.};
        constexpr auto baseHeightPixels{1080.};
        constexpr auto correctionFactor{.75};
        scale = std::min(screenInfo.availableWidthPixels / baseWidthPixels,
                         screenInfo.availableHeightPixels / baseHeightPixels) * correctionFactor;
    }
    return scale;
}

double ScaleFactorManager::getDpiOnLinux() const
{
    //the best cross platform solution found (caveat: screen agnostic)
    QProcess p;
    p.start(QString::fromUtf8("bash -c \"xrdb -query | grep dpi | awk '{print $2}'\""));
    p.waitForFinished(2000);
    QString output = QString::fromUtf8(p.readAllStandardOutput().constData()).trimmed();
    QString e = QString::fromUtf8(p.readAllStandardError().constData());
    if (e.size())
    {
        qDebug() << "Error for \"xrdb -query\" command:" << e;
    }

    return qRound(output.toDouble());
}

ScreensInfo ScaleFactorManager::createScreensInfo()
{
    auto linuxDpi{0.};
    if(mOsType==OsType::LINUX)
    {
        linuxDpi = getDpiOnLinux();
    }

    int argc = 0;
    QGuiApplication app{argc, nullptr};
    const auto screens{app.screens()};
    ScreensInfo screensInfo;
    for (const auto& screen : screens)
    {
        ScreenInfo screenInfo;
        screenInfo.name = screen->name().toStdString();
        screenInfo.availableWidthPixels = screen->availableGeometry().width();
        screenInfo.availableHeightPixels = screen->availableGeometry().height();
        screenInfo.devicePixelRatio = screen->devicePixelRatio();

        const auto isLinuxAndDpiCalculationIsCorrect{mOsType==OsType::LINUX && linuxDpi > 0};
        screenInfo.dotsPerInch = isLinuxAndDpiCalculationIsCorrect ? linuxDpi : screen->logicalDotsPerInch();

        screensInfo.push_back(screenInfo);
    }
    return screensInfo;
}
