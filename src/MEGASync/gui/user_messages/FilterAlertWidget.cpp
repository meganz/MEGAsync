#include "FilterAlertWidget.h"

#include "ui_FilterAlertWidget.h"

namespace
{
constexpr int RED_BUBBLE_MARGIN = 17;
}

FilterAlertWidget::FilterAlertWidget(QWidget* parent)
    : QWidget(parent)
    , mUi(new Ui::FilterAlertWidget)
    , mCurrentFilter(MessageType::ALL)
{
    mUi->setupUi(this);
    setUnseenNotifications(0, 0, 0, 0);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
    adjustSize();
}

FilterAlertWidget::~FilterAlertWidget()
{
    delete mUi;
}

void FilterAlertWidget::reset()
{
    setUnseenNotifications(0, 0, 0, 0);
}

MessageType FilterAlertWidget::getCurrentFilter() const
{
    return mCurrentFilter;
}

void FilterAlertWidget::setUnseenNotifications(long long all, long long contacts, long long shares, long long payment)
{
    long long allUnseen = all > 0 ? all : 0;
    long long contactsUnseen = contacts > 0 ? contacts : 0;
    long long sharesUnseen = shares > 0 ? shares : 0;
    long long paymentUnseen = payment > 0 ? payment : 0;

    if (!allUnseen)
    {
        mUi->bNumberNotifications->hide();
    }
    else
    {
        mUi->bNumberNotifications->setText(QString::number(allUnseen));
        mUi->bNumberNotifications->setFixedWidth(mUi->bNumberNotifications->fontMetrics().horizontalAdvance(QString::number(allUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotifications->show();
    }

    if (!contactsUnseen)
    {
        mUi->bNumberNotificationsContacts->hide();
    }
    else
    {
        mUi->bNumberNotificationsContacts->setText(QString::number(contactsUnseen));
        mUi->bNumberNotificationsContacts->setFixedWidth(mUi->bNumberNotificationsContacts->fontMetrics().horizontalAdvance(QString::number(contactsUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotificationsContacts->show();
    }

    if (!sharesUnseen)
    {
        mUi->bNumberNotificationsShares->hide();
    }
    else
    {
        mUi->bNumberNotificationsShares->setText(QString::number(sharesUnseen));
        mUi->bNumberNotificationsShares->setFixedWidth(mUi->bNumberNotificationsShares->fontMetrics().horizontalAdvance(QString::number(sharesUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotificationsShares->show();
    }

    if (!paymentUnseen)
    {
        mUi->bNumberNotificationsPayments->hide();
    }
    else
    {
        mUi->bNumberNotificationsPayments->setText(QString::number(paymentUnseen));
        mUi->bNumberNotificationsPayments->setFixedWidth(mUi->bNumberNotificationsPayments->fontMetrics().horizontalAdvance(QString::number(paymentUnseen)) + RED_BUBBLE_MARGIN);
        mUi->bNumberNotificationsPayments->show();
    }
    adjustSize();
}

void FilterAlertWidget::on_bAll_clicked()
{
    mCurrentFilter = MessageType::ALL;
    emit filterClicked(mCurrentFilter);
}

void FilterAlertWidget::on_bContacts_clicked()
{
    mCurrentFilter = MessageType::ALERT_CONTACTS;
    emit filterClicked(mCurrentFilter);
    QApplication::postEvent(mUi->bContacts, new QEvent(QEvent::Leave));
}

void FilterAlertWidget::on_bShares_clicked()
{
    mCurrentFilter = MessageType::ALERT_SHARES;
    emit filterClicked(mCurrentFilter);
    QApplication::postEvent(mUi->bShares, new QEvent(QEvent::Leave));
}

void FilterAlertWidget::on_bPayment_clicked()
{
    mCurrentFilter = MessageType::ALERT_PAYMENTS;
    emit filterClicked(mCurrentFilter);
    QApplication::postEvent(mUi->bPayment, new QEvent(QEvent::Leave));
}

bool FilterAlertWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
    }
    return QWidget::event(event);
}
