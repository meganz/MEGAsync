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
    on_bRefresh_clicked();
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    delete ui;
}

void AccountDetailsDialog::refresh(Preferences *preferences)
{
    ui->bRefresh->setText(tr("Refresh"));
    ui->bRefresh->setEnabled(true);

    if (preferences->usedStorage() > preferences->totalStorage())
    {
        ui->wUsage->setOverQuotaReached(true);
    }
    else
    {
        ui->wUsage->setOverQuotaReached(false);
    }

    ui->wUsage->setMaxStorage(Utilities::getSizeString(preferences->totalStorage()));
    int pCloud = ceil(360 * preferences->cloudDriveStorage() / (double)preferences->totalStorage());
    ui->wUsage->setCloudStorage((pCloud < 360) ? pCloud : 360);
    int pRubbish = ceil(360 * preferences->rubbishStorage() / (double)preferences->totalStorage());
    ui->wUsage->setRubbishStorage((pRubbish < 360) ? pRubbish : 360);
    int pInShares = ceil(360 * preferences->inShareStorage() / (double)preferences->totalStorage());
    ui->wUsage->setInShareStorage((pInShares < 360) ? pInShares : 360);
    int pInbox = ceil(360 * preferences->inboxStorage() / (double)preferences->totalStorage());
    ui->wUsage->setInboxStorage((pInbox < 360) ? pInbox : 360);

    ui->wUsage->setCloudStorageLabel(Utilities::getSizeString(preferences->cloudDriveStorage()));
    ui->wUsage->setRubbishStorageLabel(Utilities::getSizeString(preferences->rubbishStorage()));
    ui->wUsage->setInboxStorageLabel(Utilities::getSizeString(preferences->inboxStorage()));

    ui->wUsage->setInShareStorageLabel(Utilities::getSizeString(preferences->inShareStorage()));
    ui->wUsage->setUsedStorageLabel(Utilities::getSizeString(preferences->usedStorage()));
    ui->wUsage->setAvailableStorageLabel(Utilities::getSizeString(preferences->totalStorage()-preferences->usedStorage()));

    ui->bRefresh->setText(tr("Refresh"));
    ui->bRefresh->setEnabled(true);
}

void AccountDetailsDialog::on_bRefresh_clicked()
{
    megaApi->getAccountDetails();
    ui->bRefresh->setEnabled(false);
    ui->bRefresh->setText(tr("Loading..."));
    ui->wUsage->clearAll();
}
