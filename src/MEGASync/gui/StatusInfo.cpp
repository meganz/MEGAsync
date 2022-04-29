#include "StatusInfo.h"
#include "ui_StatusInfo.h"
#include "Utilities.h"

StatusInfo::StatusInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusInfo)
{
    ui->setupUi(this);

    mIsOverQuota = false;

    mScanningTimer.setSingleShot(false);
    mScanningTimer.setInterval(60);
    mScanningAnimationIndex = 1;
    connect(&mScanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));
}

StatusInfo::~StatusInfo()
{
    delete ui;
}

void StatusInfo::setState(TRANSFERS_STATES state)
{
    this->mState = state;

    switch (this->mState)
    {
        case TRANSFERS_STATES::STATE_PAUSED:
        {
            if (mScanningTimer.isActive())
            {
                mScanningTimer.stop();
            }

            const QString statusText{tr("Paused")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/ico_pause_transfers_state.png")));
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        case TRANSFERS_STATES::STATE_UPDATED:
        {
            if (mScanningTimer.isActive())
            {
                mScanningTimer.stop();
            }

            if (mIsOverQuota)
            {
                const QString statusText{tr("Account full")};
                ui->lStatusDesc->setToolTip(statusText);
                ui->lStatusDesc->setText(statusText);
                ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/ico_menu_full.png")));
                ui->bIconState->setIconSize(QSize(24, 24));
            }
            else
            {
                const QString statusText{tr("Up to date")};
                ui->lStatusDesc->setToolTip(statusText);
                ui->lStatusDesc->setText(statusText);
                ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/ico_menu_uptodate_state.png")));
                ui->bIconState->setIconSize(QSize(24, 24));
            }

            break;
        }
        case TRANSFERS_STATES::STATE_SYNCING:
        {
            if (!mScanningTimer.isActive())
            {
                mScanningAnimationIndex = 1;
                mScanningTimer.start();
            }

            const QString statusText{tr("Syncing")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        case TRANSFERS_STATES::STATE_WAITING:
        {
            if (!mScanningTimer.isActive())
            {
                mScanningAnimationIndex = 1;
                mScanningTimer.start();
            }

            const QString statusText{tr("Waiting")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        case TRANSFERS_STATES::STATE_INDEXING:
        {
            if (!mScanningTimer.isActive())
            {
                mScanningAnimationIndex = 1;
                mScanningTimer.start();
            }

            const QString statusText{tr("Scanning")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        case TRANSFERS_STATES::STATE_TRANSFERRING:
        {
            if (!mScanningTimer.isActive())
            {
                mScanningAnimationIndex = 1;
                mScanningTimer.start();
            }

            const QString statusText{tr("Transferring")+QString::fromUtf8("...")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            break;
        }
        case TRANSFERS_STATES::STATE_FAILED:
        {
            if (mScanningTimer.isActive())
            {
                mScanningTimer.stop();
            }

            const QString statusText{QCoreApplication::translate("TransferManager","Some issues occurred")};
            ui->lStatusDesc->setToolTip(statusText);
            ui->lStatusDesc->setText(statusText);
            ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/sidebar_cancel_all_ico.png")));
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        default:
            break;
    }
}

StatusInfo::TRANSFERS_STATES StatusInfo::getState()
{
    return mState;
}

void StatusInfo::setOverQuotaState(bool oq)
{
    mIsOverQuota = oq;
    setState(mState);
}

QIcon StatusInfo::scanningIcon(int& index)
{
    index = index%12;
    index++;
    return Utilities::getCachedPixmap(
                                QString::fromUtf8(":/images/ico_menu_scanning_%1.png").arg(index));
}

void StatusInfo::scanningAnimationStep()
{
    ui->bIconState->setIcon(scanningIcon(mScanningAnimationIndex));
    ui->bIconState->setIconSize(QSize(24, 24));
}

void StatusInfo::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setState(mState);
    }
    QWidget::changeEvent(event);
}
