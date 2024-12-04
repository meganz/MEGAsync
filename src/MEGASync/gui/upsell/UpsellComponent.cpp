#include "UpsellComponent.h"

#include "UpsellController.h"
#include "UpsellModel.h"

namespace
{
const QLatin1String
    URL_ABOUT_TRANSFER_QUOTA("https://help.mega.io/plans-storage/space-storage/transfer-quota");
const QLatin1String URL_RUBBISH("mega://#fm/rubbish");
}

static bool qmlRegistrationDone = false;

UpsellComponent::UpsellComponent(QObject* parent, UpsellPlans::ViewMode mode):
    QMLComponent(parent),
    mController(std::make_shared<UpsellController>()),
    mModel(std::make_shared<UpsellModel>(mController))
{
    registerQmlModules();
    mController->setViewMode(mode);
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
    mController->setViewMode(mode);
}

void UpsellComponent::buyButtonClicked(int index)
{
    mController->openPlanUrl(index);
}

void UpsellComponent::billedRadioButtonClicked(bool isMonthly)
{
    mController->setBilledPeriod(isMonthly);
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
            break;
        }
        case UpsellPlans::ViewMode::TRANSFER_EXCEEDED:
        {
            urlString = URL_ABOUT_TRANSFER_QUOTA;
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
