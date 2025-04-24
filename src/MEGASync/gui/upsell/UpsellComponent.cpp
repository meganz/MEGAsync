#include "UpsellComponent.h"

#include "UpsellController.h"
#include "UpsellModel.h"
#include "UpsellQmlDialog.h"

namespace
{
const QLatin1String
    URL_ABOUT_TRANSFER_QUOTA("https://help.mega.io/plans-storage/space-storage/transfer-quota");
const QLatin1String URL_RUBBISH("mega://#fm/rubbish");
const QLatin1String URL_PRO_FLEXI("mega://#pro?tab=flexi");
}

static bool qmlRegistrationDone = false;

UpsellComponent::UpsellComponent(QObject* parent, UpsellPlans::ViewMode mode):
    QMLComponent(parent),
    mController(std::make_shared<UpsellController>()),
    mModel(std::make_shared<UpsellModel>(mController))
{
    registerQmlModules();
    setViewMode(mode);

    mController->registerQmlRootContextProperties();

    connect(mController.get(), &UpsellController::dataReady, this, &UpsellComponent::dataReady);
    mController->requestPricingData();
}

QUrl UpsellComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/upsell/UpsellDialog.qml"));
}

void UpsellComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterUncreatableType<UpsellPlans>(
            "UpsellPlans",
            1,
            0,
            "UpsellPlans",
            QString::fromLatin1("UpsellPlans can only be used for the enum values"));
        qmlRegisterModule("UpsellComponents", 1, 0);
        qmlRegisterType<UpsellQmlDialog>("UpsellComponents", 1, 0, "UpsellQmlDialog");
        qmlRegistrationDone = true;
    }
}

void UpsellComponent::setTransferFinishTime(long long time)
{
    // Seconds since epoch.
    mController->setTransferFinishTime(time);
}

UpsellPlans::ViewMode UpsellComponent::viewMode() const
{
    return mController->viewMode();
}

void UpsellComponent::setViewMode(UpsellPlans::ViewMode mode)
{
    if (mode != viewMode())
    {
        mController->setViewMode(mode);
        mController->resetIsAnyPlanClicked();
        sendStats();
    }
}

void UpsellComponent::sendCloseEvent() const
{
    mController->sendCloseEvent();
    mController->setViewMode(UpsellPlans::ViewMode::NONE);
}

void UpsellComponent::buyButtonClicked(int index)
{
    mController->openPlanUrl(index);

    auto plan{mController->getPlans()->getPlan(index)};
    MegaSyncApp->getStatsEventHandler()->sendTrackedEventArg(
        AppStatsEvents::EventType::UPSELL_DIALOG_PLAN_BUTTON_CLICKED,
        {getViewModeString(), QString::number(plan->proLevel()), plan->name()});
}

void UpsellComponent::billedRadioButtonClicked(bool isMonthly)
{
    mController->setBilledPeriod(isMonthly);
    MegaSyncApp->getStatsEventHandler()->sendTrackedEventArg(
        isMonthly ? AppStatsEvents::EventType::UPSELL_DIALOG_BILLED_MONTHLY_CLICKED :
                    AppStatsEvents::EventType::UPSELL_DIALOG_BILLED_YEARLY_CLICKED,
        {getViewModeString()});
}

void UpsellComponent::linkInDescriptionClicked()
{
    QString urlString;
    switch (viewMode())
    {
        case UpsellPlans::ViewMode::STORAGE_ALMOST_FULL:
        case UpsellPlans::ViewMode::STORAGE_FULL:
        {
            urlString = URL_RUBBISH;
            MegaSyncApp->getStatsEventHandler()->sendTrackedEventArg(
                AppStatsEvents::EventType::UPSELL_DIALOG_EMPTY_RUBBISH_BIN_CLICKED,
                {getViewModeString()});
            break;
        }
        case UpsellPlans::ViewMode::TRANSFER_EXCEEDED:
        {
            urlString = URL_ABOUT_TRANSFER_QUOTA;
            MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
                AppStatsEvents::EventType::UPSELL_DIALOG_LEARN_MORE_TX_QUOTA_CLICKED);
            break;
        }
        default:
        {
            break;
        }
    }

    if (!urlString.isEmpty())
    {
        Utilities::openUrl(QUrl(urlString));
    }
}

void UpsellComponent::linkTryProFlexiClicked()
{
    Utilities::openUrl(QUrl(URL_PRO_FLEXI));
    MegaSyncApp->getStatsEventHandler()->sendTrackedEventArg(
        AppStatsEvents::EventType::UPSELL_DIALOG_TRY_PRO_FLEXI_CLICKED,
        {getViewModeString()});
}

QString UpsellComponent::getViewModeString() const
{
    return QString::number(static_cast<int>(viewMode()));
}

void UpsellComponent::sendStats() const
{
    switch (viewMode())
    {
        case UpsellPlans::ViewMode::STORAGE_FULL:
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::UPSELL_DIALOG_STORAGE_FULL_SHOWN);
            break;
        }
        case UpsellPlans::ViewMode::STORAGE_ALMOST_FULL:
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::UPSELL_DIALOG_STORAGE_ALMOST_FULL_SHOWN);
            break;
        }
        case UpsellPlans::ViewMode::TRANSFER_EXCEEDED:
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::UPSELL_DIALOG_TX_QUOTA_EXCEEDED_SHOWN);
            break;
        }
        default:
        {
            break;
        }
    }
}
