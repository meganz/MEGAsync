#include "PlanWidget.h"
#include "ui_PlanWidget.h"
#include "Utilities.h"
#include <QDesktopServices>
#include <QUrl>

#define TOBYTES 1024 * 1024 * 1024

PlanWidget::PlanWidget(PlanInfo data, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlanWidget)
{
    details = data;

    ui->setupUi(this);
    ui->lTip->setVisible(false);

    //Create the overlay widget with transparent background
    //that will be shown over the Plans to manage clicked() events
    overlay = new QPushButton(this);
    overlay->setObjectName(QString::fromUtf8("bOverlay"));
    overlay->setStyleSheet(QString::fromAscii(
                               "QPushButton#bOverlay:hover {border-radius: 3px; border: 3px solid; border-color: rgba(0, 0, 0);} "
                               "QPushButton#bOverlay {border-radius: 3px; border: 1px solid; border-color: rgba(0, 0, 0, 0.1); background: transparent; border: none;} "
                               "QPushButton#bOverlay:pressed {border-radius: 3px; border: 3px solid; border-color: rgba(255, 51, 58, 0.8);}"));
    overlay->resize(ui->wContainer->minimumSize());
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));

    updatePlanInfo();

}

void PlanWidget::onOverlayClicked()
{
    QDesktopServices::openUrl(QUrl(QString::fromUtf8("mega://#pro")));

    switch (details.level)
    {
        case PRO_LITE:
            QDesktopServices::openUrl(QUrl(QString::fromUtf8("mega://#pro_lite")));
            break;
        case PRO_I:
            QDesktopServices::openUrl(QUrl(QString::fromUtf8("mega://#pro_1")));
            break;
        case PRO_II:
            QDesktopServices::openUrl(QUrl(QString::fromUtf8("mega://#pro_2")));
            break;
        case PRO_III:
            QDesktopServices::openUrl(QUrl(QString::fromUtf8("mega://#pro_3")));
            break;
        default:
            QDesktopServices::openUrl(QUrl(QString::fromUtf8("mega://#pro")));
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
            ui->lTip->setText(QString::fromUtf8("popular!"));
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
            break;
    }
    ui->lPrice->setText(QString::fromUtf8("<span style='font-size:48px;'>%1</span><span style='font-size: 36px;'>.%2 %3</span>")
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
