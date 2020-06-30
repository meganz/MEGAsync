#include "OverQuotaDialog.h"
#include "ui_OverQuotaDialog.h"
#include "mega/types.h"
#include "Utilities.h"
#include <QtConcurrent/QtConcurrent>
#include <QDesktopServices>

OverQuotaDialog::OverQuotaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverquotaFullDialog)
{
    ui->setupUi(this);
    connect(ui->buttonDismiss, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->buttonUpgrade, &QPushButton::clicked, this, &OverQuotaDialog::onUpgradeClicked);
}

OverQuotaDialog::~OverQuotaDialog()
{
    delete ui;
}

std::unique_ptr<OverQuotaDialog> OverQuotaDialog::createDialog(OverquotaFullDialogType type)
{
    QString styleLabelAperture{QString::fromUtf8("<p style=\"line-height: 20px;\">")};
    QString styleLabelClosure{QString::fromUtf8("</p>")};

    auto dialog{mega::make_unique<OverQuotaDialog>()};
    if(type == OverquotaFullDialogType::storageFullSyncs)
    {
        dialog->setWindowTitle(tr("Storage full"));
        dialog->ui->labelTitle->setText(tr("Download syncs are temporarily disabled."));
        dialog->ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available storage space for your account."
                                " You can add syncs but download transfers will remain queued until there is enough space"
                                " in your account.")
                                + styleLabelClosure);
        dialog->ui->buttonUpgrade->setText(tr("Buy more space"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageStorageFull);
    }
    else if(type == OverquotaFullDialogType::storageFullUploads)
    {
        dialog->setWindowTitle(tr("Storage full"));
        dialog->ui->labelTitle->setText(tr("Uploads are temporarily disabled."));
        dialog->ui->labelMessage->setText(styleLabelAperture +  tr("You have exceeded the available storage space for your account."
                                 " You can add uploads but transfers will remain queued until there is enough space"
                                 " in your account.")
                                 + styleLabelClosure);
        dialog->ui->buttonUpgrade->setText(tr("Buy more space"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageStorageFull);
    }
    else if(type == OverquotaFullDialogType::bandwidthFullSync)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Syncs are temporarily disabled."));
        dialog->ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can add syncs but downloads transfers will remain queued until there is enough bandwidth"
                                 " in your account.")
                                 + styleLabelClosure);
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageBandwidthFull);
    }
    else if(type == OverquotaFullDialogType::bandwidthFullImportLink)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Importing links is temporarily disabled."));
        dialog->ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can import links but transfers will remain queued until there is enough bandwidth"
                                 " in your account.")
                                 + styleLabelClosure);
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageBandwidthFull);
    }
    else if(type == OverquotaFullDialogType::bandwidthFullDownloads)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Downloads are temporarily disabled."));
        dialog->ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can add downloads but transfers will remain queued until there is enough bandwidth"
                                 " in your account.")
                                 + styleLabelClosure);
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageBandwidthFull);
    }
    else if(type == OverquotaFullDialogType::bandwidthFullStream)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Streams are temporarily disabled."));
        dialog->ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can add streams but transfers will remain queued until there is enough bandwidth"
                                 " in your account.")
                                 + styleLabelClosure);
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageBandwidthFull);
    }
    else if(type == OverquotaFullDialogType::storageAndBandwidthFullSyncs)
    {
        dialog->setWindowTitle(tr("Empty transfer quota and storage full"));
        dialog->ui->labelTitle->setText(tr("Syncs are temporarily disabled."));
        dialog->ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can add syncs but transfers will remain queued until there is enough bandwidth"
                                 " in your account.")
                                 + styleLabelClosure);
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageStorageFull);
    }

    return dialog;
}

void OverQuotaDialog::onUpgradeClicked()
{
    auto url{QString::fromUtf8("mega://#pro")};
    Utilities::getPROurlWithParameters(url);
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    QDialog::accept();
}
