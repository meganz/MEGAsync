#include "StalledIssueTab.h"
#include "ui_StalledIssueTab.h"

#include "Utilities.h"

StalledIssueTab::StalledIssueTab(QWidget *parent) :
    QFrame(parent),
    mItsOn(false),
    ui(new Ui::StalledIssueTab),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr))
{
    ui->setupUi(this);
}

StalledIssueTab::~StalledIssueTab()
{
    delete ui;
}

void StalledIssueTab::setTitle(const QString &title)
{
    mTitle = title;
    ui->title->setText(title);
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
        //If turned off, turn on and turn off siblings
        if(!itsOn())
        {
            toggleOffSiblings();

            setItsOn(true);

            updateIcon();
        }
    }

    QWidget::mouseReleaseEvent(event);
}

bool StalledIssueTab::itsOn() const
{
    return mItsOn;
}

void StalledIssueTab::setItsOn(bool itsOn)
{
    mItsOn = itsOn;
    updateIcon();

    if(mItsOn)
    {
        QColor shadowColor ("#E5E5E5");
        mShadowTab->setParent(this);
        mShadowTab->setBlurRadius(5);
        mShadowTab->setXOffset(0);
        mShadowTab->setYOffset(0);
        mShadowTab->setColor(shadowColor);
        mShadowTab->setEnabled(true);

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
}
