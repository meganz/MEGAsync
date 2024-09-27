#ifndef UPSELL_PLANS_H
#define UPSELL_PLANS_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>

class UpsellPlans: public QObject
{
    Q_OBJECT

    Q_PROPERTY(ViewMode viewMode READ getViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(bool monthly READ isMonthly NOTIFY monthlyChanged)
    Q_PROPERTY(bool billingCurrency READ isBillingCurrency NOTIFY isCurrencyBillingChanged)
    Q_PROPERTY(int currentDiscount READ getCurrentDiscount NOTIFY currentDiscountChanged)
    Q_PROPERTY(QString currencyName READ getCurrencyName NOTIFY currencyChanged)
    Q_PROPERTY(QString currentPlanName READ getCurrentPlanName NOTIFY currentPlanNameChanged)
    Q_PROPERTY(
        QString transferRemainingTime READ getTransferRemainingTime NOTIFY remainingTimeChanged)

public:
    enum class ViewMode
    {
        NONE = 0,
        STORAGE_ALMOST_FULL = 1,
        STORAGE_FULL = 2,
        TRANSFER_EXCEEDED = 3
    };
    Q_ENUM(ViewMode)

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
            AccountBillingPlanData(int64_t gbStorage, int64_t gbTransfer, float price);
            ~AccountBillingPlanData() = default;

            int64_t gBStorage() const;
            int64_t gBTransfer() const;
            float price() const;

        private:
            int64_t mGBStorage;
            int64_t mGBTransfer;
            float mPrice;
        };

        Data(int proLevel, const QString& name);

        static QHash<int, QByteArray> roleNames();

        int proLevel() const;
        bool selected() const;
        bool isRecommended() const;
        const QString& name() const;
        const AccountBillingPlanData& monthlyData() const;
        const AccountBillingPlanData& yearlyData() const;

    private:
        int mProLevel;
        bool mRecommended;
        bool mSelected;
        QString mName;
        AccountBillingPlanData mMonthlyData;
        AccountBillingPlanData mYearlyData;

        friend class UpsellPlans;
        friend class UpsellController;

        void setSelected(bool newChecked);
        void setRecommended(bool newRecommended);
        void setMonthlyData(const AccountBillingPlanData& newMonthlyData);
        void setYearlyData(const AccountBillingPlanData& newYearlyData);
    };

    class CurrencyData
    {
    public:
        explicit CurrencyData() = default;

        QString currencySymbol() const;
        QString currencyName() const;

    private:
        QString mCurrencySymbol;
        QString mCurrencyName;

        friend class UpsellPlans;

        void setCurrencySymbol(const QString& newCurrencySymbol);
        void setCurrencyName(const QString& newCurrencyName);
    };

    bool addPlan(std::shared_ptr<Data> plan);

    QList<std::shared_ptr<Data>> plans() const;
    std::shared_ptr<UpsellPlans::Data> getPlan(int index) const;
    std::shared_ptr<UpsellPlans::Data> getPlanByProLevel(int proLevel) const;
    int size() const;

    ViewMode getViewMode() const;
    bool isMonthly() const;
    bool isBillingCurrency() const;
    int getCurrentDiscount() const;
    QString getCurrencySymbol() const;
    QString getCurrencyName() const;
    QString getCurrentPlanName() const;
    QString getTransferRemainingTime() const;
    int getCurrentPlanSelected() const;
    long long getTransferFinishTime() const;

signals:
    void viewModeChanged();
    void currencyChanged();
    void monthlyChanged();
    void currentDiscountChanged();
    void isCurrencyBillingChanged();
    void currentPlanNameChanged();
    void remainingTimeChanged();

private:
    QList<std::shared_ptr<Data>> mPlans;
    CurrencyData mCurrency;
    ViewMode mViewMode;
    bool mIsMonthly;
    bool mIsBillingCurrency;
    int mCurrentPlanSelected;
    int mCurrentDiscount;
    QString mCurrentPlanName;
    QString mTransferRemainingTime;
    long long mTransferFinishTime; // Seconds since epoch.

    friend class UpsellController;

    void setViewMode(ViewMode viewMode);
    void setMonthly(bool monthly);
    void setBillingCurrency(bool isCurrencyBilling);
    void setCurrentDiscount(int discount);
    void setCurrentPlanName(const QString& name);
    void setTransferRemainingTime(const QString& time);
    void setCurrentPlanSelected(int row);
    void setCurrency(const QString& symbol, const QString& name);
    void setTransferFinishTime(long long newTime);
};

#endif // UPSELL_PLANS_H
