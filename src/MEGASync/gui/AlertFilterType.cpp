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

    setActualFilter(AlertType::ALL);
}

AlertFilterType::~AlertFilterType()
{
    delete ui;
}

void AlertFilterType::setActualFilter(AlertType type)
{
    switch (type)
    {
        case AlertType::CONTACTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/contacts.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Contacts"));
            break;
        }
        case AlertType::PAYMENTS:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/payments.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Payment"));
            break;
        }
        case AlertType::SHARES:
        {
            ui->liconType->setIcon(QIcon(QString::fromUtf8(":/images/incoming_share.png")).pixmap(6.0, 6.0));
            ui->wIconType->show();
            ui->lFilterActive->setText(tr("Incoming Shares"));
            break;
        }
        case AlertType::ALL:
        case AlertType::TAKEDOWNS:
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
    return mAllFilterHasBeenSelected || mType == AlertType::ALL;
}

void AlertFilterType::resetAllFilterHasBeenSelected()
{
    if(mType != AlertType::ALL)
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
