#include "TabSelector.h"

#include "TokenizableItems/TokenPropertySetter.h"
#include "ui_TabSelector.h"
#include "Utilities.h"

#include <QDebug>
#include <QMouseEvent>

const char* SELECTED = "selected";
const char* TAB_SELECTOR_GROUP = "tabselector_group";

/*
 *  TabSelector items are autoExclusive.
 *  TabSelectors that belong to the same parent widget (or the parent with a dynamic property
 * TAB_SELECTOR_GROUP set) behave as if they were part of the same exclusive button group.
 */

TabSelector::TabSelector(QWidget* parent):
    QWidget(parent),
    ui(new Ui::TabSelector)
{
    ui->setupUi(this);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // By default the counter and the close button are hidden
    ui->lCounter->hide();
    ui->lClose->hide();

    connect(ui->lClose,
            &QPushButton::clicked,
            this,
            [this]()
            {
                hide();
                emit hidden();
            });

    setAttribute(Qt::WA_StyledBackground, true);

    // Look for a parent with the TAB_SELECTOR_GROUP property
    // If not found, use the direct parent
    mTabSelectorGroupParent =
        Utilities::getParent(this,
                             [](QWidget* parent)
                             {
                                 return parent->property(TAB_SELECTOR_GROUP).toBool();
                             });

    if (!mTabSelectorGroupParent)
    {
        mTabSelectorGroupParent = parentWidget();
    }

    setProperty(SELECTED, false);
}

TabSelector::~TabSelector()
{
    delete ui;
}

void TabSelector::setTitle(const QString& title)
{
    ui->lTitle->setText(title);
}

QString TabSelector::getTitle() const
{
    return ui->lTitle->text();
}

void TabSelector::setIcon(const QIcon& icon)
{
    ui->lIcon->setIcon(icon);
}

QIcon TabSelector::getIcon() const
{
    return ui->lIcon->icon();
}

QSize TabSelector::getIconSize() const
{
    return ui->lIcon->iconSize();
}

void TabSelector::setCloseButtonVisible(bool state)
{
    ui->lClose->setVisible(state);
}

bool TabSelector::isCloseButtonVisible() const
{
    return ui->lClose->isVisible();
}

void TabSelector::setCounter(int count)
{
    ui->lCounter->show();
    ui->lCounter->setText(QString::number(count));
}

void TabSelector::setSelected(bool state)
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

bool TabSelector::event(QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        setSelected(true);
    }

    return QWidget::event(event);
}

QList<TabSelector*> TabSelector::getTabSelectorByParent(QWidget* parent)
{
    return parent->findChildren<TabSelector*>();
}

void TabSelector::setIconTokens(const std::shared_ptr<TokenPropertySetter>& newIconTokens)
{
    mIconTokens = newIconTokens;
    mIconTokens->applyTokens(ui->lIcon);
}

void TabSelector::applyTokens(QWidget* parent,
                              std::shared_ptr<TokenPropertySetter> iconTokensSetter)
{
    auto tabs = getTabSelectorByParent(parent);

    for (auto& tab: tabs)
    {
        tab->setIconTokens(iconTokensSetter);
    }
}

void TabSelector::toggleOffSiblings()
{
    if (!mTabSelectorGroupParent)
    {
        return;
    }

    /* For a future dev: if you don´t want to affect all the dialog tabs
     * we may need to specify the number of levels that will be affected above the tab
     * For example: setLevelsAffected(2), we will try to find all the tabs in the parent of the
     * parent
     */

    QList<TabSelector*> siblings = mTabSelectorGroupParent->findChildren<TabSelector*>();

    foreach(auto& tab, siblings)
    {
        if (tab != this)
        {
            tab->setSelected(false);
        }
    }
}
