#include "TransfersStateInfoWidget.h"
#include "ui_TransfersStateInfoWidget.h"

TransfersStateInfoWidget::TransfersStateInfoWidget(QWidget *parent, int state) :
    QWidget(parent),
    ui(new Ui::TransfersStateInfoWidget)
{
    this->state = state;
    ui->setupUi(this);
    setState(state);
}

TransfersStateInfoWidget::~TransfersStateInfoWidget()
{
    delete ui;
}

void TransfersStateInfoWidget::setState(int newState)
{
    this->state = newState;
    switch(state)
    {
        case NO_DOWNLOADS:
            ui->lStatus->setText(tr("No Downloads"));
            ui->lStatusIcon->setIcon(QIcon(QString::fromAscii("://images/no_downloads.png")));
            break;
        case NO_UPLOADS:
            ui->lStatus->setText(tr("No Uploads"));
            ui->lStatusIcon->setIcon(QIcon(QString::fromAscii("://images/no_uploads.png")));
            break;
        case PAUSED:
            ui->lStatus->setText(tr("Paused Transfers"));
            ui->lStatusIcon->setIcon(QIcon(QString::fromAscii("://images/paused_transfers.png")));
            ui->wContainer->setStyleSheet((QString::fromAscii("background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                                                              "stop: 0 rgba(252, 252, 252, 90%), stop: 1 rgba(247, 247, 247, 90%));")));
            break;
        default:
            ui->lStatusIcon->setIcon(QIcon(QString::fromAscii("://images/completed_transfers.png")));
            break;
    }
}

void TransfersStateInfoWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setState(this->state);
    }
    QWidget::changeEvent(event);
}
