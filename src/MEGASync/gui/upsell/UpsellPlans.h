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
    Q_PROPERTY(int currentDiscount READ getCurrentDiscount NOTIFY currentDiscountChanged)

public:
    enum BackupFolderRoles
    {
        NAME_ROLE = Qt::UserRole + 1,
        RECOMMENDED_ROLE,
        STORAGE_ROLE,
        TRANSFER_ROLE,
        PRICE_ROLE,
        SELECTED_ROLE
    };

    explicit UpsellPlans(QObject* parent = nullptr);
    virtual ~UpsellPlans() = default;

    class Data
    {
    public:
        class AccountBillingPlanData
        {
        public:
            AccountBillingPlanData();
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

        Data(int proLevel, bool recommended);

        static QHash<int, QByteArray> roleNames();

        int proLevel() const;
        bool recommended() const;
        const AccountBillingPlanData& monthlyData() const;
        const AccountBillingPlanData& yearlyData() const;
        bool selected() const;
        int discount() const;

        void setSelected(bool newChecked);
        void setMonthlyData(const AccountBillingPlanData& newMonthlyData);
        void setYearlyData(const AccountBillingPlanData& newYearlyData);

    private:
        int mProLevel;
        bool mRecommended;
        bool mSelected;
        AccountBillingPlanData mMonthlyData;
        AccountBillingPlanData mYearlyData;
    };

    bool addPlan(std::shared_ptr<Data> plan);

    QList<std::shared_ptr<Data>> plans() const;
    std::shared_ptr<UpsellPlans::Data> getPlan(int index) const;
    std::shared_ptr<UpsellPlans::Data> getPlanByProLevel(int proLevel) const;
    int size() const;
    QString getCurrencySymbol() const;
    bool isMonthly() const;
    int getCurrentDiscount() const;

    void setCurrencySymbol(const QString& symbol);
    void setMonthly(bool monthly);
    void setCurrentPlanSelected(int row);
    void deselectCurrentPlanSelected();

    int currentPlanSelected() const;

signals:
    void currencySymbolChanged();
    void monthlyChanged();
    void currentDiscountChanged();

private:
    QList<std::shared_ptr<Data>> mPlans;
    QString mCurrencySymbol;
    bool mMonthly;
    int mCurrentPlanSelected;
};

#endif // UPSELL_PLANS_H
