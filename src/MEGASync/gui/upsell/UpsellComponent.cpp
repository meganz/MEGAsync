#include "UpsellComponent.h"

#include "UpsellController.h"
#include "UpsellModel.h"

static bool qmlRegistrationDone = false;

UpsellComponent::UpsellComponent(QObject* parent, UpsellPlans::ViewMode mode):
    QMLComponent(parent),
    mController(std::make_shared<UpsellController>()),
    mModel(std::make_shared<UpsellModel>(mController))
{
    registerQmlModules();
    mController->setViewMode(mode);
    mController->registerQmlRootContextProperties();
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

void UpsellComponent::buyButtonClicked()
{
    mController->openSelectedPlanUrl();
}

void UpsellComponent::billedRadioButtonClicked(bool isMonthly)
{
    mController->setBilledPeriod(isMonthly);
}

void UpsellComponent::rubbishLinkClicked()
{
    Utilities::openUrl(QUrl(QString::fromLatin1("mega://#fm/rubbish")));
}
