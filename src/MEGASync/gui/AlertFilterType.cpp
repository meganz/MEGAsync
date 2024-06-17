#include "AlertFilterType.h"
#include "ui_AlertFilterType.h"

AlertFilterType::AlertFilterType(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlertFilterType)
{
    ui->setupUi(this);
    ui->lFilterActive->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->bDropDownArrow->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->liconType->setAttribute(Qt::WA_TransparentForMouseEvents);

    setActualFilter(QFilterAlertsModel::FilterType::ALL);
}

AlertFilterType::~AlertFilterType()
{
    delete ui;
}

void AlertFilterType::setActualFilter(QFilterAlertsModel::FilterType type)
{
    switch (type)
    {
        case QFilterAlertsModel::FilterType::CONTACTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/contacts.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Contacts"));
            break;
        }
        case QFilterAlertsModel::FilterType::PAYMENTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/payments.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Payment"));
            break;
        }
        case QFilterAlertsModel::FilterType::SHARES:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/incoming_share.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Incoming Shares"));
            break;
        }
        case QFilterAlertsModel::FilterType::ALL:
        case QFilterAlertsModel::FilterType::TAKEDOWNS:
        default:
        {
            ui->wIconType->hide();
            ui->lFilterActive->setText(tr("All notifications"));
            break;
        }
    }
    mType = type;
}

void AlertFilterType::mousePressEvent(QMouseEvent*)
{
    emit clicked();
}

void AlertFilterType::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setActualFilter(mType);
    }
    QWidget::changeEvent(event);
}
