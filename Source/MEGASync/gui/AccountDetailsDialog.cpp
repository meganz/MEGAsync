#include "AccountDetailsDialog.h"
#include "ui_AccountDetailsDialog.h"

#include "control/Preferences.h"
#include "control/Utilities.h"

using namespace mega;

AccountDetailsDialog::AccountDetailsDialog(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AccountDetailsDialog)
{
    ui->setupUi(this);

    this->megaApi = megaApi;
    ui->bRefresh->hide();
    refresh();
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    delete ui;
}

void AccountDetailsDialog::refresh()
{
    Preferences *preferences = Preferences::instance();

    QTableWidgetItem *cloudDriveStorage = new QTableWidgetItem();
    cloudDriveStorage->setText(Utilities::getSizeString(preferences->cloudDriveStorage()));
    cloudDriveStorage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(0, 0, cloudDriveStorage);

    QTableWidgetItem *cloudDriveFiles = new QTableWidgetItem();
    cloudDriveFiles->setText(QString::number(preferences->cloudDriveFiles()));
    cloudDriveFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(0, 1, cloudDriveFiles);

    QTableWidgetItem *cloudDriveFolders = new QTableWidgetItem();
    cloudDriveFolders->setText(QString::number(preferences->cloudDriveFolders()));
    cloudDriveFolders->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(0, 2, cloudDriveFolders);

    QTableWidgetItem *inboxStorage = new QTableWidgetItem();
    inboxStorage->setText(Utilities::getSizeString(preferences->inboxStorage()));
    inboxStorage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(1, 0, inboxStorage);

    QTableWidgetItem *inboxFiles = new QTableWidgetItem();
    inboxFiles->setText(QString::number(preferences->inboxFiles()));
    inboxFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(1, 1, inboxFiles);

    QTableWidgetItem *inboxFolders = new QTableWidgetItem();
    inboxFolders->setText(QString::number(preferences->inboxFolders()));
    inboxFolders->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(1, 2, inboxFolders);

    QTableWidgetItem *rubbishStorage = new QTableWidgetItem();
    rubbishStorage->setText(Utilities::getSizeString(preferences->rubbishStorage()));
    rubbishStorage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(2, 0, rubbishStorage);

    QTableWidgetItem *rubbishFiles = new QTableWidgetItem();
    rubbishFiles->setText(QString::number(preferences->rubbishFiles()));
    rubbishFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(2, 1, rubbishFiles);

    QTableWidgetItem *rubbishFolders = new QTableWidgetItem();
    rubbishFolders->setText(QString::number(preferences->rubbishFolders()));
    rubbishFolders->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(2, 2, rubbishFolders);

    ui->tValues->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
}

void AccountDetailsDialog::on_bRefresh_clicked()
{
    megaApi->getAccountDetails();
    ui->bRefresh->setEnabled(false);
}
