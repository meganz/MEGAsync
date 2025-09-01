#include "SideBarTab.h"

#include "ui_SideBarTab.h"
#include "Utilities.h"

#include <QDebug>
#include <QMouseEvent>

SideBarTab::SideBarTab(QWidget* parent):
    QWidget(parent),
    ui(new Ui::SideBarTab)
{
    ui->setupUi(this);

    // By default the counter is hidden
    ui->lCounter->hide();

    ui->lIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_StyledBackground, true);

    mSideBarsTopParent = Utilities::getTopParent<QDialog>(parent);
}

SideBarTab::~SideBarTab()
{
    delete ui;
}

void SideBarTab::setTitle(const QString& title)
{
    ui->lTitle->setText(title);
}

void SideBarTab::setIcon(const QIcon& icon)
{
    ui->lIcon->setIcon(icon);
    auto checkedPixmap = icon.pixmap(getIconSize(), QIcon::Mode::Normal, QIcon::State::On);
    qDebug() << checkedPixmap.isNull();
}

QIcon SideBarTab::getIcon() const
{
    return ui->lIcon->icon();
}

QSize SideBarTab::getIconSize() const
{
    return ui->lIcon->iconSize();
}

void SideBarTab::setCounter(int count)
{
    ui->lCounter->show();
    ui->lCounter->setText(QString::number(count));
}

void SideBarTab::setSelected(bool state)
{
    ui->lIcon->setChecked(state);
    setProperty("selected", state);
    setStyleSheet(styleSheet());
}

bool SideBarTab::event(QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        setSelected(true);
        toggleOffSiblings(false);
        emit clicked();
    }

    return QWidget::event(event);
}

void SideBarTab::toggleOffSiblings(bool toggleMySelf)
{
    if (!mSideBarsTopParent)
    {
        return;
    }

    QList<SideBarTab*> siblings = mSideBarsTopParent->findChildren<SideBarTab*>();

    foreach(auto& tab, siblings)
    {
        if (toggleMySelf || tab != this)
        {
            tab->setSelected(false);
        }
    }
}
