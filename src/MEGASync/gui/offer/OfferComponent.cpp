#include "OfferComponent.h"

#include "QmlManager.h"
#include "ServiceUrls.h"
#include "UpsellController.h"
#include "UpsellPlans.h"

namespace
{
static bool qmlRegistrationDone = false;
constexpr int COUNT_DOWN_UPDATE_INTERVAL = 60000;
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
    if (secsRemaining < 0)
        return 0;
    return static_cast<int>(secsRemaining / 86400); // 86400 secs in a day
}

int OfferComponent::getHours() const
{
    qint64 secsRemaining = QDateTime::currentDateTime().secsTo(mOfferEndTime);
    if (secsRemaining < 0)
        return 0;
    return static_cast<int>((secsRemaining % 86400) / 3600);
}

int OfferComponent::getMinutes() const
{
    qint64 secsRemaining = QDateTime::currentDateTime().secsTo(mOfferEndTime);
    if (secsRemaining < 0)
        return 0;
    return static_cast<int>((secsRemaining % 3600) / 60);
}

void OfferComponent::setOfferExpirationDate(QDateTime date)
{
    mOfferEndTime = date;
    emit countdownChanged();
    connect(&mCountDownTimer,
            &QTimer::timeout,
            this,
            [this]()
            {
                emit countdownChanged();

                if (QDateTime::currentDateTime() >= mOfferEndTime)
                {
                    mCountDownTimer.stop();
                }
            });
    mCountDownTimer.start(COUNT_DOWN_UPDATE_INTERVAL);
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
    mDiscountInfo = discount;
    mDiscountedPlan = findPlanByLevel(mDiscountInfo->getAccountLevel());
    setOfferExpirationDate(QDateTime::fromSecsSinceEpoch(mDiscountInfo->getExpiry()));
    emit dataUpdated();
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

bool OfferComponent::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        emit dataUpdated();
    }
    return QMLComponent::eventFilter(obj, event);
}
