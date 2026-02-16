#pragma once

#include "megaapi.h"
#include "Preferences.h"

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

signals:
    void campaignActivated();
    void campaignDeactivated();

private:
    void load();
    void persist() const;

private:
    // State
    bool mIsCampaignActive = false;
    QDateTime mLastTimeShownUtc;
    QDateTime mCampaignExpiryDateUtc;
    QString mDiscountCode;

    std::shared_ptr<mega::MegaDiscountCodeInfo> mDiscountInfo;
    std::shared_ptr<Preferences> mPreferences;
};
