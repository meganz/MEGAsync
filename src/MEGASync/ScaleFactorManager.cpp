#include "ScaleFactorManager.h"

#include <QDebug>
#include <QGuiApplication>
#include <QProcess>
#include <QScreen>

#include <stdexcept>

const QString LINUX_OS_DEEPIN_20 = QString::fromUtf8("Deepin 20");

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

ScreensInfo createScreensInfo(OsType osType, const QString& desktopName)
{
    if (QSysInfo::prettyProductName() == LINUX_OS_DEEPIN_20)
    {
        return {};
    }

    auto linuxDpi = 0.;
    if (osType == OsType::LINUX)
    {
        linuxDpi = getDpiOnLinux();

        if (desktopName == QString::fromUtf8("XFCE"))
        {
            linuxDpi *= getWindowScalingFactorOnXcfe();
        }
    }
    else if (osType == OsType::WIN)
    {
        // Set QT_SCALE_FACTOR_ROUNDING_POLICY to PassThrough, but do not override user setting
        // Note: Qt5 default value is Round, but Qt6 default value is PassThrough
        const auto scaleFactorRoundingPolicyFromEnv =
            qEnvironmentVariable("QT_SCALE_FACTOR_ROUNDING_POLICY");
        const auto scaleFactorRoundingPolicyFromQt =
            QGuiApplication::highDpiScaleFactorRoundingPolicy();

        const auto targetPolicy = Qt::HighDpiScaleFactorRoundingPolicy::PassThrough;

        if (scaleFactorRoundingPolicyFromEnv.isEmpty() &&
            scaleFactorRoundingPolicyFromQt != targetPolicy)
        {
            QGuiApplication::setHighDpiScaleFactorRoundingPolicy(targetPolicy);
            qDebug() << "Setting QT scale factor rounding policy to " << targetPolicy;
        }
        else
        {
            qDebug() << "QT scale factor rounding policy already set to "
                     << scaleFactorRoundingPolicyFromQt;
        }
    }

    // QGuiApplication needs to be created, even if screens() is static.
    // Otherwise, the screens are not detected.
    int argc = 0;
    QGuiApplication app{argc, nullptr};
    const auto screens = app.screens();

    ScreensInfo screensInfo;
    for (const auto& screen: screens)
    {
        ScreenInfo screenInfo;
        screenInfo.name = screen->name();
        screenInfo.availableWidthPixels = screen->availableGeometry().width();
        screenInfo.availableHeightPixels = screen->availableGeometry().height();
        screenInfo.devicePixelRatio = screen->devicePixelRatio();

        const bool isLinuxAndDpiCalculationIsCorrect{osType == OsType::LINUX && linuxDpi > 0};
        screenInfo.dotsPerInch =
            isLinuxAndDpiCalculationIsCorrect ? linuxDpi : screen->logicalDotsPerInch();

        screensInfo.push_back(screenInfo);
    }
    return screensInfo;
}

QString getDesktopName()
{
    return qEnvironmentVariable("XDG_CURRENT_DESKTOP");
}

ScaleFactorManager::ScaleFactorManager(OsType osType):
    ScaleFactorManager(osType,
                       createScreensInfo(osType, getDesktopName()),
                       QSysInfo::prettyProductName(),
                       getDesktopName())
{}

ScaleFactorManager::ScaleFactorManager(OsType osType,
                                       ScreensInfo screensInfo,
                                       const QString& osName,
                                       const QString& desktopName):
    mOsType(osType),
    mOsName(osName),
    mScreensInfo(screensInfo),
    mDesktopName(desktopName)
{
    if (mDesktopName.isEmpty())
    {
        mLogMessages.push_back(mOsName);
    }
    else
    {
        mLogMessages.push_back(mOsName + QString::fromUtf8(" (") + mDesktopName +
                               QString::fromUtf8(")"));
    }

    for (const auto& screenInfo: mScreensInfo)
    {
        QString msg = QString::fromUtf8("Screen detected: ") + screenInfo.toString();
        mLogMessages.push_back(msg);
    }
}

QString createScreenScaleFactorsVariable(std::vector<double> calculatedScales)
{
    QString screenScaleFactorVariable;
    for (const double& calculatedScale : calculatedScales)
    {
        if (!screenScaleFactorVariable.isEmpty())
        {
            screenScaleFactorVariable.append(QString::fromUtf8(";"));
        }

        screenScaleFactorVariable.append(QString::number(calculatedScale));
    }
    return screenScaleFactorVariable;
}

void ScaleFactorManager::setScaleFactorEnvironmentVariable()
{
    if (mScreensInfo.empty() && mOsName != LINUX_OS_DEEPIN_20)
    {
        throw std::runtime_error("No screens found");
    }

    if(!checkEnvironmentVariables())
    {
        if (mOsName == LINUX_OS_DEEPIN_20)
        {
            const auto scale = getDpiOnLinux() / 96.0;
            qputenv("QT_SCALE_FACTOR", QString::number(scale).toLatin1());
            return;
        }

        const bool needsRescaling{computeScales()};
        if(needsRescaling)
        {
            if(mScreensInfo.size() > 1)
            {
                if (mOsName == LINUX_OS_DEEPIN_20)
                {
                    const auto minCalculatedScale =
                        *std::min_element(mCalculatedScales.begin(), mCalculatedScales.end());
                    const QString minCalculatedScaleString = QString::number(minCalculatedScale);
                    qputenv("QT_SCALE_FACTOR", minCalculatedScaleString.toUtf8());
                    mLogMessages.push_back(QString::fromUtf8("QT_SCALE_FACTOR set to ") +
                                           minCalculatedScaleString);
                }
                else
                {
                    const auto screenScaleFactorVariable =
                        createScreenScaleFactorsVariable(mCalculatedScales);
                    qputenv("QT_SCREEN_SCALE_FACTORS", screenScaleFactorVariable.toUtf8());
                    mLogMessages.push_back(QString::fromUtf8("QT_SCREEN_SCALE_FACTORS set to ") +
                                           screenScaleFactorVariable);
                }
            }
            else
            {
                const QString scaleString = QString::number(mCalculatedScales.front());
                qputenv("QT_SCALE_FACTOR", scaleString.toUtf8());
                mLogMessages.push_back(QString::fromUtf8("QT_SCALE_FACTOR set to ") + scaleString);
            }
        }
        else
        {
            mLogMessages.push_back(QString::fromUtf8("Scaling not needed."));
        }
    }
}

QVector<QString> ScaleFactorManager::getLogMessages() const
{
    return mLogMessages;
}

bool ScaleFactorManager::checkEnvironmentVariables() const
{
    QString envValueQtScaleFactor = qEnvironmentVariable("QT_SCALE_FACTOR");

    if (!envValueQtScaleFactor.isEmpty())
    {
        qDebug() << "Not setting scale factors. Using predefined QT_SCALE_FACTOR="
                 << envValueQtScaleFactor;
        QString msg =
            QString::fromUtf8(
                "Scale factor not calculated because QT_SCALE_FACTOR is already set to: ") +
            envValueQtScaleFactor;
        mLogMessages.push_back(msg);
        return true;
    }

    QString envValueQtScreenScaleFactors = qEnvironmentVariable("QT_SCREEN_SCALE_FACTORS");

    if (!envValueQtScreenScaleFactors.isEmpty())
    {
        qDebug() << "Predefined QT_SCREEN_SCALE_FACTORS found:" << envValueQtScreenScaleFactors;

        bool screenScaleFactorsValid = true;
        for (const auto& screenInfo: mScreensInfo)
        {
            if (!envValueQtScreenScaleFactors.contains(screenInfo.name))
            {
                screenScaleFactorsValid = false;
                QString msg =
                    QString::fromUtf8("Screen name ") + screenInfo.name +
                    QString::fromUtf8(" not found in predefined QT_SCREEN_SCALE_FACTORS: ") +
                    envValueQtScreenScaleFactors;

                qDebug() << msg;
                mLogMessages.push_back(msg);
                break;
            }
        }

        if (screenScaleFactorsValid)
        {
            qDebug() << "Not setting scale factors. Using predefined QT_SCREEN_SCALE_FACTORS="
                     << envValueQtScreenScaleFactors;
            QString msg = QString::fromUtf8("Scale factor not calculated because "
                                            "QT_SCREEN_SCALE_FACTORS is already set to: ") +
                          envValueQtScreenScaleFactors;
            mLogMessages.push_back(msg);
            return true;
        }
    }
    return false;
}

double adjustScaleValueToSuitableIncrement(double scale, double maxScale)
{
    scale = std::min(scale, maxScale);
    if (maxScale > 1.)
    {
        auto dpiScreensSuitableStep = 0.25; // Move by .25 steps
        scale =
            qRound(scale / dpiScreensSuitableStep) * dpiScreensSuitableStep; // Set to nearest step

        while (scale > maxScale)
        {
            scale -= dpiScreensSuitableStep;
        }
    }
    return scale;
}

double calculateMaxScale(const ScreenInfo& screenInfo, bool multiScreen)
{
    // This is the height of the biggest dialog in megassync (Settings) + headroom
    constexpr auto maxDialogHeight = 810;

    // We use different env vars to set scaling depending on the number of screens:
    // It seems the scaling has not the same effect depending on the variable used...
    // So we compute the scale differently.
    // TODO: revamp all scaling code
    return screenInfo.availableHeightPixels * (multiScreen ? screenInfo.devicePixelRatio : 1.)
           / static_cast<double>(maxDialogHeight);
}

bool ScaleFactorManager::computeScales()
{
    bool needsRescaling = false;
    const bool multiScreen = mScreensInfo.size() > 1;
    for(auto& screenInfo : mScreensInfo)
    {
        auto scale = multiScreen ? screenInfo.devicePixelRatio : 1.;
        if(mOsType == OsType::LINUX)
        {
            scale = computeScaleLinux(screenInfo);
        }
        const auto maxScale = calculateMaxScale(screenInfo, multiScreen);
        scale = adjustScaleValueToSuitableIncrement(scale, maxScale);

        const bool hdpiAutoEnabled{screenInfo.devicePixelRatio > 1.0};
        const bool scaleNeedsToBeAdjusted{scale < 1.0 && !hdpiAutoEnabled};
        if(scaleNeedsToBeAdjusted)
        {
            scale = 1.0;
        }

        if ((!multiScreen && scale != 1.0) || (multiScreen && scale != screenInfo.devicePixelRatio))
        {
            needsRescaling = true;
            qDebug() << "Screen " << screenInfo.name << " needs rescaling: " << scale;
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
