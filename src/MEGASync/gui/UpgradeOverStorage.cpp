#include "UpgradeOverStorage.h"
#include "ui_UpgradeOverStorage.h"
#include "Utilities.h"
#include "Preferences.h"
#include <QDateTime>
#include <QUrl>

using namespace mega;

UpgradeOverStorage::UpgradeOverStorage(MegaApi *megaApi, MegaPricing *pricing, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgradeOverStorage)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);  

    this->megaApi = megaApi;
    if (pricing)
    {
        this->pricing = pricing->copy();
    }
    else
    {
        this->pricing = NULL;
    }

    checkAchievementsEnabled();
    refreshUsedStorage();

    plansLayout = new QHBoxLayout();
    plansLayout->setContentsMargins(20,5,20,0);
    plansLayout->setSpacing(2);

    delete ui->wPlans->layout();
    ui->wPlans->setLayout(plansLayout);

    updatePlans();
}

void UpgradeOverStorage::setPricing(MegaPricing *pricing)
{
    if (!pricing)
    {
        return;
    }

    if (this->pricing)
    {
        delete this->pricing;
    }

    this->pricing = pricing->copy();
    updatePlans();
}

//TODO: Refresh me when account details is updated.
void UpgradeOverStorage::refreshUsedStorage()
{
    Preferences *preferences = Preferences::instance();
    long long used  = preferences->usedStorage();
    long long total = preferences->totalStorage();
    if (used > total)
    {
        ui->lUsageStorage->setText(tr("Your account is full ([A] / [B]). All file uploads to MEGA are currently disabled.")
                                   .replace(QString::fromUtf8("[A]"), Utilities::getSizeString(used))
                                   .replace(QString::fromUtf8("[B]"), Utilities::getSizeString(total)));
    }
    else
    {
        ui->lUsageStorage->setText(QString());
    }

    checkAchievementsEnabled();
}

UpgradeOverStorage::~UpgradeOverStorage()
{
    delete ui;
    delete pricing;
}

void UpgradeOverStorage::updatePlans()
{
    if (!pricing)
    {
        return;
    }

    clearPlans();

    QString userAgent = QString::fromUtf8(megaApi->getUserAgent());
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
            plansLayout->addWidget(new UpgradeWidget(data, userAgent));
        }
    }
}

void UpgradeOverStorage::checkAchievementsEnabled()
{
    if (megaApi && megaApi->isAchievementsEnabled())
    {
        ui->lAchievements->setTextFormat(Qt::RichText);
        ui->lAchievements->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        ui->lAchievements->setOpenExternalLinks(true);
        ui->lAchievements->setText(ui->lAchievements->text()
                                   .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"mega://#fm/account/achievements\"><span style=\"color:#333333; text-decoration:underline;\">"))
                                   .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>")));
        ui->wAchievements->show();
    }
    else
    {
        ui->wAchievements->hide();
    }
}

void UpgradeOverStorage::clearPlans()
{
    while (QLayoutItem* item = plansLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            delete widget;
        }
        delete item;
    }
}

QString UpgradeOverStorage::convertCurrency(const char *currency)
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

void UpgradeOverStorage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        refreshUsedStorage();
    }
    QDialog::changeEvent(event);
}

