#ifndef UPSELL_PLANS_H
#define UPSELL_PLANS_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>

#include <memory>

class UpsellPlans: public QObject
{
    Q_OBJECT

    Q_PROPERTY(ViewMode viewMode READ getViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(bool monthly READ isMonthly NOTIFY monthlyChanged)
    Q_PROPERTY(bool billingCurrency READ isBillingCurrency NOTIFY isCurrencyBillingChanged)
    Q_PROPERTY(int currentDiscount READ getCurrentDiscount NOTIFY currentDiscountChanged)
    Q_PROPERTY(QString currencyName READ getCurrencyName NOTIFY currencyChanged)
    Q_PROPERTY(
        QString transferRemainingTime READ getTransferRemainingTime NOTIFY remainingTimeChanged)
    Q_PROPERTY(int plansCount READ size NOTIFY sizeChanged)
    Q_PROPERTY(
        bool onlyProFlexiAvailable READ isOnlyProFlexiAvailable NOTIFY onlyProFlexiAvailableChanged)

public:
    enum class ViewMode
    {
        NONE = 0,
        STORAGE_ALMOST_FULL = 1,
        STORAGE_FULL = 2,
        TRANSFER_EXCEEDED = 3
    };
    Q_ENUM(ViewMode)

    enum UpsellPlanRoles
    {
        NAME_ROLE = Qt::UserRole + 1,
        BUTTON_NAME_ROLE,
        RECOMMENDED_ROLE,
        STORAGE_ROLE,
        TRANSFER_ROLE,
        PRICE_ROLE,
        TOTAL_PRICE_WITHOUT_DISCOUNT_ROLE,
        MONTHLY_PRICE_WITH_DISCOUNT_ROLE,
        CURRENT_PLAN_ROLE,
        AVAILABLE_ROLE,
        SHOW_PRO_FLEXI_MESSAGE,
        SHOW_ONLY_PRO_FLEXI
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

            bool isValid() const;

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
        bool isRecommended() const;
        const QString& name() const;
        const AccountBillingPlanData& monthlyData() const;
        const AccountBillingPlanData& yearlyData() const;

    private:
        int mProLevel;
        bool mRecommended;
        QString mName;
        AccountBillingPlanData mMonthlyData;
        AccountBillingPlanData mYearlyData;

        friend class UpsellPlans;
        friend class UpsellController;

        void setProLevel(int newProLevel);
        void setRecommended(bool newRecommended);
        void setMonthlyData(const AccountBillingPlanData& newMonthlyData);
        void setYearlyData(const AccountBillingPlanData& newYearlyData);
        void setName(const QString& name);
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

    void addPlans(const QList<std::shared_ptr<Data>>& plans);

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
    QString getTransferRemainingTime() const;
    long long getTransferFinishTime() const;
    bool isOnlyProFlexiAvailable() const;

signals:
    void viewModeChanged();
    void currencyChanged();
    void monthlyChanged();
    void currentDiscountChanged();
    void isCurrencyBillingChanged();
    void remainingTimeChanged();
    void sizeChanged();
    void onlyProFlexiAvailableChanged();

private:
    QList<std::shared_ptr<Data>> mPlans;
    CurrencyData mCurrency;
    ViewMode mViewMode;
    bool mIsMonthly;
    bool mIsBillingCurrency;
    int mCurrentDiscount;
    QString mTransferRemainingTime;
    long long mTransferFinishTime; // Seconds since epoch.
    bool mIsOnlyProFlexiAvailable;

    friend class UpsellController;

    void setViewMode(ViewMode viewMode);
    void setMonthly(bool monthly);
    void setBillingCurrency(bool isCurrencyBilling);
    void setCurrentDiscount(int discount);
    void setTransferRemainingTime(const QString& time);
    void setCurrency(const QString& symbol, const QString& name);
    void setTransferFinishTime(long long newTime);
    void setOnlyProFlexiAvailable(bool onlyProFlexiAvailable);
};

#endif // UPSELL_PLANS_H
