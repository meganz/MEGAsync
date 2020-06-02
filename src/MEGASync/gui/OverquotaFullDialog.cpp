#include "OverquotaFullDialog.h"
#include "ui_OverquotaFullDialog.h"
#include "mega/types.h"

OverquotaFullDialog::OverquotaFullDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverquotaFullDialog)
{
    ui->setupUi(this);
    connect(ui->buttonDismiss, &QPushButton::clicked, this, &QDialog::close);
}

OverquotaFullDialog::~OverquotaFullDialog()
{
    delete ui;
}

std::unique_ptr<OverquotaFullDialog> OverquotaFullDialog::createDialog(OverquotaFullDialogType type)
{
    auto dialog{mega::make_unique<OverquotaFullDialog>()};
    if(type == OverquotaFullDialogType::storageFullSyncs)
    {
        dialog->setWindowTitle(tr("Storage full"));
        dialog->ui->labelTitle->setText(tr("Download syncs are temporarily disabled."));
        dialog->ui->labelMessage->setText(tr("<html><head/><body><p>You have exceed the available storage space for your account."
                                 " You can add<br>syncs but download transfers will remain queued until there is enought space"
                                 " in your<br>account.</p></body></html>"));
        dialog->ui->buttonUpgrade->setText(tr("Buy more space"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageStorageFull);
    }
    else if(type == OverquotaFullDialogType::storageFullUploads)
    {
        dialog->setWindowTitle(tr("Storage full"));
        dialog->ui->labelTitle->setText(tr("Uploads are temporarily disabled."));
        dialog->ui->labelMessage->setText(tr("<html><head/><body><p>You have exceed the available storage space for your account."
                                 " You can add<br>uploads but transfers will remain queued until there is enough space"
                                 " in your<br>account.</p></body></html>"));
        dialog->ui->buttonUpgrade->setText(tr("Buy more space"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageStorageFull);
    }
    else if(type == OverquotaFullDialogType::bandwidthFullSync)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Syncs are temporarily disabled."));
        dialog->ui->labelMessage->setText(tr("<html><head/><body><p>You have exceed the available transfer quota for your account."
                                 " You can add<br>syncs but downloads transfers will remain queued until there is enough bandwidth"
                                 " in your<br>account.</p></body></html>"));
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
    }
    else if(type == OverquotaFullDialogType::bandwidthFullImportLink)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Importing links is temporarily disabled."));
        dialog->ui->labelMessage->setText(tr("<html><head/><body><p>You have exceed the available transfer quota for your account."
                                 " You can<br>import links but transfers will remain queued until there is enough bandwidth"
                                 " in your<br>account.</p></body></html>"));
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageBandwidthFull);
    }
    else if(type == OverquotaFullDialogType::bandwidthFullDownlads)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Downloads are temporarily disabled."));
        dialog->ui->labelMessage->setText(tr("<html><head/><body><p>You have exceed the available transfer quota for your account."
                                 " You can add<br>downloads but transfers will remain queued until there is enough bandwidth"
                                 " in your<br>account.</p></body></html>"));
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageBandwidthFull);
    }
    else if(type == OverquotaFullDialogType::bandwidthFullStream)
    {
        dialog->setWindowTitle(tr("Empty transfer quota"));
        dialog->ui->labelTitle->setText(tr("Streams are temporarily disabled."));
        dialog->ui->labelMessage->setText(tr("<html><head/><body><p>You have exceed the available transfer quota for your account."
                                 " You can add<br>streams but transfers will remain queued until there is enough bandwidth"
                                 " in your<br>account.</p></body></html>"));
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageBandwidthFull);
    }
    else if(type == OverquotaFullDialogType::storageAndBandwidthFullSyncs)
    {
        dialog->setWindowTitle(tr("Empty transfer quota and storage full"));
        dialog->ui->labelTitle->setText(tr("Syncs are temporarily disabled."));
        dialog->ui->labelMessage->setText(tr("<html><head/><body><p>You have exceed the available transfer quota for your account."
                                 " You can add<br>syncs but transfers will remain queued until there is enough bandwidth"
                                 " in your<br>account.</p></body></html>"));
        dialog->ui->buttonUpgrade->setText(tr("Upgrade Account"));
        dialog->ui->stackedWidgetBigIcons->setCurrentWidget(dialog->ui->pageStorageFull);
    }

    return dialog;
}
