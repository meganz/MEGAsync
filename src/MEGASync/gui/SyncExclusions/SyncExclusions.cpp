#include "SyncExclusions.h"
#include "ExclusionsQmlDialog.h"
#include <QQmlEngine>
#include "Platform.h"
#include "Preferences/Preferences.h"

using namespace mega;

SyncExclusions::SyncExclusions(QWidget *parent, const QString& path)
    : QMLComponent(parent)
    , mMinimumAllowedSize(0)
    , mMaximumAllowedSize(0)
    , mMegaIgnoreManager(std::make_shared<MegaIgnoreManager>())
    , mRulesModel(new ExclusionRulesModel(this, mMegaIgnoreManager))
{
    qmlRegisterModule("SyncExclusions", 1, 0);
    qmlRegisterType<ExclusionsQmlDialog>("ExclusionsQmlDialog", 1, 0, "ExclusionsQmlDialog");
    qmlRegisterUncreatableType<MegaIgnoreNameRule>("WildCardEnum", 1, 0, "WildCard", QString::fromLatin1("MyEnum is an uncreatable type"));

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

void SyncExclusions::setMaximumAllowedSize(int maximumSize)
{
    if(maximumSize == mMaximumAllowedSize)
    {
        return;
    }
    mMaximumAllowedSize = maximumSize;

    auto highLimit = mMegaIgnoreManager->getHighLimitRule();
    highLimit->setValue(maximumSize);
    emit maximumAllowedSizeChanged(mMaximumAllowedSize);
}

void SyncExclusions::setMinimumAllowedSize(int minimumSize)
{
    if(minimumSize == mMinimumAllowedSize)
    {
        return;
    }
    mMinimumAllowedSize = minimumSize;
    mMegaIgnoreManager->getLowLimitRule()->setValue(mMinimumAllowedSize);
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
        highLimit->setUnit(mMaximumAllowedUnit);
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
        lowLimit->setUnit(mMinimumAllowedUnit);
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
        return SyncExclusions::SizeExclusionStatus::SMALLER_THAN;
    }
    if (lowLimit  && !lowLimit->isCommented())
    {
        return SyncExclusions::SizeExclusionStatus::BIGGER_THAN;
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
    case SyncExclusions::SizeExclusionStatus::BIGGER_THAN:
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
    case SyncExclusions::SizeExclusionStatus::SMALLER_THAN:
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
        setMaximumAllowedSize(highLimit->value());
        setMaximumAllowedUnit(highLimit->unit());
    }
    auto lowLimit = mMegaIgnoreManager->getLowLimitRule();
    if (lowLimit)
    {
        setMinimumAllowedSize(lowLimit->value());
        setMinimumAllowedUnit(lowLimit->unit());
    }
}

void SyncExclusions::restoreDefaults()
{
    mMegaIgnoreManager->restreDefaults();
}

void SyncExclusions::chooseFile()
{
    SelectorInfo info;
    info.defaultDir = mFolderFullPath;
    info.multiSelection = false;
    info.title = tr("Select the file you want to exclude");
    info.func = [this](QStringList selection){
        if(!selection.isEmpty())
        {
            const auto absolutePath = QDir::toNativeSeparators(selection.first());
            const auto relativePath = QDir (mFolderFullPath).relativeFilePath(absolutePath);
            emit fileChoosen(relativePath);
        }
    };
    Platform::getInstance()->fileSelector(info);
}

void SyncExclusions::chooseFolder()
{
    auto processResult = [this](const QStringList& selection){
        if(!selection.isEmpty())
        {
            const auto absolutePath = QDir::toNativeSeparators(selection.first());
            const auto relativePath = QDir(mFolderFullPath).relativeFilePath(absolutePath);
            emit folderChoosen(relativePath);
        }
    };
    SelectorInfo info;
    info.defaultDir = mFolderFullPath;
    info.multiSelection = false;
    info.func = processResult;
    info.title = tr("Select the folder you want to exclude");
    Platform::getInstance()->folderSelector(info);
}

bool SyncExclusions::isAskOnExclusionRemove()  const
{
    return Preferences::instance()->isAskOnExclusionRemove();
}

void SyncExclusions::setAskOnExclusionRemove(bool value)
{
    Preferences::instance()->setAskOnExclusionRemove(value);
}
