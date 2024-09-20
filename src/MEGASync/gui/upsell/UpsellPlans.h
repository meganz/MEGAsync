#ifndef UPSELL_PLANS_H
#define UPSELL_PLANS_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>

class UpsellPlans: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currencySymbol READ getCurrencySymbol WRITE setCurrencySymbol NOTIFY
                   currencySymbolChanged)
    Q_PROPERTY(bool monthly READ isMonthly WRITE setMonthly NOTIFY monthlyChanged)

public:
    enum BackupFolderRoles
    {
        NAME_ROLE = Qt::UserRole + 1,
        RECOMMENDED_ROLE,
        STORAGE_ROLE,
        TRANSFER_ROLE,
        PRICE_ROLE
    };

    explicit UpsellPlans(QObject* parent = nullptr);
    virtual ~UpsellPlans() = default;

    class Data
    {
    public:
        class AccountBillingPlanData
        {
        public:
            AccountBillingPlanData(int gbStorage, int gbTransfer, int price);
            ~AccountBillingPlanData() = default;

            int64_t gBStorage() const;
            int64_t gBTransfer() const;
            float price() const;

        private:
            int64_t mGBStorage;
            int64_t mGBTransfer;
            float mPrice;
        };

        Data(int proLevel,
             bool recommended,
             const AccountBillingPlanData& monthlyData,
             const AccountBillingPlanData& yearlyData);

        static QHash<int, QByteArray> roleNames();

        int proLevel() const;
        bool recommended() const;
        const AccountBillingPlanData& monthlyData() const;
        const AccountBillingPlanData& yearlyData() const;

    private:
        int mProLevel;
        bool mRecommended;
        AccountBillingPlanData mMonthlyData;
        AccountBillingPlanData mYearlyData;
    };

    void addPlan(std::shared_ptr<Data> plan);
    std::shared_ptr<UpsellPlans::Data> getPlan(int index) const;
    int size() const;

    QString getCurrencySymbol() const;
    void setCurrencySymbol(const QString& symbol);
    bool isMonthly() const;
    void setMonthly(bool monthly);

signals:
    void currencySymbolChanged();
    void monthlyChanged();

private:
    QList<std::shared_ptr<Data>> mPlans;
    QString mCurrencySymbol;
    bool mMonthly;
};

#endif // UPSELL_PLANS_H
