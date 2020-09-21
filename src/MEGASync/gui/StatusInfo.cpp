#include "StatusInfo.h"
#include "ui_StatusInfo.h"
#include "Utilities.h"

StatusInfo::StatusInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusInfo)
{
    ui->setupUi(this);

    isOverQuota = false;

    scanningTimer.setSingleShot(false);
    scanningTimer.setInterval(60);
    scanningAnimationIndex = 1;
    connect(&scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));
}

StatusInfo::~StatusInfo()
{
    delete ui;
}

void StatusInfo::setState(int state)
{
    this->state = state;

    switch (this->state)
    {
        case STATE_PAUSED:
        {
            if (scanningTimer.isActive())
            {
                scanningTimer.stop();
            }

            const auto statusText{tr("Paused")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/ico_pause_transfers_state.png")));
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        case STATE_UPDATED:
        {
            if (scanningTimer.isActive())
            {
                scanningTimer.stop();
            }

            if (isOverQuota)
            {
                const auto statusText{tr("Account full")};
                ui->lStatusDesc->setToolTip(statusText);
                ui->lStatusDesc->setText(statusText);
                ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/ico_menu_full.png")));
                ui->bIconState->setIconSize(QSize(24, 24));
            }
            else
            {
                const auto statusText{tr("Up to date")};
                ui->lStatusDesc->setToolTip(statusText);
                ui->lStatusDesc->setText(statusText);
                ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/ico_menu_uptodate_state.png")));
                ui->bIconState->setIconSize(QSize(24, 24));
            }

            break;
        }
        case STATE_SYNCING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            const auto statusText{tr("Syncing")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        case STATE_WAITING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            const auto statusText{tr("Waiting")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        case STATE_INDEXING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            const auto statusText{tr("Scanning")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        case STATE_TRANSFERRING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            const auto statusText{tr("Transferring")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        default:
            break;
    }
}

void StatusInfo::setOverQuotaState(bool oq)
{
    isOverQuota = oq;
    setState(state);
}

void StatusInfo::scanningAnimationStep()
{
    scanningAnimationIndex = scanningAnimationIndex%12;
    scanningAnimationIndex++;
    ui->bIconState->setIcon(Utilities::getCachedPixmap(
                                QString::fromUtf8(":/images/ico_menu_scanning_%1.png").arg(scanningAnimationIndex)));
    ui->bIconState->setIconSize(QSize(24, 24));
}

void StatusInfo::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setState(state);
    }
    QWidget::changeEvent(event);
}
