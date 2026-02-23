#pragma once

#include "megaapi.h"
#include "Preferences.h"
#include "UpsellController.h"

#include <QDateTime>
#include <QObject>
#include <QTimer>

class DiscountPolicy: public QObject
{
    Q_OBJECT

public:
    explicit DiscountPolicy(QObject* parent = nullptr);

    void activateCampaign(std::shared_ptr<mega::MegaDiscountCodeInfo> discountInfo);
    void deactivateCampaign();
    bool isCampaignActive() const;
    void recordShown();
    void recordDismissed();
    void recordAccepted();
    QDateTime getDialogLastShownDateUtc() const;
    std::shared_ptr<mega::MegaDiscountCodeInfo> getDiscountInfo();
    QDateTime getExpiryDateUtc() const;
    QString getPlanName() const;

signals:
    void campaignActivated();
    void campaignDeactivated();
    void planNameReady();

protected slots:
    void setPlanName();

protected:
    bool load();
    void persist() const;
    void checkAndDeactivateExpiredCampaign();
    bool isCampaignExpired(const QDateTime& expiryDate);

    // State
    bool mIsCampaignActive = false;
    bool mIsLoadingPersistedDataNeeded = true;
    QDateTime mLastTimeShownUtc;
    QDateTime mCampaignExpiryDateUtc;
    QString mDiscountCode;
    QString mPlanName;

    std::shared_ptr<mega::MegaDiscountCodeInfo> mDiscountInfo;
    std::shared_ptr<Preferences> mPreferences;
    QPointer<UpsellController> mUpsellController;
};
