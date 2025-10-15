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

    setActualFilter(MessageType::ALL);
}

AlertFilterType::~AlertFilterType()
{
    delete ui;
}

void AlertFilterType::setActualFilter(MessageType type)
{
    switch (type)
    {
        case MessageType::ALERT_CONTACTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/users.svg")).pixmap(16.0, 16.0));
            ui->wIconType->hide();
            ui->lFilterActive->setText(tr("Contacts"));
            break;
        }
        case MessageType::ALERT_PAYMENTS:
        {
            ui->liconType->setIcon(
                QIcon(QString::fromUtf8(":/currency-dollar.svg")).pixmap(16.0, 16.0));
            ui->wIconType->hide();
            ui->lFilterActive->setText(tr("Payments"));
            break;
        }
        case MessageType::ALERT_SHARES:
        {
            ui->liconType->setIcon(
                QIcon(QString::fromUtf8(":/folder-users.svg")).pixmap(16.0, 16.0));
            ui->wIconType->hide();
            ui->lFilterActive->setText(tr("Incoming Shares"));
            break;
        }
        case MessageType::ALL:
        case MessageType::ALERT_TAKEDOWNS:
        default:
        {
            ui->wIconType->hide();
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/bell.svg")).pixmap(16.0, 16.0));

            ui->lFilterActive->setText(tr("All notifications"));
            mAllFilterHasBeenSelected = true;
            break;
        }
    }
    mType = type;
}

bool AlertFilterType::allFilterHasBeenSelected() const
{
    return mAllFilterHasBeenSelected || mType == MessageType::ALL;
}

void AlertFilterType::resetAllFilterHasBeenSelected()
{
    if(mType != MessageType::ALL)
    {
        mAllFilterHasBeenSelected = false;
    }
}

void AlertFilterType::mousePressEvent(QMouseEvent*)
{
    emit clicked();
}

bool AlertFilterType::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setActualFilter(mType);
    }
    return QWidget::event(event);
}
