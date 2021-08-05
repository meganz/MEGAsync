#include "PlanWidget.h"
#include "ui_PlanWidget.h"
#include "Utilities.h"
#include <QDesktopServices>
#include <QUrl>
#include <QtCore>
#include "Preferences.h"
#include "megaapi.h"

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
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));

    updatePlanInfo();
}

void PlanWidget::onOverlayClicked()
{
    QString url;
    switch (details.level)
    {
        case PRO_LITE:
            url = QString::fromUtf8("mega://#propay_4");
            break;
        case PRO_I:
            url = QString::fromUtf8("mega://#propay_1");
            break;
        case PRO_II:
            url = QString::fromUtf8("mega://#propay_2");
            break;
        case PRO_III:
            url = QString::fromUtf8("mega://#propay_3");
            break;
        case BUSINESS:
            url = QString::fromUtf8("mega://#registerb");
            break;
        default:
            url = QString::fromUtf8("mega://#pro");
            break;
    }

    Utilities::getPROurlWithParameters(url);
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
}

PlanWidget::~PlanWidget()
{
    delete ui;
}

void PlanWidget::updatePlanInfo()
{
    QString colorPrice;
    ui->lPeriod->setText(QString::fromUtf8("/%1").arg(tr("month")));

    switch (details.level)
    {
        case PRO_LITE:
            ui->lProPlan->setText(tr("Pro Lite"));
            colorPrice = QString::fromUtf8("color: #ffa500;");
            break;
        case PRO_I:
            ui->lProPlan->setText(QString::fromUtf8("Pro I"));
            colorPrice = QString::fromUtf8("color: #ff333a;");
            break;
        case PRO_II:
            ui->lProPlan->setText(QString::fromUtf8("Pro II"));
            colorPrice = QString::fromUtf8("color: #ff333a;");
            break;
        case PRO_III:
            ui->lProPlan->setText(QString::fromUtf8("Pro III"));
            colorPrice = QString::fromUtf8("color: #ff333a;");
            break;
        case BUSINESS:
            ui->lProPlan->setText(tr("Business"));
            colorPrice = QString::fromUtf8("color: #2BA6DE;");
            ui->lPeriod->setText(tr("per user %1").arg(ui->lPeriod->text()));
            break;
        default:
            ui->lProPlan->setText(QString::fromUtf8("Pro"));
            colorPrice = QString::fromUtf8("color: #2BA6DE;");
            break;
    }

    ui->lPrice->setText(QString::fromUtf8("<span style='font-family:\"Lato\"; font-size:48px; %1'>%2</span><span style='font-family:\"Lato\"; font-size: 26px; %1'>%3</span>")
                        .arg(colorPrice)
                        .arg(details.amount / 100)
                        .arg(details.amount % 100 ? QString::fromUtf8(".%3 %4").arg(details.amount % 100).arg(details.currency) : details.currency));

    if (details.gbStorage == -1) //UNLIMITED
    {
        ui->lStorageInfo->setText(formatRichString(tr("SCALABLE"), STORAGE));
        ui->lBandWidthInfo->setText(formatRichString(tr("UNLIMITED"), BANDWIDTH));
    }
    else
    {
        ui->lStorageInfo->setText(formatRichString(Utilities::getSizeString(details.gbStorage * TOBYTES), STORAGE));
        ui->lBandWidthInfo->setText(formatRichString(Utilities::getSizeString(details.gbTransfer * TOBYTES), BANDWIDTH));
    }
}

void PlanWidget::setPlanInfo(PlanInfo data)
{
    details = data;
    updatePlanInfo();
}

QString PlanWidget::formatRichString(QString str, int type)
{
    return QString::fromUtf8("<span style='color:#333333; font-family: Lato; font-size: 15px; font-weight: 600; text-decoration:none;'>%1 </span>"
                             "<span style='color:#666666; font-family: Lato; font-size: 13px; text-decoration:none;'>%2</span>")
            .arg(str)
            .arg(type == STORAGE ? tr("Storage") : tr("Transfer"));
}

void PlanWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        updatePlanInfo();
    }
    QWidget::changeEvent(event);
}
