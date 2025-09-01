#include "SideBarTab.h"

#include "ui_SideBarTab.h"
#include "Utilities.h"

#include <QDebug>
#include <QMouseEvent>

const char* SELECTED = "selected";

SideBarTab::SideBarTab(QWidget* parent):
    QWidget(parent),
    ui(new Ui::SideBarTab)
{
    ui->setupUi(this);

    // By default the counter is hidden
    ui->lCounter->hide();
    ui->lClose->hide();

    ui->lIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_StyledBackground, true);

    mSideBarsTopParent = Utilities::getTopParent<QDialog>(parent);

    setProperty(SELECTED, false);
}

SideBarTab::~SideBarTab()
{
    delete ui;
}

void SideBarTab::setTitle(const QString& title)
{
    ui->lTitle->setText(title);
}

QString SideBarTab::getTitle() const
{
    return ui->lTitle->text();
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

void SideBarTab::showCloseButton()
{
    ui->lClose->show();
    connect(ui->lClose,
            &QToolButton::clicked,
            this,
            [this]()
            {
                hide();
                emit hidden();
            });
}

void SideBarTab::setCounter(int count)
{
    ui->lCounter->show();
    ui->lCounter->setText(QString::number(count));
}

void SideBarTab::setSelected(bool state)
{
    if (property(SELECTED).toBool() != state)
    {
        ui->lIcon->setChecked(state);
        setProperty(SELECTED, state);
        setStyleSheet(styleSheet());

        if (state)
        {
            if (isHidden())
            {
                show();
            }

            toggleOffSiblings();
            emit clicked();
        }
    }
}

bool SideBarTab::event(QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        setSelected(true);
    }

    return QWidget::event(event);
}

void SideBarTab::toggleOffSiblings()
{
    if (!mSideBarsTopParent)
    {
        return;
    }

    QList<SideBarTab*> siblings = mSideBarsTopParent->findChildren<SideBarTab*>();

    foreach(auto& tab, siblings)
    {
        if (tab != this)
        {
            tab->setSelected(false);
        }
    }
}
