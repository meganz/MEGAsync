#include "OverQuotaDialog.h"
#include "ui_OverQuotaDialog.h"
#include "mega/types.h"
#include "Utilities.h"
#include <QtConcurrent/QtConcurrent>
#include <QDesktopServices>

OverQuotaDialog::OverQuotaDialog(OverQuotaDialogType type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverquotaFullDialog)
{
    ui->setupUi(this);
    ui->labelTitle->setWordWrap(false);

#ifndef __APPLE__
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    connect(ui->buttonDismiss, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->buttonUpgrade, &QPushButton::clicked, this, &OverQuotaDialog::onUpgradeClicked);
    connect(ui->labelTitle, &CustomLabel::labelSizeChange, this, &OverQuotaDialog::onTitleLengthChanged);

    configureDialog(type);
}

OverQuotaDialog::~OverQuotaDialog()
{
    delete ui;
}

std::unique_ptr<OverQuotaDialog> OverQuotaDialog::createDialog(OverQuotaDialogType type)
{
    return mega::make_unique<OverQuotaDialog>(type);
}

void OverQuotaDialog::configureDialog(OverQuotaDialogType type)
{
    const QString styleLabelAperture{QString::fromUtf8("<p style=\"line-height: 20px;\">")};
    const QString styleLabelClosure{QString::fromUtf8("</p>")};
    const QString storageFullTitle{tr("Storage full")};
    const QString transferQuotaDepletedTitle{tr("Depleted transfer quota")};

    if(type == OverQuotaDialogType::STORAGE_SYNCS)
    {
        setWindowTitle(storageFullTitle);
        ui->labelTitle->setText(tr("Syncs are temporarily disabled."));
        ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available storage space for your account."
                                " You can add syncs but they will remain disabled until there is enough space"
                                " on your account.")
                                + styleLabelClosure);
        ui->buttonUpgrade->setText(tr("Buy more space"));
        ui->stackedWidgetBigIcons->setCurrentWidget(ui->pageStorageFull);
    }
    else if(type == OverQuotaDialogType::STORAGE_UPLOAD)
    {
        setWindowTitle(storageFullTitle);
        ui->labelTitle->setText(tr("Uploads are temporarily disabled."));
        ui->labelMessage->setText(styleLabelAperture +  tr("You have exceeded the available storage space for your account."
                                 " You can add uploads but transfers will remain queued until there is enough space"
                                 " on your account.")
                                 + styleLabelClosure);
        ui->buttonUpgrade->setText(tr("Buy more space"));
        ui->stackedWidgetBigIcons->setCurrentWidget(ui->pageStorageFull);
    }
    else if(type == OverQuotaDialogType::BANDWITH_SYNC)
    {
        setWindowTitle(transferQuotaDepletedTitle);
        ui->labelTitle->setText(tr("Syncs are temporarily disabled."));
        ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can add syncs but they will remain disable until there is enough bandwidth"
                                 " on your account.")
                                 + styleLabelClosure);
        ui->buttonUpgrade->setText(tr("Upgrade Account"));
        ui->stackedWidgetBigIcons->setCurrentWidget(ui->pageBandwidthFull);
    }
    else if(type == OverQuotaDialogType::BANDWIDTH_IMPORT_LINK)
    {
        setWindowTitle(transferQuotaDepletedTitle);
        ui->labelTitle->setText(tr("Importing links is temporarily disabled."));
        ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can import links but transfers will remain queued until there is enough bandwidth"
                                 " on your account.")
                                 + styleLabelClosure);
        ui->buttonUpgrade->setText(tr("Upgrade Account"));
        ui->stackedWidgetBigIcons->setCurrentWidget(ui->pageBandwidthFull);
    }
    else if(type == OverQuotaDialogType::BANDWIDTH_DOWNLOAD)
    {
        setWindowTitle(transferQuotaDepletedTitle);
        ui->labelTitle->setText(tr("Downloads are temporarily disabled."));
        ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can add downloads but transfers will remain queued until there is enough bandwidth"
                                 " on your account.")
                                 + styleLabelClosure);
        ui->buttonUpgrade->setText(tr("Upgrade Account"));
        ui->stackedWidgetBigIcons->setCurrentWidget(ui->pageBandwidthFull);
    }
    else if(type == OverQuotaDialogType::BANDWIDTH_STREAM)
    {
        setWindowTitle(transferQuotaDepletedTitle);
        ui->labelTitle->setText(tr("Streams are temporarily disabled."));
        ui->labelMessage->setText(styleLabelAperture + tr("You have exceeded the available transfer quota for your account."
                                 " You can add streams but transfers will remain queued until there is enough bandwidth"
                                 " on your account.")
                                 + styleLabelClosure);
        ui->buttonUpgrade->setText(tr("Upgrade Account"));
        ui->stackedWidgetBigIcons->setCurrentWidget(ui->pageBandwidthFull);
    }
}

void OverQuotaDialog::onUpgradeClicked()
{
    QString url{QString::fromUtf8("mega://#pro")};
    Utilities::getPROurlWithParameters(url);
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    QDialog::accept();
}

void OverQuotaDialog::onTitleLengthChanged()
{
    int sizeLimitToWrap = ui->widgetHeader->width() - ui->buttonWarning->width() - ui->widgetHeader->layout()->spacing();

#ifndef Q_OS_MACOS
    sizeLimitToWrap -= ui->widgetHeader->layout()->contentsMargins().left() + ui->widgetHeader->layout()->contentsMargins().right();
#endif

    if(ui->labelTitle->width() >= sizeLimitToWrap)
    {
        ui->labelTitle->setWordWrap(true);
    }
}
