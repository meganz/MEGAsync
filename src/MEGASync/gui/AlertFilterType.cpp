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

    setActualFilter(NotificationAlertProxyModel::FilterType::ALL);
}

AlertFilterType::~AlertFilterType()
{
    delete ui;
}

void AlertFilterType::setActualFilter(NotificationAlertProxyModel::FilterType type)
{
    switch (type)
    {
        case NotificationAlertProxyModel::FilterType::CONTACTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/contacts.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Contacts"));
            break;
        }
        case NotificationAlertProxyModel::FilterType::PAYMENTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/payments.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Payment"));
            break;
        }
        case NotificationAlertProxyModel::FilterType::SHARES:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/incoming_share.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Incoming Shares"));
            break;
        }
        case NotificationAlertProxyModel::FilterType::ALL:
        case NotificationAlertProxyModel::FilterType::TAKEDOWNS:
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
