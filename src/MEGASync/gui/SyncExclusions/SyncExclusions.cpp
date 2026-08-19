#include "SyncExclusions.h"

#include "MessageDialogOpener.h"
#include "Preferences.h"
#include "SyncController.h"

#include <QPointer>
#include <QQmlEngine>

#include <cmath>
#include <utility>

using namespace mega;

bool isEqual(double a, double b, double epsilon = 1e-2) {
    return fabs(a - b) < epsilon;
}

SyncExclusions::SyncExclusions(QWidget* parent,
                               const QString& path,
                               std::shared_ptr<SyncSettings> syncSetting):
    QMLComponent(parent),
    mMinimumAllowedSize(0),
    mMaximumAllowedSize(0),
    mMegaIgnoreManager(std::make_shared<MegaIgnoreManager>()),
    mRulesModel(new ExclusionRulesModel(this, mMegaIgnoreManager)),
    mSyncSetting(std::move(syncSetting))
{
    qmlRegisterModule("SyncExclusions", 1, 0);

    qmlRegisterUncreatableType<MegaIgnoreNameRule>(
        "WildCardEnum",
        1,
        0,
        "WildCard",
        QString::fromLatin1("WildCard is an uncreatable type"));

    qmlRegisterUncreatableMetaObject(
        ExclusionRulesModel::staticMetaObject,
        "ExclusionRulesModel",
        1,
        0,
        "ExclusionRulesModel",
        QString::fromUtf8("ExclusionRulesModel is not meant to be created"));

    // Rule changes in the model can make the setup match or diverge from the defaults
    connect(mRulesModel,
            &QAbstractItemModel::rowsInserted,
            this,
            &SyncExclusions::isDefaultChanged);
    connect(mRulesModel, &QAbstractItemModel::rowsRemoved, this, &SyncExclusions::isDefaultChanged);
    connect(mRulesModel, &QAbstractItemModel::dataChanged, this, &SyncExclusions::isDefaultChanged);
    connect(mRulesModel, &QAbstractItemModel::modelReset, this, &SyncExclusions::isDefaultChanged);

    setFolder(path);
}

SyncExclusions::~SyncExclusions()
{
}

QUrl SyncExclusions::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/sync_exclusions/ExclusionsQmlDialog.qml"));
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
    emit isDefaultChanged();
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
    emit isDefaultChanged();
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
    emit isDefaultChanged();
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
    emit isDefaultChanged();
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
    emit isDefaultChanged();
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
    emit isDefaultChanged();
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
    // Captured by value (not [this]) because pauseRunAndResume() blocks on a nested event
    // loop, during which this dialog could be closed and destroyed.
    auto megaIgnoreManager = mMegaIgnoreManager;

    if (!megaIgnoreManager->hasDefaultFile() && mSyncSetting)
    {
        // There's no .megaignore.default to copy from, so restoreDefaults() below will
        // remove .megaignore instead and rely on the SDK to regenerate it. Pause the sync
        // first so it can't see the folder with .megaignore momentarily missing and start
        // syncing files that should stay excluded; resume afterwards so the SDK's
        // transition into RUNSTATE_RUNNING actually regenerates the file.
        SyncController::instance().pauseRunAndResume(mSyncSetting,
                                                     [megaIgnoreManager]()
                                                     {
                                                         megaIgnoreManager->restoreDefaults();
                                                     });
    }
    else
    {
        megaIgnoreManager->restoreDefaults();
    }
}

bool SyncExclusions::isDefault() const
{
    return mMegaIgnoreManager && mMegaIgnoreManager->isDefault();
}

void SyncExclusions::showRemoveRuleConfirmationMessageDialog(const QString& descriptionText)
{
    MessageDialogInfo msgInfo;
    msgInfo.titleText = QCoreApplication::translate("ExclusionsStrings", "Remove this exclusion?");
    msgInfo.descriptionText = descriptionText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::Ok,
                         QCoreApplication::translate("ExclusionsStrings", "Remove"));
    textsByButton.insert(QMessageBox::Cancel,
                         QCoreApplication::translate("ExclusionsStrings", "Cancel"));
    msgInfo.buttonsText = textsByButton;
    msgInfo.defaultButton = QMessageBox::Ok;
    msgInfo.checkboxText = QCoreApplication::translate("ExclusionsStrings", "Don’t ask me again");
    msgInfo.checkboxChecked = true;
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Ok)
        {
            emit acceptedClicked();
            setAskOnExclusionRemove(!msg->isChecked());
        }
    };
    MessageDialogOpener::warning(msgInfo);
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
