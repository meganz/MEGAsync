#include "StatusInfo.h"
#include "ui_StatusInfo.h"
#include <QMouseEvent>

StatusInfo::StatusInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusInfo)
{
    ui->setupUi(this);
    ui->bIconState->setAttribute(Qt::WA_TransparentForMouseEvents);

    isHovered = false;
    isOverQuota = false;

    scanningTimer.setSingleShot(false);
    scanningTimer.setInterval(60);
    scanningAnimationIndex = 1;
    connect(&scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));

    installEventFilter(this);
}

StatusInfo::~StatusInfo()
{
    delete ui;
}

void StatusInfo::setState(int state)
{
    this->state = state;
    if (isHovered)
    {
        return;
    }

    switch (this->state)
    {
        case STATE_PAUSED:
        {
            if (scanningTimer.isActive())
            {
                scanningTimer.stop();
            }

            ui->lStatusDesc->setText(tr("Paused"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/ico_pause_transfers_state.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bIconState->setIcon(icon);
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        case STATE_WAITING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            ui->lStatusDesc->setText(tr("Waiting"));
            break;
        }
        case STATE_INDEXING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            ui->lStatusDesc->setText(tr("Scanning..."));
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
                ui->lStatusDesc->setText(tr("Account full"));
                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/ico_menu_full.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->bIconState->setIcon(icon);
                ui->bIconState->setIconSize(QSize(24, 24));
            }
            else
            {
                ui->lStatusDesc->setText(tr("Up to date"));
                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/ico_menu_uptodate_state.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->bIconState->setIcon(icon);
                ui->bIconState->setIconSize(QSize(24, 24));
            }
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
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/images/ico_menu_scanning_")+
                 QString::number(scanningAnimationIndex) + QString::fromUtf8(".png") , QSize(), QIcon::Normal, QIcon::Off);

    ui->bIconState->setIcon(icon);
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

bool StatusInfo::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress
            && ((QMouseEvent *)e)->button() == Qt::LeftButton && isHovered)
    {
        isHovered = false;
        emit clicked();
    }
    else if (e->type() == QEvent::Enter)
    {
        if (scanningTimer.isActive())
        {
            scanningTimer.stop();
        }

        isHovered = true;
        if (state == STATE_PAUSED)
        {
            ui->lStatusDesc->setText(tr("Resume Transfers"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/ico_resume_transfers_state.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bIconState->setIcon(icon);
            ui->bIconState->setIconSize(QSize(24, 24));
        }
        else
        {
            ui->lStatusDesc->setText(tr("Pause Transfers"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/ico_pause_transfers_state.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bIconState->setIcon(icon);
            ui->bIconState->setIconSize(QSize(24, 24));
        }
    }
    else if (e->type() == QEvent::Leave)
    {
        isHovered = false;
        setState(state);
    }

    return QWidget::eventFilter(obj, e);
}
