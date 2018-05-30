#include "StatusInfo.h"
#include "ui_StatusInfo.h"
#include <QMouseEvent>

StatusInfo::StatusInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusInfo)
{
    ui->setupUi(this);
    isHovered = false;
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
            ui->lStatusDesc->setText(tr("Paused"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/ico_pause_transfers_state.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bIconState->setIcon(icon);
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        case STATE_WAITING:
        {
            ui->lStatusDesc->setText(tr("Waiting"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/ico_menu_scanning_state.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bIconState->setIcon(icon);
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        case STATE_INDEXING:
        {
            ui->lStatusDesc->setText(tr("Scanning..."));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/ico_menu_scanning_state.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bIconState->setIcon(icon);
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        case STATE_UPDATED:
        {
            ui->lStatusDesc->setText(tr("Up to date"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/ico_menu_uptodate_state.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bIconState->setIcon(icon);
            ui->bIconState->setIconSize(QSize(24, 24));
            break;
        }
        default:
            break;
    }
}

void StatusInfo::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
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
