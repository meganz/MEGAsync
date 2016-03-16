#include "UpgradeDialog.h"
#include "ui_UpgradeDialog.h"
#include "Utilities.h"
#include "Preferences.h"
#include <QDateTime>

using namespace mega;

UpgradeDialog::UpgradeDialog(MegaPricing *pricing, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgradeDialog)
{
    ui->setupUi(this);
    this->pricing = pricing->copy();
    finishTime = 0;

    ui->lDescRecommendation->setTextFormat(Qt::RichText);
    ui->lDescRecommendation->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->lDescRecommendation->setOpenExternalLinks(true);
    refreshAccountDetails();

    plansLayout = new QHBoxLayout();
    plansLayout->setContentsMargins(0,0,0,0);
    plansLayout->setSpacing(0);
    updatePlans();

    timer = new QTimer();
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(unitTimeElapsed()));
}

void UpgradeDialog::setTimestamp(long long time)
{
    finishTime = time;
    unitTimeElapsed();
    if (!timer->isActive())
    {
        timer->start(1000);
    }
}

void UpgradeDialog::setPricing(MegaPricing *pricing)
{
    if (this->pricing)
    {
        delete this->pricing;
    }
    this->pricing = pricing->copy();
    updatePlans();
}

void UpgradeDialog::refreshAccountDetails()
{
    Preferences *preferences = Preferences::instance();
    if (preferences->temporalBandwidth())
    {
        ui->lDescRecommendation->setText(tr("You have utilized %1 of data transfer in the last 6 hours, "
                                            "which took you over our current limit. To circumvent this limit, "
                                            "you can [A]upgrade to Pro[/A], which will give you your own bandwidth "
                                            "package and also ample extra storage space. ")
                                        .arg(Utilities::getSizeString(preferences->temporalBandwidth()))
                                        .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"mega://#pro\"><span style=\"color:#d90007; text-decoration:none;\">"))
                                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>"))
                                        .replace(QString::fromUtf8("6"), QString::number(preferences->temporalBandwidthInterval())));
    }
    else
    {
        ui->lDescRecommendation->setText(QString());
    }
}

UpgradeDialog::~UpgradeDialog()
{
    delete ui;
    delete pricing;
}

void UpgradeDialog::updatePlans()
{
    clearPlans(plansLayout);

    int products = pricing->getNumProducts();
    for (int it = 0; it < products; it++)
    {
        if (pricing->getMonths(it) == 1)
        {
            PlanInfo data = {
                pricing->getAmount(it),
                convertCurrency(pricing->getCurrency(it)),
                pricing->getGBStorage(it),
                pricing->getGBTransfer(it),
                pricing->getProLevel(it)
            };
            plansLayout->addWidget(new PlanWidget(data));
        }
    }

    ui->wPlans->setLayout(plansLayout);   
}

void UpgradeDialog::clearPlans(QLayout *layout)
{
    while (QLayoutItem* item = plansLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            delete widget;
        }

        if (QLayout* childLayout = item->layout())
        {
            clearPlans(childLayout);
        }
        delete item;
    }
}

QString UpgradeDialog::convertCurrency(const char *currency)
{
    if (!strcmp(currency, "EUR"))
    {
        return QString::fromUtf8("€");
    }

    if (!strcmp(currency, "USD"))
    {
        return QString::fromUtf8("$");
    }

    return QString::fromUtf8(currency);
}

void UpgradeDialog::unitTimeElapsed()
{
    long long remainingTime = finishTime - QDateTime::currentMSecsSinceEpoch() / 1000;
    if (remainingTime < 0)
    {
        remainingTime = 0;
    }
    ui->lRemainingTime->setText(tr("Please upgrade to Pro to continue immediately, or wait %1 to continue for free. ").arg(Utilities::getTimeString(remainingTime)));
}
