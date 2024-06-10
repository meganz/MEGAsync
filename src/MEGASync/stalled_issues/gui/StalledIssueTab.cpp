#include "StalledIssueTab.h"
#include "ui_StalledIssueTab.h"

#include "Utilities.h"
#include "StalledIssuesModel.h"
#include "MegaApplication.h"

const char* StalledIssueTab::HOVER_PROPERTY = "itsHover";

StalledIssueTab::StalledIssueTab(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StalledIssueTab),
    mItsOn(false),
    mFilterCriterion(-1),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr))
{
    ui->setupUi(this);
    ui->title->installEventFilter(this);


    connect(MegaSyncApp->getStalledIssuesModel(),
            &StalledIssuesModel::stalledIssuesCountChanged, this, &StalledIssueTab::onUpdateCounter);
}

StalledIssueTab::~StalledIssueTab()
{
    delete ui;
}

void StalledIssueTab::setIconPrefix(const QString &iconPrefix)
{
    mIconPrefix = iconPrefix;
    updateIcon();
}

void StalledIssueTab::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(toggleTab())
        {
            //We don´t want to propagate it to the stalled issue dialog
            return;
        }
    }

    QWidget::mouseReleaseEvent(event);
}

void StalledIssueTab::enterEvent(QEvent*)
{
   if(isEnabled())
   {
       setProperty(HOVER_PROPERTY,true);
       setStyleSheet(styleSheet());

       if(!mIconPrefix.isEmpty())
       {
           QString iconName =  QString::fromUtf8(":images/StalledIssues/") + mIconPrefix
                   + QString::fromUtf8("-solid")
                   + QString::fromUtf8(".png");
           QIcon icon = Utilities::getCachedPixmap(iconName);

           ui->icon->setPixmap(icon.pixmap(ui->icon->size()));
       }
   }
}

void StalledIssueTab::leaveEvent(QEvent*)
{
    setProperty(HOVER_PROPERTY,false);
    setStyleSheet(styleSheet());
    updateIcon();
}

void StalledIssueTab::onUpdateCounter()
{
    createTitle();
}

bool StalledIssueTab::itsOn() const
{
    return mItsOn;
}

bool StalledIssueTab::toggleTab()
{
    //If turned off, turn on and turn off siblings
    if(!itsOn())
    {
        toggleOffSiblings();

        setItsOn(true);

        updateIcon();

        return true;
    }

    return false;
}

void StalledIssueTab::setItsOn(bool itsOn)
{
    mItsOn = itsOn;
    updateIcon();

    if(mItsOn)
    {
        emit tabToggled(static_cast<StalledIssueFilterCriterion>(mFilterCriterion));
    }

    setStyleSheet(styleSheet());
}

void StalledIssueTab::updateIcon()
{
    if(!mIconPrefix.isEmpty())
    {
        QString iconName =  QString::fromUtf8(":images/StalledIssues/") + mIconPrefix
                + (mItsOn ? QString::fromUtf8("-solid") : QString::fromUtf8("-outline"))
                + QString::fromUtf8(".png");
        QIcon icon = Utilities::getCachedPixmap(iconName);

        ui->icon->setPixmap(icon.pixmap(ui->icon->size()));
    }
}

void StalledIssueTab::toggleOffSiblings()
{
    QList<StalledIssueTab*> siblings = parentWidget()->findChildren<StalledIssueTab *>();

    foreach(auto& headerItem, siblings)
    {
        if(headerItem != this)
        {
            headerItem->setItsOn(false);
        }
    }
}

int StalledIssueTab::filterCriterion() const
{
    return mFilterCriterion;
}

void StalledIssueTab::setFilterCriterion(int filterCriterion)
{
    mFilterCriterion = filterCriterion;
    createTitle();
}

void StalledIssueTab::createTitle()
{
    const auto itemsCount = MegaSyncApp->getStalledIssuesModel()->getCountByFilterCriterion(
        static_cast<StalledIssueFilterCriterion>(mFilterCriterion));
    switch (static_cast<StalledIssueFilterCriterion>(mFilterCriterion))
    {
        case StalledIssueFilterCriterion::ALL_ISSUES:
            mTitle = tr("All issues: %1").arg(itemsCount);
            break;
        case StalledIssueFilterCriterion::NAME_CONFLICTS:
            mTitle = tr("Name conflict: %n", "", itemsCount);
            break;
        case StalledIssueFilterCriterion::ITEM_TYPE_CONFLICTS:
            mTitle = tr("Item type conflict: %n", "", itemsCount);
            break;
        case StalledIssueFilterCriterion::OTHER_CONFLICTS:
            mTitle = tr("Other: %n", "", itemsCount);
            break;
        case StalledIssueFilterCriterion::FAILED_CONFLICTS:
            mTitle = tr("Failed: %n", "", itemsCount);
            break;
        case StalledIssueFilterCriterion::SOLVED_CONFLICTS:
            mTitle = tr("Resolved: %n", "", itemsCount);
            break;
        default:
            mTitle = QString();
    }

    ui->title->setText(mTitle);
}

void StalledIssueTab::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        createTitle();
    }

    QFrame::changeEvent(event);
}

void StalledIssueTab::resizeEvent(QResizeEvent *event)
{
    createTitle();
}

bool StalledIssueTab::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->title && event->type() == QEvent::Paint)
    {
        QPainter painter(ui->title);
        QRect rect = ui->title->contentsRect();
        QString elidedText = ui->title->fontMetrics().elidedText(mTitle, Qt::ElideMiddle, rect.width());
        painter.drawText(rect, Qt::AlignVCenter, elidedText);
        return true;
    }

    return QFrame::eventFilter(watched, event);
}
