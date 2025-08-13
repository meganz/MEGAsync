#include "StalledIssueTab.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "ui_StalledIssueTab.h"
#include "Utilities.h"

const char* StalledIssueTab::HOVER_PROPERTY = "itsHover";

StalledIssueTab::StalledIssueTab(QWidget* parent):
    QFrame(parent),
    ui(new Ui::StalledIssueTab),
    mIsSelected(false),
    mFilterCriterion(-1),
    mShadowTab(new QGraphicsDropShadowEffect(nullptr))
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

void StalledIssueTab::setIconName(const QString& icon)
{
    mIconName = icon;
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

bool StalledIssueTab::isSelected() const
{
    return mIsSelected;
}

bool StalledIssueTab::toggleTab()
{
    //If turned off, turn on and turn off siblings
    if (!isSelected())
    {
        toggleOffSiblings();

        setIsSelected(true);

        updateIcon();

        return true;
    }

    return false;
}

void StalledIssueTab::setIsSelected(bool selected)
{
    mIsSelected = selected;
    updateIcon();

    if (mIsSelected)
    {
        emit tabToggled(static_cast<StalledIssueFilterCriterion>(mFilterCriterion));
    }

    setStyleSheet(styleSheet());
}

void StalledIssueTab::updateIcon()
{
    if (!mIconName.isEmpty())
    {
        ui->icon->setPixmap(Utilities::getPixmap(mIconName,
                                                 isSelected() ? Utilities::AttributeType::SOLID :
                                                                Utilities::AttributeType::OUTLINE,
                                                 ui->icon));
    }
}

void StalledIssueTab::toggleOffSiblings()
{
    QList<StalledIssueTab*> siblings = parentWidget()->findChildren<StalledIssueTab *>();

    foreach(auto& headerItem, siblings)
    {
        if(headerItem != this)
        {
            headerItem->setIsSelected(false);
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

bool StalledIssueTab::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        createTitle();
    }

    return QFrame::event(event);
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
