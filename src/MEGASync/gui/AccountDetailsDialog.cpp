#include "AccountDetailsDialog.h"
#include "ui_AccountDetailsDialog.h"

#include "control/Preferences.h"
#include "control/Utilities.h"
#include "math.h"

using namespace mega;

AccountDetailsDialog::AccountDetailsDialog(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AccountDetailsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->megaApi = megaApi;
    megaApi->getAccountDetails();
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    delete ui;
}

void AccountDetailsDialog::refresh(Preferences *preferences)
{

    if (preferences->totalStorage() == 0)
    {
        ui->pUsageStorage->setValue(0);
        ui->lTotalUsedStorage->setText(tr("USED STORAGE %1").arg(tr("Data temporarily unavailable")));
    }
    else
    {
        int percentage = ceil((100 * ((double)preferences->usedStorage()) / preferences->totalStorage()));
        ui->pUsageStorage->setValue((percentage < 100) ? percentage : 100);
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
    }

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

    ui->lUsedCloudDrive->setText(Utilities::getSizeString(preferences->cloudDriveStorage()));
    ui->lUsedInbox->setText(Utilities::getSizeString(preferences->inboxStorage()));
    ui->lUsedShares->setText(Utilities::getSizeString(preferences->inShareStorage()));
    ui->lUsedRubbish->setText(Utilities::getSizeString(preferences->rubbishStorage()));
    ui->lSpaceAvailable->setText(Utilities::getSizeString(preferences->totalStorage() - preferences->usedStorage()));
    ui->lUsedByVersions->setText(Utilities::getSizeString(preferences->versionsStorage()));

}
