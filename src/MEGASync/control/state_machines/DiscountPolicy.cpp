#include "DiscountPolicy.h"

#include "StatsEventHandler.h"
#include "Utilities.h"

DiscountPolicy::DiscountPolicy(QObject* parent):
    QObject(parent),
    mPreferences(Preferences::instance())
{
    if (load())
    {
        checkAndDeactivateExpiredCampaign();
    }
}

void DiscountPolicy::activateCampaign(std::shared_ptr<mega::MegaDiscountCodeInfo> discountInfo)
{
    // We have several cases to take into account here:
    // - no stored code + no running campaign: start new campaign
    // - running campaign + same code: do nothing
    // - no running campaign + stored code + same new code: restore campaign
    // - stored code + expired stored campaign + new code: stop old campaign and start new
    // - running campaign + new code: stop old campaign and start new
    // - stored code + expired stored campaign + new code + expired new campaign: stop old campaign

    if (!discountInfo)
    {
        return;
    }

    bool deactivateCampaignFirst = false;
    bool activateCampaign = false;

    // Check persisted campaign
    if ((mIsLoadingPersistedDataNeeded && load()) ||
        (!mIsLoadingPersistedDataNeeded && !mIsCampaignActive && !mDiscountCode.isEmpty()))
    {
        // We want to deactivate an expired persisted campaign
        deactivateCampaignFirst = isCampaignExpiredUtc(mCampaignExpiryDateUtc);
        // And activate it if it's not expired
        activateCampaign = !deactivateCampaignFirst;
    }

    const auto newCode = QString::fromUtf8(discountInfo->getCode());
    const auto isNewCampaign = newCode != mDiscountCode;

    // If it's a new campaign we want to activate it
    activateCampaign |= isNewCampaign;

    const auto newExpiryDateUtc = QDateTime::fromSecsSinceEpoch(discountInfo->getExpiry(), Qt::UTC);
    const auto isNewCampaignExpired =
        isCampaignExpiredUtc(newExpiryDateUtc); // Very low probability

    // We want to deactivate an active, but different campaign, or if the new campaign is already
    // expired when we process the info.
    deactivateCampaignFirst |= mIsCampaignActive && (isNewCampaign || isNewCampaignExpired);

    // Deactivate if needed
    if (deactivateCampaignFirst)
    {
        deactivateCampaign();
    }

    // Evaluate here because deactivating can change mIsCampaignActive value.
    // We want to activate only if the campaign is not already running, or if the new campaign is
    // already expired when we process the info.
    activateCampaign &= !(mIsCampaignActive || isNewCampaignExpired);

    // Activate if needed
    if (activateCampaign)
    {
        if (isNewCampaign)
        {
            MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
                AppStatsEvents::EventType::TARGETED_DISCOUNT_CAMPAIGN_STARTED);
            mDiscountCode = newCode;
        }
        // Update in case it has changed (not very probable)
        mCampaignExpiryDateUtc = newExpiryDateUtc;
        persist();
        mDiscountInfo = discountInfo;
        mIsCampaignActive = true;

        emit campaignActivated();
    }
}

void DiscountPolicy::deactivateCampaign()
{
    // We want to send an event if the campaign was runing or we had a persisted campaign pending
    // re-activation
    auto wasCampaignActive = mIsCampaignActive || (!mIsLoadingPersistedDataNeeded &&
                                                   !mIsCampaignActive && !mDiscountCode.isEmpty());

    mIsCampaignActive = false;
    mDiscountInfo.reset();

    mDiscountCode.clear();
    mCampaignExpiryDateUtc = QDateTime();
    mLastTimeShownUtc = QDateTime();
    persist();

    if (wasCampaignActive)
    {
        MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
            AppStatsEvents::EventType::TARGETED_DISCOUNT_CAMPAIGN_STOPPED);
        emit campaignDeactivated();
    }
}

bool DiscountPolicy::isCampaignActive() const
{
    return mIsCampaignActive && mDiscountInfo;
}

void DiscountPolicy::recordShown()
{
    mLastTimeShownUtc = QDateTime::currentDateTimeUtc();
    persist();
    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
        AppStatsEvents::EventType::TARGETED_DISCOUNT_DIALOG_SHOWN);
}

void DiscountPolicy::recordDismissed()
{
    mLastTimeShownUtc = QDateTime::currentDateTimeUtc();
    persist();
    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
        AppStatsEvents::EventType::TARGETED_DISCOUNT_DIALOG_DISMISSED);
}

void DiscountPolicy::recordAccepted()
{
    mLastTimeShownUtc = QDateTime::currentDateTimeUtc();
    persist();
    MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
        AppStatsEvents::EventType::TARGETED_DISCOUNT_DIALOG_DEAL_GRABBED);
}

QDateTime DiscountPolicy::getDialogLastShownDateUtc() const
{
    return mLastTimeShownUtc;
}

std::shared_ptr<mega::MegaDiscountCodeInfo> DiscountPolicy::getDiscountInfo()
{
    return mDiscountInfo;
}

QDateTime DiscountPolicy::getExpiryDateUtc() const
{
    return mCampaignExpiryDateUtc;
}

bool DiscountPolicy::load()
{
    if (mPreferences->logged())
    {
        mLastTimeShownUtc = mPreferences->getOfferDialogLastExecution();
        mDiscountCode = mPreferences->getDiscountCode();
        mCampaignExpiryDateUtc = mPreferences->getOfferDialogCampaignExpiryDate();
        mIsLoadingPersistedDataNeeded = false;
    }
    return !mDiscountCode.isEmpty();
}

void DiscountPolicy::persist() const
{
    if (mPreferences->logged())
    {
        mPreferences->setOfferDialogLastExecution(mLastTimeShownUtc);
        mPreferences->setDiscountCode(mDiscountCode);
        mPreferences->setOfferDialogCampaignExpiryDate(mCampaignExpiryDateUtc);
    }
}

void DiscountPolicy::checkAndDeactivateExpiredCampaign()
{
    if (isCampaignExpiredUtc(mCampaignExpiryDateUtc))
    {
        deactivateCampaign();
    }
}

bool DiscountPolicy::isCampaignExpiredUtc(const QDateTime& expiryDateUtc)
{
    return expiryDateUtc <= QDateTime::currentDateTimeUtc();
}
