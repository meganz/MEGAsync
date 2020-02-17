#include "UpgradeOverStorage.h"
#include "ui_UpgradeOverStorage.h"
#include "Utilities.h"
#include "Preferences.h"
#include <QDateTime>
#include <QUrl>
#include <QStyle>
#include <math.h>
#include "gui/PlanWidget.h"

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

    highDpiResize.init(this);
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
    long long usedStorage  = preferences->usedStorage();
    long long totalStorage = preferences->totalStorage();

    if (totalStorage == 0 || usedStorage < totalStorage)
    {
        ui->wUsage->hide();
    }
    else
    {
        ui->wUsage->show();

        int percentage = floor((100 * ((double)usedStorage) / totalStorage));
        ui->pUsageStorage->setValue(percentage);

        if (percentage >= 100)
        {
            ui->pUsageStorage->setProperty("almostoq", false);
            ui->pUsageStorage->setProperty("crossedge", true);
        }
        else if (percentage > 90)
        {
            ui->pUsageStorage->setProperty("crossedge", false);
            ui->pUsageStorage->setProperty("almostoq", true);
        }
        else
        {
            ui->pUsageStorage->setProperty("crossedge", false);
            ui->pUsageStorage->setProperty("almostoq", false);
        }

        ui->pUsageStorage->style()->unpolish(ui->pUsageStorage);
        ui->pUsageStorage->style()->polish(ui->pUsageStorage);

        QString used = tr("%1 of %2").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">%1</span>")
                                     .arg(QString::number(percentage).append(QString::fromAscii("%&nbsp;"))))
                                     .arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;%1</span>")
                                     .arg(Utilities::getSizeString(totalStorage)));
        ui->lPercentageUsedStorage->setText(used);
        ui->lTotalUsedStorage->setText(tr("USED STORAGE %1").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;&nbsp;%1</span>")
                                       .arg(Utilities::getSizeString(usedStorage))));
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
            plansLayout->addWidget(new PlanWidget(data, userAgent));
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
        setFixedHeight(600);
    }
    else
    {
        ui->wAchievements->hide();
        setFixedHeight(540);
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

