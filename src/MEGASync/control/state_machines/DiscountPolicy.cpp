#include "DiscountPolicy.h"

#include "Preferences.h"
#include "StatsEventHandler.h"
#include "Utilities.h"

DiscountPolicy::DiscountPolicy(QObject* parent):
    QObject(parent),
    mPreferences(Preferences::instance())
{
    load();
    if (mIsCampaignActive && mCampaignExpiryDateUtc <= QDateTime::currentDateTimeUtc())
    {
        deactivateCampaign();
    }
}

void DiscountPolicy::load()
{
    if (mPreferences->logged())
    {
        mLastTimeShownUtc = mPreferences->getOfferDialogLastExecution();
        mDiscountCode = mPreferences->getDiscountCode();
        mCampaignExpiryDateUtc = mPreferences->getOfferDialogCampaignExpiryDate();
    }
    mIsCampaignActive = !mDiscountCode.isEmpty();
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

void DiscountPolicy::activateCampaign(std::shared_ptr<mega::MegaDiscountCodeInfo> discountInfo)
{
    if (!discountInfo)
    {
        return;
    }

    // Load here?
    if (!mIsCampaignActive)
    {
        load();
        if (mIsCampaignActive && mCampaignExpiryDateUtc <= QDateTime::currentDateTimeUtc())
        {
            deactivateCampaign();
        }
    }

    // Check if we have a new campaign, and stop the currently running one if it's not the same
    const auto newCode = QString::fromUtf8(discountInfo->getCode());
    if (mIsCampaignActive && newCode != mDiscountCode)
    {
        deactivateCampaign();
    }

    if (!mIsCampaignActive || newCode == mDiscountCode)
    {
        // Check that the campaign has not expired
        mCampaignExpiryDateUtc = QDateTime::fromSecsSinceEpoch(discountInfo->getExpiry());
        if (mCampaignExpiryDateUtc > QDateTime::currentDateTime())
        {
            MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
                AppStatsEvents::EventType::TARGETED_DISCOUNT_CAMPAIGN_STARTED);
            mIsCampaignActive = true;
            mDiscountInfo = discountInfo;
            mDiscountCode = newCode;
            persist();
            emit campaignActivated();
        }
    }
}

void DiscountPolicy::deactivateCampaign()
{
    auto wasCampaignActive = mIsCampaignActive;

    mIsCampaignActive = false;
    mDiscountCode.clear();
    mCampaignExpiryDateUtc = QDateTime();
    mLastTimeShownUtc = QDateTime();
    mDiscountInfo.reset();

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
