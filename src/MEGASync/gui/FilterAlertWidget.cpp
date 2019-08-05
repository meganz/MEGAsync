#include "FilterAlertWidget.h"
#include "ui_FilterAlertWidget.h"
#include "qdebug.h"

FilterAlertWidget::FilterAlertWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FilterAlertWidget)
{
    isContactsAvailable, isSharesAvailable, isPaymentAvailable = false;

    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
}

FilterAlertWidget::~FilterAlertWidget()
{
    delete ui;
}

void FilterAlertWidget::enableFilters(bool contacts, bool shares, bool payment)
{
    enableFilterContacts(contacts);
    enableFilterShares(shares);
    enableFilterPayment(payment);

    //Set minimum size without filter rows to calculate needed height
    setMinimumHeight(55);
    setMaximumHeight(55);

    int rowTotalHeight = 0;

    if (isContactsAvailable)
    {
        rowTotalHeight += ui->wContactsNotifications->height();
    }
    if (isSharesAvailable)
    {
        rowTotalHeight += ui->wSharesNotifications->height();
    }
    if (isPaymentAvailable)
    {
        rowTotalHeight += ui->wPaymentNotifications->height();
    }

    setMinimumHeight(height() + rowTotalHeight);
    setMaximumHeight(height() + rowTotalHeight);
}

void FilterAlertWidget::enableFilterContacts(bool opt)
{
    isContactsAvailable = opt;
    ui->wContactsNotifications->setVisible(opt);
}

void FilterAlertWidget::enableFilterShares(bool opt)
{
    isSharesAvailable = opt;
    ui->wSharesNotifications->setVisible(opt);
}

void FilterAlertWidget::enableFilterPayment(bool opt)
{
    isPaymentAvailable = opt;
    ui->wPaymentNotifications->setVisible(opt);
}

void FilterAlertWidget::on_bAll_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::NO_FILTER);
}

void FilterAlertWidget::on_bContacts_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::FILTER_CONTACTS);
}

void FilterAlertWidget::on_bShares_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::FILTER_SHARES);
}

void FilterAlertWidget::on_bPayment_clicked()
{
    emit onFilterClicked(QFilterAlertsModel::FILTER_PAYMENT);
}
