#include "AccountState.h"

#include "control/TextDecorator.h"
#include "megaapi.h"
#include "Preferences.h"
#include "Utilities.h"

#include <QCoreApplication>

#include <algorithm>

namespace
{
QString decorateTooltipText(const QString& text)
{
    auto decoratedText = text;
    Text::NewLine().process(decoratedText);
    return decoratedText;
}

QVariantMap buildUsageSegment(AccountState::UsageSegmentType type,
                              long long value,
                              const QVariantList& children = QVariantList())
{
    QVariantMap segment;
    segment.insert(QString::fromLatin1("type"), static_cast<int>(type));
    segment.insert(QString::fromLatin1("value"), QVariant::fromValue(value));
    segment.insert(QString::fromLatin1("sizeText"), Utilities::getSizeString(value));
    if (!children.isEmpty())
    {
        segment.insert(QString::fromLatin1("children"), children);
    }
    return segment;
}

void appendUsageSegment(QVariantList& segments,
                        AccountState::UsageSegmentType type,
                        long long value)
{
    if (value <= 0)
    {
        return;
    }

    segments.push_back(buildUsageSegment(type, value));
}

QVariantList buildStorageSegments(Preferences* preferences)
{
    const auto cloudDriveStorage = preferences->cloudDriveStorage();
    const auto versionsStorage = preferences->versionsStorage();
    const auto availableStorage =
        std::max(0ll, preferences->totalStorage() - preferences->usedStorage());
    const auto isBusinessAccount = Utilities::isBusinessAccount();

    QVariantList segments;
    if (cloudDriveStorage > 0 || versionsStorage > 0)
    {
        QVariantList children;
        appendUsageSegment(children, AccountState::Versions, versionsStorage);

        segments.push_back(
            buildUsageSegment(AccountState::CloudDrive, cloudDriveStorage, children));
    }
    appendUsageSegment(segments, AccountState::Backups, preferences->vaultStorage());
    appendUsageSegment(segments, AccountState::RubbishBin, preferences->rubbishStorage());
    if (!isBusinessAccount)
    {
        appendUsageSegment(segments, AccountState::Free, availableStorage);
    }

    return segments;
}

QVariantList buildTransferSegments(Preferences* preferences)
{
    QVariantList segments;
    appendUsageSegment(segments, AccountState::Downloads, preferences->usedBandwidth());

    const auto totalBandwidth = preferences->totalBandwidth();
    if (totalBandwidth > 0)
    {
        const auto availableBandwidth =
            std::max(0ll, totalBandwidth - preferences->usedBandwidth());
        appendUsageSegment(segments, AccountState::Free, availableBandwidth);
    }

    return segments;
}

AccountState::ProgressState quotaStateToProgressState(QuotaState quotaState)
{
    switch (quotaState)
    {
        case QuotaState::WARNING:
            return AccountState::WARNING;
        case QuotaState::OVERQUOTA:
        case QuotaState::FULL:
            return AccountState::FULL;
        case QuotaState::OK:
        default:
            return AccountState::OK;
    }
}

AccountState::ProgressState storageStateToProgressState(int storageState)
{
    switch (storageState)
    {
        case mega::MegaApi::STORAGE_STATE_PAYWALL:
        case mega::MegaApi::STORAGE_STATE_RED:
            return AccountState::FULL;
        case mega::MegaApi::STORAGE_STATE_ORANGE:
            return AccountState::WARNING;
        case mega::MegaApi::STORAGE_STATE_UNKNOWN:
        case mega::MegaApi::STORAGE_STATE_GREEN:
        default:
            return AccountState::OK;
    }
}
}

AccountState::AccountState(QObject* parent):
    QObject(parent)
{}

bool AccountState::showStorageCard() const
{
    return mShowStorageCard;
}

bool AccountState::showTransferCard() const
{
    return mShowTransferCard;
}

QString AccountState::storageSummary() const
{
    return mStorageSummary;
}

QString AccountState::storageFreeText() const
{
    return mStorageFreeText;
}

QString AccountState::storageFreeTooltipText() const
{
    return mStorageFreeTooltipText;
}

int AccountState::storagePercentage() const
{
    return mStoragePercentage;
}

int AccountState::storageState() const
{
    return mStorageState;
}

QVariantList AccountState::storageSegments() const
{
    return mStorageSegments;
}

bool AccountState::storageUsageOnly() const
{
    return mStorageUsageOnly;
}

QString AccountState::transferSummary() const
{
    return mTransferSummary;
}

QString AccountState::transferFreeText() const
{
    return mTransferFreeText;
}

QString AccountState::transferFreeTooltipText() const
{
    return mTransferFreeTooltipText;
}

int AccountState::transferPercentage() const
{
    return mTransferPercentage;
}

int AccountState::transferState() const
{
    return mTransferState;
}

QVariantList AccountState::transferSegments() const
{
    return mTransferSegments;
}

bool AccountState::transferValueOnly() const
{
    return mTransferValueOnly;
}

QString AccountState::transferValueText() const
{
    return mTransferValueText;
}

void AccountState::setShowStorageCard(bool showStorageCard)
{
    if (mShowStorageCard != showStorageCard)
    {
        mShowStorageCard = showStorageCard;
        emit showStorageCardChanged();
    }
}

void AccountState::setShowTransferCard(bool showTransferCard)
{
    if (mShowTransferCard == showTransferCard)
    {
        return;
    }

    mShowTransferCard = showTransferCard;
    emit showTransferCardChanged();
}

void AccountState::updateStorageData()
{
    auto preferences = Preferences::instance().get();
    if (!preferences)
    {
        return;
    }

    const auto totalStorage = preferences->totalStorage();
    const auto usedStorage = preferences->usedStorage();
    const auto availableStorage = std::max(0ll, totalStorage - usedStorage);
    const auto isBusinessAccount = Utilities::isBusinessAccount();

    QString storageSummary;
    QString availableText;
    int percentage = 0;
    auto state = AccountState::OK;
    const auto segments = buildStorageSegments(preferences);

    if (totalStorage > 0 && isBusinessAccount)
    {
        storageSummary = Utilities::createSimpleUsedString(usedStorage);
        percentage = 100;
    }
    else if (totalStorage > 0)
    {
        percentage = Utilities::partPer(usedStorage, totalStorage);
        state = storageStateToProgressState(preferences->getStorageState());
        storageSummary = Utilities::createCompleteUsedString(usedStorage, totalStorage, percentage);
        availableText = Utilities::getSizeString(availableStorage);
    }

    setStorageData(storageSummary,
                   availableText,
                   percentage,
                   state,
                   segments,
                   isBusinessAccount && totalStorage > 0);
}

void AccountState::updateTransferData(QuotaState quotaState)
{
    auto preferences = Preferences::instance().get();
    if (!preferences)
    {
        return;
    }

    const auto totalBandwidth = preferences->totalBandwidth();
    const auto usedBandwidth = preferences->usedBandwidth();
    const auto availableBandwidth = std::max(0ll, totalBandwidth - usedBandwidth);
    const auto isBusinessAccount = Utilities::isBusinessAccount();

    QString transferSummary;
    QString availableText;
    int percentage = 0;
    auto state = AccountState::OK;
    const auto segments = buildTransferSegments(preferences);
    QString transferValueText;

    if (isBusinessAccount)
    {
        transferValueText = Utilities::getSizeString(usedBandwidth);
    }
    else if (totalBandwidth > 0)
    {
        percentage = Utilities::partPer(usedBandwidth, totalBandwidth);
        state = quotaStateToProgressState(quotaState);
        transferSummary = Utilities::createCompleteUsedString(usedBandwidth,
                                                              totalBandwidth,
                                                              std::min(percentage, 100));
        availableText = Utilities::getSizeString(availableBandwidth);
    }

    setTransferData(transferSummary,
                    availableText,
                    percentage,
                    state,
                    segments,
                    isBusinessAccount,
                    transferValueText);
}

void AccountState::setStorageData(const QString& summary,
                                  const QString& freeText,
                                  int percentage,
                                  ProgressState state,
                                  const QVariantList& segments,
                                  bool usageOnly)
{
    if (mStorageSummary != summary)
    {
        mStorageSummary = summary;
        emit storageSummaryChanged();
    }

    // Translation context kept from AccountStateQuickWidget (pre SNC-6795) so the
    // existing translations still apply.
    const auto tooltipText =
        freeText.isEmpty() ?
            QString() :
            decorateTooltipText(
                QCoreApplication::translate("AccountStateQuickWidget", "Available[BR]%1")
                    .arg(freeText));
    if (mStorageFreeText != freeText)
    {
        mStorageFreeText = freeText;
        emit storageFreeTextChanged();
    }
    if (mStorageFreeTooltipText != tooltipText)
    {
        mStorageFreeTooltipText = tooltipText;
        emit storageFreeTooltipTextChanged();
    }
    if (mStoragePercentage != percentage)
    {
        mStoragePercentage = percentage;
        emit storagePercentageChanged();
    }
    if (mStorageState != state)
    {
        mStorageState = state;
        emit storageStateChanged();
    }
    if (mStorageSegments != segments)
    {
        mStorageSegments = segments;
        emit storageSegmentsChanged();
    }
    if (mStorageUsageOnly != usageOnly)
    {
        mStorageUsageOnly = usageOnly;
        emit storageUsageOnlyChanged();
    }
}

void AccountState::setTransferData(const QString& summary,
                                   const QString& freeText,
                                   int percentage,
                                   ProgressState state,
                                   const QVariantList& segments,
                                   bool valueOnly,
                                   const QString& valueText)
{
    if (mTransferSummary != summary)
    {
        mTransferSummary = summary;
        emit transferSummaryChanged();
    }

    // Translation context kept from AccountStateQuickWidget (pre SNC-6795) so the
    // existing translations still apply.
    const auto tooltipText =
        freeText.isEmpty() ?
            QString() :
            decorateTooltipText(
                QCoreApplication::translate("AccountStateQuickWidget", "Available[BR]%1")
                    .arg(freeText));
    if (mTransferFreeText != freeText)
    {
        mTransferFreeText = freeText;
        emit transferFreeTextChanged();
    }
    if (mTransferFreeTooltipText != tooltipText)
    {
        mTransferFreeTooltipText = tooltipText;
        emit transferFreeTooltipTextChanged();
    }
    if (mTransferPercentage != percentage)
    {
        mTransferPercentage = percentage;
        emit transferPercentageChanged();
    }
    if (mTransferState != state)
    {
        mTransferState = state;
        emit transferStateChanged();
    }
    if (mTransferSegments != segments)
    {
        mTransferSegments = segments;
        emit transferSegmentsChanged();
    }
    if (mTransferValueOnly != valueOnly)
    {
        mTransferValueOnly = valueOnly;
        emit transferValueOnlyChanged();
    }
    if (mTransferValueText != valueText)
    {
        mTransferValueText = valueText;
        emit transferValueTextChanged();
    }
}
