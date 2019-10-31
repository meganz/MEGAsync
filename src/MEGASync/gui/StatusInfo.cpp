#include "StatusInfo.h"
#include "ui_StatusInfo.h"
#include "Utilities.h"
#include <QMouseEvent>

StatusInfo::StatusInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusInfo)
{
    ui->setupUi(this);
    ui->bIconState->setAttribute(Qt::WA_TransparentForMouseEvents);

    isHovered = false;
    isOverQuota = false;
    wasClickedLastTimeIn = false;

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
                ui->lStatusDesc->setText(tr("Account full"));
                ui->bIconState->setIcon(Utilities::getCachedPixmap(QString::fromUtf8(":/images/ico_menu_full.png")));
                ui->bIconState->setIconSize(QSize(24, 24));
            }
            else
            {
                ui->lStatusDesc->setText(tr("Up to date"));
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

            ui->lStatusDesc->setText(tr("Syncinc")+QString::fromUtf8("..."));
            break;
        }
        case STATE_WAITING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            ui->lStatusDesc->setText(tr("Waiting")+QString::fromUtf8("..."));
            break;
        }
        case STATE_INDEXING:
        {
            if (!scanningTimer.isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer.start();
            }

            ui->lStatusDesc->setText(tr("Scanning")+QString::fromUtf8("..."));
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

void StatusInfo::mouseMoveEvent(QMouseEvent *event)
{
    bool withintextarea = true;
    ui->lStatusDesc->ensurePolished();
    int textwidth = ui->lStatusDesc->fontMetrics().size(0, ui->lStatusDesc->text()).width(); //if it's elided width should exceed this widget's width
    QPoint pos = this->mapFromGlobal(QCursor::pos());
    if (pos.x() > ui->lStatusDesc->pos().x() + textwidth + ui->lStatusDesc->contentsRect().x())
    {
        withintextarea = false;
    }

    if (withintextarea)
    {
        if (!wasClickedLastTimeIn)
        {
            HoveredIn();
        }
    }
    else
    {
        HoveredOut();
    }
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

void StatusInfo::HoveredIn()
{
    if (scanningTimer.isActive())
    {
        scanningTimer.stop();
    }

    isHovered = true;
#ifndef Q_OS_MACX
    setCursor(Qt::PointingHandCursor);
#endif
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

void StatusInfo::HoveredOut()
{
    isHovered = false;
#ifndef Q_OS_MACX
    setCursor(Qt::ArrowCursor);
#endif

    wasClickedLastTimeIn = false;
    setState(state);
}

bool StatusInfo::eventFilter(QObject *obj, QEvent *e)
{
    bool withintextarea = true;
    ui->lStatusDesc->ensurePolished();
    int textwidth = ui->lStatusDesc->fontMetrics().size(0, ui->lStatusDesc->text()).width(); //if it's elided width should exceed this widget's width
    QPoint pos = this->mapFromGlobal(QCursor::pos());
    if (pos.x() > ui->lStatusDesc->pos().x() + textwidth + ui->lStatusDesc->contentsRect().x())
    {
        withintextarea = false;
    }

    if (e->type() == QEvent::MouseButtonPress
            && ((QMouseEvent *)e)->button() == Qt::LeftButton && isHovered && withintextarea)
    {
        isHovered = false;
        wasClickedLastTimeIn = true;
#ifndef Q_OS_MACX
    setCursor(Qt::ArrowCursor);
#endif
        emit clicked();
    }
    else if (e->type() == QEvent::Leave)
    {
        HoveredOut();
    }

    return QWidget::eventFilter(obj, e);
}
