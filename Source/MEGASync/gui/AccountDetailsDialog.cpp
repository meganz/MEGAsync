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
    on_bRefresh_clicked();
}

AccountDetailsDialog::~AccountDetailsDialog()
{
    delete ui;
}

void AccountDetailsDialog::refresh(MegaAccountDetails *details)
{
    MegaNode *root = megaApi->getRootNode();
    MegaNode *rubbish = megaApi->getRubbishNode();
    MegaNode *inbox = megaApi->getInboxNode();
    if(!root || !rubbish || !inbox || !details->getNumUsageItems())
    {
        delete root;
        delete rubbish;
        delete inbox;
        ui->bRefresh->setText(tr("Refresh"));
        ui->bRefresh->setEnabled(true);
        return;
    }

    ui->tValues->setRowCount(3);

    QTableWidgetItem *cloudDriveStorage = new QTableWidgetItem();
    cloudDriveStorage->setText(Utilities::getSizeString(details->getStorageUsed(root->getHandle())));
    cloudDriveStorage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(0, 0, cloudDriveStorage);

    QTableWidgetItem *cloudDriveFiles = new QTableWidgetItem();
    cloudDriveFiles->setText(QString::number(details->getNumFiles(root->getHandle())));
    cloudDriveFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(0, 1, cloudDriveFiles);

    QTableWidgetItem *cloudDriveFolders = new QTableWidgetItem();
    cloudDriveFolders->setText(QString::number(details->getNumFolders(root->getHandle())));
    cloudDriveFolders->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(0, 2, cloudDriveFolders);

    QTableWidgetItem *inboxStorage = new QTableWidgetItem();
    inboxStorage->setText(Utilities::getSizeString(details->getStorageUsed(inbox->getHandle())));
    inboxStorage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(1, 0, inboxStorage);

    QTableWidgetItem *inboxFiles = new QTableWidgetItem();
    inboxFiles->setText(QString::number(details->getNumFiles(inbox->getHandle())));
    inboxFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(1, 1, inboxFiles);

    QTableWidgetItem *inboxFolders = new QTableWidgetItem();
    inboxFolders->setText(QString::number(details->getNumFolders(inbox->getHandle())));
    inboxFolders->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(1, 2, inboxFolders);

    QTableWidgetItem *rubbishStorage = new QTableWidgetItem();
    rubbishStorage->setText(Utilities::getSizeString(details->getStorageUsed(rubbish->getHandle())));
    rubbishStorage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(2, 0, rubbishStorage);

    QTableWidgetItem *rubbishFiles = new QTableWidgetItem();
    rubbishFiles->setText(QString::number(details->getNumFiles(rubbish->getHandle())));
    rubbishFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(2, 1, rubbishFiles);

    QTableWidgetItem *rubbishFolders = new QTableWidgetItem();
    rubbishFolders->setText(QString::number(details->getNumFolders(rubbish->getHandle())));
    rubbishFolders->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tValues->setItem(2, 2, rubbishFolders);

    if(details->getNumUsageItems() > 3)
    {
        MegaNodeList *nodeList = megaApi->getInShares();
        ui->tValues->setRowCount(3 + nodeList->size());
        ui->tValues->verticalHeader()->setTextElideMode(Qt::ElideMiddle);

        for(int i=0; i < nodeList->size(); i++)
        {
            MegaNode *node = nodeList->get(i);

            QTableWidgetItem *header = new QTableWidgetItem();
            header->setText(QString::fromUtf8(node->getName()));
            header->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            header->setToolTip(QString::fromUtf8(megaApi->getNodePath(node)));
            ui->tValues->setVerticalHeaderItem(3+i, header);

            QTableWidgetItem *nodeStorage = new QTableWidgetItem();
            nodeStorage->setText(Utilities::getSizeString(details->getStorageUsed(node->getHandle())));
            nodeStorage->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->tValues->setItem(3+i, 0, nodeStorage);

            QTableWidgetItem *nodeFiles = new QTableWidgetItem();
            nodeFiles->setText(QString::number(details->getNumFiles(node->getHandle())));
            nodeFiles->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->tValues->setItem(3+i, 1, nodeFiles);

            QTableWidgetItem *nodeFolders = new QTableWidgetItem();
            nodeFolders->setText(QString::number(details->getNumFolders(node->getHandle())));
            nodeFolders->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->tValues->setItem(3+i, 2, nodeFolders);
        }
        delete nodeList;
    }

    ui->tValues->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tValues->verticalHeader()->setFixedWidth(140);


    delete root;
    delete rubbish;
    delete inbox;
    ui->bRefresh->setText(tr("Refresh"));
    ui->bRefresh->setEnabled(true);
}

void AccountDetailsDialog::on_bRefresh_clicked()
{
    megaApi->getAccountDetails();
    ui->bRefresh->setEnabled(false);
    ui->bRefresh->setText(tr("Loading..."));
}
