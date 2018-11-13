#include "UpgradeWidget.h"
#include "ui_UpgradeWidget.h"
#include "Utilities.h"
#include <QDesktopServices>
#include <QUrl>
#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

#define TOBYTES 1024 * 1024 * 1024

UpgradeWidget::UpgradeWidget(PlanInfo data, QString userAgent, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UpgradeWidget)
{
    details = data;
    this->userAgent = userAgent;

    ui->setupUi(this);

    //Create the overlay widget with transparent background
    //that will be shown over the Plans to manage clicked() events
    overlay = new QPushButton(this);
    overlay->setObjectName(QString::fromUtf8("bOverlay"));
    overlay->setStyleSheet(QString::fromAscii(
                               "QPushButton#bOverlay:hover {border-image: url(://images/card_border.png);} "
                               "QPushButton#bOverlay {border-radius: 3px; border: 1px solid; border-color: rgba(0, 0, 0, 0.1); border: none;} "
                               ));

    overlay->setCursor(Qt::PointingHandCursor);
    overlay->resize(this->size());
    ui->lPeriod->setText(QString::fromUtf8("/%1").arg(ui->lPeriod->text()));
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));

    updatePlanInfo();
}

void UpgradeWidget::onOverlayClicked()
{
    QString escapedUserAgent = QString::fromUtf8(QUrl::toPercentEncoding(userAgent));
    switch (details.level)
    {
        case PRO_LITE:
            QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#pro_lite/uao=%1").arg(escapedUserAgent)));
            break;
        case PRO_I:
            QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#pro_1/uao=%1").arg(escapedUserAgent)));
            break;
        case PRO_II:
            QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#pro_2/uao=%1").arg(escapedUserAgent)));
            break;
        case PRO_III:
            QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#pro_3/uao=%1").arg(escapedUserAgent)));
            break;
        default:
            QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#pro/uao=%1").arg(escapedUserAgent)));
            break;
    }
}

UpgradeWidget::~UpgradeWidget()
{
    delete ui;
}

void UpgradeWidget::updatePlanInfo()
{
    switch (details.level)
    {
        case PRO_LITE:
            ui->lProPlan->setText(QString::fromUtf8("PRO LITE"));
            ui->lPrice->setStyleSheet(QString::fromUtf8("color: #ffa500;"));
            break;
        case PRO_I:
            ui->lProPlan->setText(QString::fromUtf8("PRO I"));
            ui->lPrice->setStyleSheet(QString::fromUtf8("color: #F0373A;"));
            break;
        case PRO_II:
            ui->lProPlan->setText(QString::fromUtf8("PRO II"));
            ui->lPrice->setStyleSheet(QString::fromUtf8("color: #F0373A;"));
            break;
        case PRO_III:
            ui->lProPlan->setText(QString::fromUtf8("PRO III"));
            ui->lPrice->setStyleSheet(QString::fromUtf8("color: #F0373A;"));
            break;
        default:
            ui->lProPlan->setText(QString::fromUtf8("PRO"));
            ui->lPrice->setStyleSheet(QString::fromUtf8("color: #ffa500;"));
            break;
    }
    ui->lPrice->setText(QString::fromUtf8("<span style='font-family:\"Lato\"; font-size:48px;'>%1</span><span style='font-family:\"Lato\"; font-size: 26px;'>.%2 %3</span>")
                        .arg(details.amount / 100)
                        .arg(details.amount % 100)
                        .arg(details.currency));

    QString maxStorage = QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; font-family:\"Source Sans Pro\"; text-decoration:none;\">%1&nbsp;</span>").arg(Utilities::getSizeString(details.gbStorage * TOBYTES)) + tr("storage");
    QString maxTransfer = QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; font-family:\"Source Sans Pro\"; text-decoration:none;\">%1&nbsp;</span>").arg(Utilities::getSizeString(details.gbTransfer * TOBYTES)) + tr("transfer");

    ui->lStorageInfo->setText(maxStorage);
    ui->lBandWidthInfo->setText(maxTransfer);
}

void UpgradeWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ui->lPeriod->setText(QString::fromUtf8("/%1").arg(ui->lPeriod->text()));
        updatePlanInfo();
    }
    QWidget::changeEvent(event);
}

void UpgradeWidget::setPlanInfo(PlanInfo data)
{
    details = data;
    updatePlanInfo();
}

