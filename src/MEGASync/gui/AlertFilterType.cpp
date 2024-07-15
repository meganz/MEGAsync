#include "AlertFilterType.h"
#include "ui_AlertFilterType.h"

AlertFilterType::AlertFilterType(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AlertFilterType)
    , mAllFilterHasBeenSelected(false)
{
    ui->setupUi(this);
    ui->lFilterActive->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->bDropDownArrow->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->liconType->setAttribute(Qt::WA_TransparentForMouseEvents);

    setActualFilter(UserMessageType::ALL);
}

AlertFilterType::~AlertFilterType()
{
    delete ui;
}

void AlertFilterType::setActualFilter(UserMessageType type)
{
    switch (type)
    {
        case UserMessageType::ALERT_CONTACTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/contacts.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Contacts"));
            break;
        }
        case UserMessageType::ALERT_PAYMENTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/payments.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Payment"));
            break;
        }
        case UserMessageType::ALERT_SHARES:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/incoming_share.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Incoming Shares"));
            break;
        }
        case UserMessageType::ALL:
        case UserMessageType::ALERT_TAKEDOWNS:
        default:
        {
            ui->wIconType->hide();
            ui->lFilterActive->setText(tr("All notifications"));
            mAllFilterHasBeenSelected = true;
            break;
        }
    }
    mType = type;
}

bool AlertFilterType::allFilterHasBeenSelected() const
{
    return mAllFilterHasBeenSelected || mType == UserMessageType::ALL;
}

void AlertFilterType::resetAllFilterHasBeenSelected()
{
    if(mType != UserMessageType::ALL)
    {
        mAllFilterHasBeenSelected = false;
    }
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
