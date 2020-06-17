#include "AccountDetailsDialog.h"
#include "ui_AccountDetailsDialog.h"

#include "control/Preferences.h"
#include "control/Utilities.h"
#include "math.h"
#include "MegaApplication.h"

#include <QStyle>

using namespace mega;

AccountDetailsDialog::AccountDetailsDialog(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AccountDetailsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->megaApi = megaApi;
    ui->lLoading->setText(ui->lLoading->text().toUpper());
    refresh(Preferences::instance());
    highDpiResize.init(this);
    ((MegaApplication*)qApp)->attachStorageObserver(*this);
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    ((MegaApplication*)qApp)->dettachStorageObserver(*this);
    delete ui;
}

void AccountDetailsDialog::refresh(Preferences *preferences)
{

    if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        setMinimumHeight(220);
        setMaximumHeight(220);
        setContentsMargins(0, 0, 0, 24);
        ui->sHeader->hide();
        ui->wAvailable->hide();
        ui->wInbox->hide();
    }
    else
    {
        setMinimumHeight(320);
        setMaximumHeight(320);
        setContentsMargins(0, 24, 0, 24);
        ui->sHeader->show();
        ui->wAvailable->show();
        ui->wInbox->show();
    }

    if (preferences->totalStorage() == 0)
    {
        ui->pUsageStorage->setValue(0);
        ui->lTotalUsedStorage->setText(tr("USED STORAGE %1").arg(tr("Data temporarily unavailable")));
        ui->sHeader->setCurrentWidget(ui->pLoading);
        usageDataAvailable(false);
    }
    else
    {
        ui->sHeader->setCurrentWidget(ui->pUsedData);
        int percentage = floor((100 * ((double)preferences->usedStorage()) / preferences->totalStorage()));
        ui->pUsageStorage->setValue(percentage > ui->pUsageStorage->maximum() ? ui->pUsageStorage->maximum() : percentage);
        if (percentage > 100)
        {
            ui->pUsageStorage->setProperty("crossedge", true);
        }
        else
        {
            ui->pUsageStorage->setProperty("crossedge", false);
        }
        ui->pUsageStorage->style()->unpolish(ui->pUsageStorage);
        ui->pUsageStorage->style()->polish(ui->pUsageStorage);

        QString used = tr("%1 of %2").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 18px; text-decoration:none;\">%1&nbsp;</span>")
                                     .arg(QString::number(percentage).append(QString::fromAscii(" %"))))
                                     .arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 18px; text-decoration:none;\">&nbsp;%1</span>")
                                     .arg(Utilities::getSizeString(preferences->totalStorage())));
        ui->lPercentageUsedStorage->setText(used);
        ui->lTotalUsedStorage->setText(tr("USED STORAGE %1").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 18px; text-decoration:none;\">&nbsp;&nbsp;%1</span>")
                                       .arg(Utilities::getSizeString(preferences->usedStorage()))));


        if (preferences->usedStorage() > preferences->totalStorage())
        {
            ui->pUsageStorage->setProperty("overquota", true);
        }
        else
        {
            ui->pUsageStorage->setProperty("overquota", false);
        }

        ui->pUsageStorage->style()->unpolish(ui->pUsageStorage);
        ui->pUsageStorage->style()->polish(ui->pUsageStorage);

        usageDataAvailable(true);
        ui->lUsedCloudDrive->setText(Utilities::getSizeString(preferences->cloudDriveStorage()));
        ui->lUsedInbox->setText(Utilities::getSizeString(preferences->inboxStorage()));
        ui->lUsedShares->setText(Utilities::getSizeString(preferences->inShareStorage()));
        ui->lUsedRubbish->setText(Utilities::getSizeString(preferences->rubbishStorage()));
        ui->lSpaceAvailable->setText(Utilities::getSizeString(preferences->availableStorage()));
        ui->lUsedByVersions->setText(Utilities::getSizeString(preferences->versionsStorage()));
    }
}

void AccountDetailsDialog::updateStorageElements()
{
    refresh(Preferences::instance());
}

void AccountDetailsDialog::usageDataAvailable(bool isAvailable)
{
    ui->lUsedCloudDrive->setVisible(isAvailable);
    ui->lUsedInbox->setVisible(isAvailable);
    ui->lUsedShares->setVisible(isAvailable);
    ui->lUsedRubbish->setVisible(isAvailable);
    ui->lSpaceAvailable->setVisible(isAvailable);
    ui->lUsedByVersions->setVisible(isAvailable);
}
