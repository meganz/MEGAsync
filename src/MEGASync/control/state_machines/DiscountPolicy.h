#pragma once

#include "megaapi.h"
#include "Preferences.h"

#include <QDateTime>
#include <QObject>
#include <QString>

#include <memory>

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

signals:
    void campaignActivated();
    void campaignDeactivated();

protected:
    bool load();
    void persist() const;
    void checkAndDeactivateExpiredCampaign();
    bool isCampaignExpiredUtc(const QDateTime& expiryDateUtc);
    static bool isDiscountValid(const std::shared_ptr<mega::MegaDiscountCodeInfo>& discountInfo);

    // State
    bool mIsCampaignActive = false;
    bool mIsLoadingPersistedDataNeeded = true;
    QDateTime mLastTimeShownUtc;
    QDateTime mCampaignExpiryDateUtc;
    QString mDiscountCode;

    std::shared_ptr<mega::MegaDiscountCodeInfo> mDiscountInfo;
    std::shared_ptr<Preferences> mPreferences;
};
