#include "SyncExclusions.h"

#include "Platform.h"
#include "Preferences.h"

#include <cmath>
#include <QQmlEngine>

using namespace mega;

bool isEqual(double a, double b, double epsilon = 1e-2) {
    return fabs(a - b) < epsilon;
}

SyncExclusions::SyncExclusions(QWidget *parent, const QString& path)
    : QMLComponent(parent)
    , mMinimumAllowedSize(0)
    , mMaximumAllowedSize(0)
    , mMegaIgnoreManager(std::make_shared<MegaIgnoreManager>())
    , mRulesModel(new ExclusionRulesModel(this, mMegaIgnoreManager))
{
    qmlRegisterModule("SyncExclusions", 1, 0);

    qmlRegisterUncreatableType<MegaIgnoreNameRule>(
        "WildCardEnum",
        1,
        0,
        "WildCard",
        QString::fromLatin1("WildCard is an uncreatable type"));

    setFolder(path);
}

SyncExclusions::~SyncExclusions()
{
}

QUrl SyncExclusions::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/sync_exclusions/ExclusionsQmlDialog.qml"));
}

QString SyncExclusions::contextName()
{
    return QString::fromUtf8("syncExclusionsAccess");
}

void SyncExclusions::setMaximumAllowedSize(double maximumSize)
{
    if(isEqual(maximumSize, mMaximumAllowedSize))
    {
        return;
    }
    mMaximumAllowedSize = maximumSize;

    auto highLimit = mMegaIgnoreManager->getHighLimitRule();
    if(highLimit)
    {
        auto value = fromDisplay(mMaximumAllowedSize, mMaximumAllowedUnit);
        highLimit->setValue(value.first);
        highLimit->setUnit(value.second);
    }
    emit maximumAllowedSizeChanged(mMaximumAllowedSize);
}

void SyncExclusions::setMinimumAllowedSize(double minimumSize)
{
    if(isEqual(minimumSize, mMinimumAllowedSize))
    {
        return;
    }
    mMinimumAllowedSize = minimumSize;
    auto value = fromDisplay(mMinimumAllowedSize, mMinimumAllowedUnit);
    mMegaIgnoreManager->getLowLimitRule()->setValue(value.first);
    mMegaIgnoreManager->getLowLimitRule()->setUnit(value.second);
    emit minimumAllowedSizeChanged(mMinimumAllowedSize);
}

void SyncExclusions::setMaximumAllowedUnit(int maximumUnit)
{
    if(maximumUnit == mMaximumAllowedUnit)
    {
        return;
    }
    mMaximumAllowedUnit = static_cast<MegaIgnoreSizeRule::UnitTypes>(maximumUnit);
    auto highLimit = mMegaIgnoreManager->getHighLimitRule();
    if (highLimit)
    {
        auto value = fromDisplay(mMaximumAllowedSize, mMaximumAllowedUnit);
        highLimit->setValue(value.first);
        highLimit->setUnit(value.second);
    }
    emit maximumAllowedUnitChanged(mMaximumAllowedUnit);
}

void SyncExclusions::setMinimumAllowedUnit(int minimumUnit)
{
    if(minimumUnit == mMinimumAllowedUnit)
    {
        return;
    }
    mMinimumAllowedUnit = static_cast<MegaIgnoreSizeRule::UnitTypes>(minimumUnit);
    auto lowLimit = mMegaIgnoreManager->getLowLimitRule();
    if (lowLimit)
    {
        auto value = fromDisplay(mMinimumAllowedSize, mMinimumAllowedUnit);
        lowLimit->setValue(value.first);
        lowLimit->setUnit(value.second);
    }
    emit minimumAllowedUnitChanged(mMinimumAllowedUnit);
}

void SyncExclusions::applyChanges()
{
    mMegaIgnoreManager->applyChanges(false);
}

SyncExclusions::SizeExclusionStatus SyncExclusions::getSizeExclusionStatus()  const
{
    if(!mMegaIgnoreManager)
    {
        return SyncExclusions::SizeExclusionStatus::DISABLED;
    }
    auto highLimit = mMegaIgnoreManager->getHighLimitRule();
    auto lowLimit = mMegaIgnoreManager->getLowLimitRule();
    if (highLimit && lowLimit && !highLimit->isCommented() && !lowLimit->isCommented())
    {
        return SyncExclusions::SizeExclusionStatus::OUTSIDE_OF;
    }
    if (highLimit  && !highLimit->isCommented())
    {
        return SyncExclusions::SizeExclusionStatus::BIGGER_THAN;
    }
    if (lowLimit  && !lowLimit->isCommented())
    {
        return SyncExclusions::SizeExclusionStatus::SMALLER_THAN;
    }
    return SyncExclusions::SizeExclusionStatus::DISABLED;
}

void SyncExclusions::setSizeExclusionStatus(SyncExclusions::SizeExclusionStatus status)
{
    if(!mMegaIgnoreManager)
    {
        return;
    }
    auto highLimit = mMegaIgnoreManager->getHighLimitRule();
    auto lowLimit = mMegaIgnoreManager->getLowLimitRule();
    switch (status)
    {
    case SyncExclusions::SizeExclusionStatus::OUTSIDE_OF:
    {
        if(highLimit)
        {
            highLimit->setCommented(false);
        }
        if(lowLimit)
        {
            lowLimit->setCommented(false);
        }
    }
    break;
    case SyncExclusions::SizeExclusionStatus::DISABLED:
    {
        if(highLimit)
        {
            highLimit->setCommented(true);
        }
        if(lowLimit)
        {
            lowLimit->setCommented(true);
        }
    }
    break;
    case SyncExclusions::SizeExclusionStatus::SMALLER_THAN:
    {
        if(highLimit)
        {
            highLimit->setCommented(true);
        }
        if(lowLimit)
        {
            lowLimit->setCommented(false);
        }
    }
    break;
    case SyncExclusions::SizeExclusionStatus::BIGGER_THAN:
    {
        if(highLimit)
        {
            highLimit->setCommented(false);
        }
        if(lowLimit)
        {
            lowLimit->setCommented(true);
        }
    }
    break;
    default:
        break;
    }
}

void SyncExclusions::setFolder(const QString& folderName)
{
    mFolderName = QDir(folderName).dirName();
    mFolderFullPath = folderName;
    mRulesModel->setFolder(folderName);
    mMegaIgnoreManager = mRulesModel->getMegaIgnoreManager();
    auto highLimit = mMegaIgnoreManager->getHighLimitRule();
    emit sizeExclusionStatusChanged(getSizeExclusionStatus());
    emit folderNameChanged(mFolderName);
    if (highLimit)
    {
        auto displayValue = toDisplay(highLimit->value(), highLimit->unit());
        setMaximumAllowedSize(displayValue.first);
        setMaximumAllowedUnit(displayValue.second);
    }
    auto lowLimit = mMegaIgnoreManager->getLowLimitRule();
    if (lowLimit)
    {
        auto displayValue = toDisplay(lowLimit->value(), lowLimit->unit());
        setMinimumAllowedSize(displayValue.first);
        setMinimumAllowedUnit(displayValue.second);
    }
}

void SyncExclusions::restoreDefaults()
{
    mMegaIgnoreManager->restreDefaults();
}

bool SyncExclusions::isAskOnExclusionRemove()  const
{
    return Preferences::instance()->isAskOnExclusionRemove();
}

void SyncExclusions::setAskOnExclusionRemove(bool value)
{
    Preferences::instance()->setAskOnExclusionRemove(value);
}

std::pair<unsigned long long, int> SyncExclusions::fromDisplay(double value , int unit) const
{
    double whole, fractional;
    fractional = std::modf(value, &whole);

    if(unit == MegaIgnoreSizeRule::B || isEqual(fractional, 0.0))
    {
        auto truncatedValue(static_cast<unsigned long long>(trunc(value)));
        return {truncatedValue, unit};
    }

    auto nextUnitTruncatedValue(trunc(value * 1024.0));
    --unit;
    return {static_cast<unsigned long long>(nextUnitTruncatedValue), unit};
}


std::pair<double, int> SyncExclusions::toDisplay(unsigned long long value , int unit) const
{
    double doubleValue = static_cast<double>(value);
    int updatedUnit = unit;
    for (int unitIterator = unit; unitIterator < MegaIgnoreSizeRule::G; unitIterator++)
    {
        if(doubleValue <= 999)
        {
            return {doubleValue, unitIterator};
        }
        doubleValue = doubleValue/1024.;
        ++updatedUnit;
    }
    return {doubleValue, updatedUnit};
}
