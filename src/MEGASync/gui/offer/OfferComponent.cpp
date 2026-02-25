#include "OfferComponent.h"

#include "QmlManager.h"
#include "ServiceUrls.h"
#include "UpsellController.h"
#include "UpsellPlans.h"

namespace
{
static bool qmlRegistrationDone = false;
constexpr int MS_IN_ONE_MIN = 1000 * 60; // 1 minute
constexpr int COUNT_DOWN_UPDATE_INTERVAL_MS = MS_IN_ONE_MIN; // 1 minute
constexpr long long COUNT_DOWN_TOLERANCE_MS = 1500; // 1.5 seconds
}

OfferComponent::OfferComponent(QObject* parent):
    QMLComponent(parent),
    mUpsellController(std::make_shared<UpsellController>())
{
    mUpsellController->setBilledPeriod(true);
    mUpsellController->requestPricingData();
    connect(mUpsellController.get(),
            &UpsellController::dataReady,
            this,
            &OfferComponent::onPlansReady);

    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("offerComponentAccess"),
                                                   this);
    qApp->installEventFilter(this);

    mCountDownTimer.setInterval(COUNT_DOWN_UPDATE_INTERVAL_MS);
    connect(&mCountDownTimer, &QTimer::timeout, this, &OfferComponent::onTimerFired);
}

OfferComponent::~OfferComponent() {}

QUrl OfferComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/offer/OfferDialog.qml"));
}

void OfferComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("Offer", 1, 0);
        qmlRegisterType<OfferComponent>("OfferComponent", 1, 0, "OfferComponent");
        qmlRegistrationDone = true;
    }
}

QString OfferComponent::getCurrencySymbol() const
{
    auto plans = mUpsellController->getPlans();
    if (!plans)
    {
        return {};
    }
    return plans->getCurrencySymbol();
}

QString OfferComponent::getCurrencyName() const
{
    const char* code = mDiscountInfo ? mDiscountInfo->getLocalCurrencyCode() : nullptr;

    return (code && *code) ? QString::fromUtf8(code) : QStringLiteral("EUR");
}

QString OfferComponent::getPlanName() const
{
    return mDiscountedPlan ? Utilities::getReadablePlanFromId(mDiscountedPlan->proLevel()) :
                             QLatin1String{};
}

QString OfferComponent::getStorage() const
{
    if (mDiscountedPlan)
    {
        auto transferBytes = mDiscountedPlan->monthlyData().gBStorage();
        return Utilities::getSizeString(transferBytes);
    }
    return {};
}

QString OfferComponent::getTransfer() const
{
    if (mDiscountedPlan)
    {
        auto transferBytes = mDiscountedPlan->monthlyData().gBTransfer() * getMonths();
        return Utilities::getSizeString(transferBytes);
    }
    return {};
}

QStringList OfferComponent::getPlanFeatures() const
{
    QStringList features;
    features << QCoreApplication::translate("OfferStrings", "MEGA VPN");
    features << QCoreApplication::translate("OfferStrings", "MEGA Pass");
    features << QCoreApplication::translate("OfferStrings", "Object storage");
    return features;
}

QString OfferComponent::getPrice() const
{
    if (mDiscountInfo)
    {
        const auto plans = mUpsellController->getPlans();
        QString currency;
        if (plans)
        {
            currency = plans->getCurrencySymbol();
        }
        const auto localByteSymbol = Utilities::decodeUnicodeEscapes(
            QString::fromUtf8(mDiscountInfo->getLocalCurrencySymbol()));

        if (localByteSymbol.isEmpty())
        {
            return Utilities::toPrice(mDiscountInfo->getEuroTotalPriceNet(), currency, true);
        }
        else
        {
            return Utilities::toPrice(mDiscountInfo->getLocalTotalPriceNet(),
                                      localByteSymbol,
                                      true);
        }
    }
    return {};
}

QString OfferComponent::getDiscountedPrice() const
{
    if (mDiscountInfo)
    {
        const auto plans = mUpsellController->getPlans();
        QString currency;
        if (plans)
        {
            currency = plans->getCurrencySymbol();
        }

        const auto localByteSymbol = Utilities::decodeUnicodeEscapes(
            QString::fromUtf8(mDiscountInfo->getLocalCurrencySymbol()));

        if (localByteSymbol.isEmpty())
        {
            return Utilities::toPrice(mDiscountInfo->getEuroDiscountedTotalPriceNet(),
                                      currency,
                                      true);
        }
        else
        {
            return Utilities::toPrice(mDiscountInfo->getLocalDiscountedTotalPriceNet(),
                                      localByteSymbol,
                                      true);
        }
    }
    return {};
}

int OfferComponent::getDays() const
{
    qint64 secsRemaining = QDateTime::currentDateTime().secsTo(mOfferEndTime);
    if (secsRemaining <= 0)
    {
        return 0;
    }
    return static_cast<int>(secsRemaining / 86400); // 86400 secs in a day
}

int OfferComponent::getHours() const
{
    qint64 secsRemaining = QDateTime::currentDateTime().secsTo(mOfferEndTime);
    if (secsRemaining <= 0)
    {
        return 0;
    }
    return static_cast<int>((secsRemaining % 86400) / 3600);
}

int OfferComponent::getMinutes() const
{
    qint64 secsRemaining = QDateTime::currentDateTime().secsTo(mOfferEndTime);
    if (secsRemaining <= 0)
    {
        return 0;
    }
    return static_cast<int>((secsRemaining % 3600) / 60);
}

qint64 OfferComponent::getSeconds() const
{
    qint64 secsRemaining = QDateTime::currentDateTime().secsTo(mOfferEndTime);
    if (secsRemaining <= 0)
    {
        return 0;
    }
    return secsRemaining;
}

void OfferComponent::setOfferExpirationDate(const QDateTime& date)
{
    mOfferEndTime = date;

    // Set single shot timer to next minute (relative to end date)
    QTimer::singleShot(msToNextCountdownMinuteTick(), this, &OfferComponent::onTimerFired);

    emit countdownChanged();
}

int OfferComponent::getPercentage() const
{
    return mDiscountInfo ? mDiscountInfo->getPercentageDiscount() : 0;
}

int OfferComponent::getMonths() const
{
    return mDiscountInfo ? mDiscountInfo->getMonths() : 0;
}

void OfferComponent::setDiscountInfo(std::shared_ptr<mega::MegaDiscountCodeInfo> discount)
{
    if (discount)
    {
        mDiscountInfo = std::move(discount);
        mDiscountedPlan = findPlanByLevel(mDiscountInfo->getAccountLevel());
        setOfferExpirationDate(QDateTime::fromSecsSinceEpoch(mDiscountInfo->getExpiry(), Qt::UTC));
        emit dataUpdated();
    }
}

std::shared_ptr<UpsellPlans::Data> OfferComponent::findPlanByLevel(int level) const
{
    auto plans = mUpsellController->getPlans();
    if (!plans)
    {
        return nullptr;
    }

    for (int i = 0; i < plans->size(); ++i)
    {
        auto plan = plans->getPlan(i);
        if (plan->proLevel() == level)
        {
            return plan;
        }
    }

    return nullptr;
}

void OfferComponent::onGrabDeal()
{
    if (mDiscountInfo)
    {
        Utilities::openUrl(
            ServiceUrls::instance()->getDiscountUrl(QString::fromUtf8(mDiscountInfo->getCode())));
    }
}

bool OfferComponent::localCurrencyIsBillingCurrency() const
{
    auto localCurrencyIsBillingCurrency = true; // Default to true
    if (mDiscountInfo)
    {
        // Local currency is billing currency if the API doesn't give us a local currency symbol.
        localCurrencyIsBillingCurrency =
            QString::fromUtf8(mDiscountInfo->getLocalCurrencySymbol()).isEmpty();
    }
    return localCurrencyIsBillingCurrency;
}

bool OfferComponent::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        emit dataUpdated();
    }
    return QMLComponent::eventFilter(obj, event);
}

void OfferComponent::onPlansReady()
{
    auto plans = mUpsellController->getPlans();

    if (mDiscountInfo && plans && plans->size() > 0)
    {
        mDiscountedPlan =
            findPlanByLevel(mDiscountInfo->getAccountLevel()); // For now, use the first plan
    }

    emit dataUpdated();
}

void OfferComponent::onTimerFired()
{
    emit countdownChanged();

    // Realign if we drifted (sleep/wake or long stall) + check if we passed expiry
    const auto msToMinTick = msToNextCountdownMinuteTick();
    // We've reached the end
    if (msToMinTick <= 0)
    {
        mCountDownTimer.stop();
        // Grey out button
        return;
    }

    const bool aligned = (msToMinTick <= COUNT_DOWN_TOLERANCE_MS) ||
                         (MS_IN_ONE_MIN - msToMinTick <= COUNT_DOWN_TOLERANCE_MS);
    if (!aligned)
    {
        mCountDownTimer.stop();
        // Set single shot timer to next minute (relative to end date)
        QTimer::singleShot(msToMinTick, this, &OfferComponent::onTimerFired);
    }
    // Start timer if needed
    else if (!mCountDownTimer.isActive())
    {
        mCountDownTimer.start();
    }
}

long long OfferComponent::msToNextCountdownMinuteTick() const
{
    auto msLeft = QDateTime::currentDateTimeUtc().msecsTo(mOfferEndTime);

    // Return 0 if end has been reached
    if (msLeft < 0)
    {
        msLeft = 0;
    }
    // Return ms to next tick
    else if (msLeft > 0)
    {
        msLeft %= MS_IN_ONE_MIN;
        if (msLeft == 0)
        {
            msLeft = MS_IN_ONE_MIN;
        }
    }
    return msLeft;
}
