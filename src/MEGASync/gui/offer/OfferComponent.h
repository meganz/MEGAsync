#include "QmlDialogWrapper.h"

#include <memory>

class UpsellController;
class UpsellPlans;

class OfferComponent: public QMLComponent
{
    Q_OBJECT
    Q_PROPERTY(QString currencySymbol READ getCurrencySymbol NOTIFY dataUpdated)
    Q_PROPERTY(QString currencyName READ getCurrencyName NOTIFY dataUpdated)
    Q_PROPERTY(QString planName READ getPlanName NOTIFY dataUpdated)
    Q_PROPERTY(QString storage READ getStorage NOTIFY dataUpdated)
    Q_PROPERTY(QString transfer READ getTransfer NOTIFY dataUpdated)
    Q_PROPERTY(QString totalPrice READ getPrice NOTIFY dataUpdated)
    Q_PROPERTY(QString discountedPrice READ getDiscountedPrice NOTIFY dataUpdated)
    Q_PROPERTY(int days READ getDays NOTIFY countdownChanged)
    Q_PROPERTY(int hours READ getHours NOTIFY countdownChanged)
    Q_PROPERTY(int minutes READ getMinutes NOTIFY countdownChanged)
    Q_PROPERTY(int discountPercentage READ getPercentage NOTIFY dataUpdated)
    Q_PROPERTY(int discountMonthes READ getMonths NOTIFY dataUpdated)

public:
    explicit OfferComponent(QObject* parent = nullptr);
    virtual ~OfferComponent();
    QUrl getQmlUrl() override;
    static void registerQmlModules();
    QString getCurrencySymbol() const;
    QString getCurrencyName() const;
    QString getPlanName() const;
    QString getStorage() const;
    QString getTransfer() const;
    QString getPrice() const;
    QString getDiscountedPrice() const;
    int getDays() const;
    int getHours() const;
    int getMinutes() const;
    void setOfferExpirationDate(QDateTime date);
    Q_INVOKABLE QStringList getPlanFeatures() const;
    int getPercentage() const;
    int getMonths() const;
    void setDiscountInfo(std::shared_ptr<mega::MegaDiscountCodeInfo> discount);
    std::shared_ptr<UpsellPlans::Data> findPlanByLevel(int level) const;
    Q_INVOKABLE void onGrabDeal();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onPlansReady();

signals:
    void dataUpdated();
    void countdownChanged();

private:
    std::shared_ptr<UpsellController> mUpsellController;
    std::shared_ptr<UpsellPlans::Data> mDiscountedPlan = nullptr;
    QTimer mCountDownTimer;
    QDateTime mOfferEndTime;
    std::shared_ptr<mega::MegaDiscountCodeInfo> mDiscountInfo{nullptr};
};
