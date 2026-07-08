#include "TabSelector.h"

#include "ArrowTooltip.h"
#include "TokenizableItems/TokenPropertySetter.h"
#include "ui_TabSelector.h"
#include "Utilities.h"

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>

const char* SELECTED = "selected";
const char* HOVER = "hover";
const char* TAB_SELECTOR_GROUP = "tabselector_group";

/*
 *  TabSelector items are autoExclusive.
 *  TabSelectors that belong to the same parent widget (or the parent with a dynamic property
 * TAB_SELECTOR_GROUP set) behave as if they were part of the same exclusive button group.
 */

TabSelector::TabSelector(QWidget* parent):
    QWidget(parent),
    ui(new Ui::TabSelector),
    mConnectedToDropEvent(false),
    mCloseButtonVisible(false),
    mIconOnly(false)
{
    ui->setupUi(this);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // By default the counter and the close button are hidden
    ui->lCounter->hide();
    ui->lClose->hide();

    ui->lTitle->installEventFilter(this);

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
    closeCollapsedTooltip();
    delete ui;
}

void TabSelector::setTitle(const QString& title)
{
    mTitle = title;
    ui->lTitle->setText(mTitle);
}

QString TabSelector::getTitle() const
{
    return mTitle;
}

void TabSelector::setIcon(const QIcon& icon)
{
    ui->lIcon->setIcon(icon);
}

QIcon TabSelector::getIcon() const
{
    return ui->lIcon->icon();
}

void TabSelector::setIconSize(const QSize& size)
{
    ui->lIcon->setFixedSize(size);
    ui->lIcon->setIconSize(size);
}

QSize TabSelector::getIconSize() const
{
    return ui->lIcon->iconSize();
}

void TabSelector::setCloseButtonVisible(bool state)
{
    mCloseButtonVisible = state;
    ui->lClose->setVisible(mCloseButtonVisible && !mIconOnly);
}

bool TabSelector::isCloseButtonVisible() const
{
    return mCloseButtonVisible;
}

void TabSelector::setCounter(unsigned long long count)
{
    auto currentValue(ui->lCounter->text().toULongLong());

    if (currentValue != count)
    {
        if (mIconOnly)
        {
            ui->lCounter->setText(QString());
            ui->lCounter->hide();
            return;
        }

        if (count > 0)
        {
            ui->lCounter->show();
            ui->lCounter->setText(QString::number(count));
        }
        else
        {
            ui->lCounter->setText(QString());
            ui->lCounter->hide();
        }
    }
}

bool TabSelector::hasEmptyCount()
{
    auto currentValue(ui->lCounter->text().toULongLong());
    return currentValue <= 0;
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

bool TabSelector::isSelected() const
{
    return property(SELECTED).toBool();
}

bool TabSelector::isIconOnly() const
{
    return mIconOnly;
}

void TabSelector::setIconOnly(bool state)
{
    if (mIconOnly == state)
    {
        return;
    }

    mIconOnly = state;

    if (mIconOnly)
    {
        layout()->setContentsMargins(0, 0, 0, 0);
        layout()->setAlignment(ui->lIcon, Qt::AlignCenter);

        ui->lTitle->hide();
        ui->lCounter->hide();
        ui->lClose->setVisible(false);
        ui->lTitle->setText(QString());
    }
    else
    {
        layout()->setAlignment(ui->lIcon, Qt::AlignVCenter);

        ui->lCounter->setVisible(hasEmptyCount());
        ui->lTitle->show();
        closeCollapsedTooltip();
    }
}

void TabSelector::setNormalOff(const QString& token)
{
    mNormalOff = token;
    ui->lIcon->setProperty("normal_off", token);
}

QString TabSelector::getNormalOff() const
{
    return mNormalOff;
}

void TabSelector::setNormalOn(const QString& token)
{
    mNormalOn = token;
    ui->lIcon->setProperty("normal_on", token);
}

QString TabSelector::getNormalOn() const
{
    return mNormalOn;
}

bool TabSelector::event(QEvent* event)
{
    if (isEnabled())
    {
        if (event->type() == QEvent::MouseButtonRelease)
        {
            setSelected(true);
        }
        else if (event->type() == QEvent::Enter || event->type() == QEvent::Leave)
        {
            const bool entering = event->type() == QEvent::Enter;
            setProperty(HOVER, entering);
            setStyleSheet(styleSheet());

            if (mIconOnly && entering && !mTitle.isEmpty())
            {
                showCollapsedTooltip();
            }
            else
            {
                closeCollapsedTooltip();
            }
        }
    }

    return QWidget::event(event);
}

bool TabSelector::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Paint)
    {
        if (watched == ui->lTitle && event->type() == QEvent::Paint)
        {
            QPainter painter(ui->lTitle);
            QRect rect = ui->lTitle->contentsRect();
            QString elidedText =
                ui->lTitle->fontMetrics().elidedText(mTitle, Qt::ElideMiddle, rect.width());
            painter.drawText(rect, Qt::AlignVCenter, elidedText);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

// You need to use setAcceptDrops externally in order to activate this feature
void TabSelector::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
    setSelected(true);
}

void TabSelector::dropEvent(QDropEvent* event)
{
    if (mConnectedToDropEvent)
    {
        std::shared_ptr<QDropEvent> newEvent =
            std::make_shared<QDropEvent>(QPoint(-1, -1),
                                         event->possibleActions(),
                                         event->mimeData(),
                                         event->mouseButtons(),
                                         event->keyboardModifiers(),
                                         event->type());

        emit dropOnTabSelector(newEvent);
        event->accept();
    }
    else
    {
        event->ignore();
    }
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

void TabSelector::selectTabIf(QWidget* parent, const char* property, const QVariant& value)
{
    auto tabs = getTabSelectorByParent(parent);

    for (auto& tab: tabs)
    {
        if (tab->property(property) == value)
        {
            tab->setSelected(true);
        }
    }
}

void TabSelector::applyActionToTabSelectors(QWidget* parent, std::function<void(TabSelector*)> func)
{
    auto tabs = getTabSelectorByParent(parent);

    for (auto& tab: tabs)
    {
        func(tab);
    }
}

void TabSelector::deselectAll(QWidget* parent)
{
    applyActionToTabSelectors(parent,
                              [](TabSelector* tab)
                              {
                                  tab->setSelected(false);
                              });
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

void TabSelector::hideIcon()
{
    ui->lIcon->setVisible(false);
}

void TabSelector::hide()
{
    closeCollapsedTooltip();
    setCounter(0);
    QWidget::hide();
}

void TabSelector::showCollapsedTooltip()
{
    if (!mIconOnly || mTitle.isEmpty())
    {
        return;
    }

    closeCollapsedTooltip();

    auto* tooltip = new ArrowTooltip(this);
    tooltip->setText(mTitle);

    static constexpr int H_GAP = 8;
    const int x = mapToGlobal(QPoint(width() + H_GAP, 0)).x();
    const int y = mapToGlobal(QPoint(0, (height() - tooltip->height()) / 2)).y();
    tooltip->move(x, y);
    tooltip->show();

    mTooltip = tooltip;
}

void TabSelector::closeCollapsedTooltip()
{
    if (mTooltip)
    {
        mTooltip->close();
        mTooltip = nullptr;
    }
}

void TabSelector::connectToDropEvent(std::function<void(std::shared_ptr<QDropEvent>)> slot)
{
    mConnectedToDropEvent = true;
    connect(this, &TabSelector::dropOnTabSelector, this, slot);
}
