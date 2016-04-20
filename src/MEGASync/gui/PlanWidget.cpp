#include "PlanWidget.h"
#include "ui_PlanWidget.h"
#include "Utilities.h"
#include <QDesktopServices>
#include <QUrl>
#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

#define TOBYTES 1024 * 1024 * 1024

PlanWidget::PlanWidget(PlanInfo data, QString userAgent, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlanWidget)
{
    details = data;
    this->userAgent = userAgent;

    ui->setupUi(this);
    ui->lTip->setText(QString::fromUtf8(""));

    //Create the overlay widget with transparent background
    //that will be shown over the Plans to manage clicked() events
    overlay = new QPushButton(this);
    overlay->setObjectName(QString::fromUtf8("bOverlay"));
    overlay->setStyleSheet(QString::fromAscii(
                               "QPushButton#bOverlay:hover {border-image: url(://images/account_type_over.png);} "
                               "QPushButton#bOverlay {border-radius: 3px; border: 1px solid; border-color: rgba(0, 0, 0, 0.1); border: none;} "
                               "QPushButton#bOverlay:pressed {border-image: url(://images/account_type_press.png);}"));

    overlay->setCursor(Qt::PointingHandCursor);
    overlay->resize(this->size());
    ui->lBandWidth->setText(ui->lBandWidth->text().toUpper());
    ui->lStorage->setText(ui->lStorage->text().toUpper());
    ui->lPeriod->setText(QString::fromUtf8("/%1").arg(ui->lPeriod->text()));
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));

    updatePlanInfo();
}

void PlanWidget::onOverlayClicked()
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

PlanWidget::~PlanWidget()
{
    delete ui;
}

void PlanWidget::updatePlanInfo()
{
    switch (details.level)
    {
        case PRO_LITE:
            ui->bcrest->setIcon(QIcon(QString::fromAscii("://images/litecrest.png")));
            ui->bcrest->setIconSize(QSize(58, 58));
            ui->lProPlan->setText(QString::fromUtf8("PRO LITE"));
            ui->lPeriod->setStyleSheet(QString::fromUtf8("color: #ffa500;"));
            break;
        case PRO_I:
            ui->lTip->setText(tr("popular!"));
            ui->bcrest->setIcon(QIcon(QString::fromAscii("://images/proicrest.png")));
            ui->bcrest->setIconSize(QSize(58, 58));
            ui->lProPlan->setText(QString::fromUtf8("PRO I"));
            ui->lPeriod->setStyleSheet(QString::fromUtf8("color: #ff333a;"));
            break;
        case PRO_II:
            ui->bcrest->setIcon(QIcon(QString::fromAscii("://images/proiicrest.png")));
            ui->bcrest->setIconSize(QSize(58, 58));
            ui->lProPlan->setText(QString::fromUtf8("PRO II"));
            ui->lPeriod->setStyleSheet(QString::fromUtf8("color: #ff333a;"));
            break;
        case PRO_III:
            ui->bcrest->setIcon(QIcon(QString::fromAscii("://images/proiiicrest.png")));
            ui->bcrest->setIconSize(QSize(58, 58));
            ui->lProPlan->setText(QString::fromUtf8("PRO III"));
            ui->lPeriod->setStyleSheet(QString::fromUtf8("color: #ff333a;"));
            break;
        default:
            ui->bcrest->setIcon(QIcon(QString::fromAscii("://images/litecrest.png")));
            ui->bcrest->setIconSize(QSize(58, 58));
            ui->lProPlan->setText(QString::fromUtf8("PRO"));
            ui->lPeriod->setStyleSheet(QString::fromUtf8("color: #ffa500;"));
            break;
    }
    ui->lPrice->setText(QString::fromUtf8("<span style='font-family:\"HelveticaNeue\"; font-size:44px;'>%1</span><span style='font-family:\"HelveticaNeue\"; font-size: 33px;'>.%2 %3</span>")
                        .arg(details.amount / 100)
                        .arg(details.amount % 100)
                        .arg(details.currency));
    ui->lStorageInfo->setText(Utilities::getSizeString(details.gbStorage * TOBYTES));
    ui->lBandWidthInfo->setText(Utilities::getSizeString(details.gbTransfer * TOBYTES));
}

void PlanWidget::setPlanInfo(PlanInfo data)
{
    details = data;
    updatePlanInfo();
}
