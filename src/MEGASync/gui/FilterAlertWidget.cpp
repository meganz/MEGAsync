#include "FilterAlertWidget.h"
#include "ui_FilterAlertWidget.h"

FilterAlertWidget::FilterAlertWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterAlertWidget)
{
    ui->setupUi(this);
    setUnseenNotifications(0, 0, 0, 0);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
}

FilterAlertWidget::~FilterAlertWidget()
{
    delete ui;
}

void FilterAlertWidget::reset()
{
    setUnseenNotifications(0, 0, 0, 0);
}

void FilterAlertWidget::setUnseenNotifications(long long all, long long contacts, long long shares, long long payment)
{
    long long allUnseen = all > 0 ? all : 0;
    long long contactsUnseen = contacts > 0 ? contacts : 0;
    long long sharesUnseen = shares > 0 ? shares : 0;
    long long paymentUnseen = payment > 0 ? payment : 0;

    if (!allUnseen)
    {
        ui->bNumberNotifications->hide();
    }
    else
    {
        ui->bNumberNotifications->setText(QString::number(allUnseen));
        ui->bNumberNotifications->show();
    }

    if (!contactsUnseen)
    {
        ui->bNumberNotificationsContacts->hide();
    }
    else
    {
        ui->bNumberNotificationsContacts->setText(QString::number(contactsUnseen));
        ui->bNumberNotificationsContacts->show();
    }

    if (!sharesUnseen)
    {
        ui->bNumberNotificationsShares->hide();
    }
    else
    {
        ui->bNumberNotificationsShares->setText(QString::number(sharesUnseen));
        ui->bNumberNotificationsShares->show();
    }

    if (!paymentUnseen)
    {
        ui->bNumberNotificationsPayments->hide();
    }
    else
    {
        ui->bNumberNotificationsPayments->setText(QString::number(paymentUnseen));
        ui->bNumberNotificationsPayments->show();
    }
}

void FilterAlertWidget::on_bAll_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::NO_FILTER);
}

void FilterAlertWidget::on_bContacts_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::FILTER_CONTACTS);
    QApplication::postEvent(ui->bContacts, new QEvent(QEvent::Leave));
}

void FilterAlertWidget::on_bShares_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::FILTER_SHARES);
    QApplication::postEvent(ui->bShares, new QEvent(QEvent::Leave));
}

void FilterAlertWidget::on_bPayment_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::FILTER_PAYMENT);
    QApplication::postEvent(ui->bPayment, new QEvent(QEvent::Leave));
}

void FilterAlertWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
