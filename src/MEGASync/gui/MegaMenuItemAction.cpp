#include "MegaMenuItemAction.h"

#include "MegaMenuItem.h"
#include "TokenParserWidgetManager.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QStyleOption>

MegaMenuItemAction::MegaMenuItemAction(const QString& text,
                                       const QString& iconName,
                                       int treeDepth,
                                       QObject* parent):
    QWidgetAction(parent),
    m_text(text),
    m_iconName(iconName),
    m_treeDepth(treeDepth),
    m_iconSpacing(8),
    m_textSpacing(0),
    m_beforeIconSpacing(0),
    m_itemHeight(40),
    m_itemWidth(240),
    m_submenu(nullptr)
{}

QWidget* MegaMenuItemAction::createWidget(QWidget* parent)
{
    m_item = new MegaMenuItem(m_text, m_iconName, m_treeDepth, menu() != nullptr, parent);
    m_item->setEnabled(isEnabled());
    // Install event filter to intercept paint events
    m_item->installEventFilter(this);

    // Apply all spacing settings
    m_item->setIconSpacing(m_iconSpacing);
    m_item->setTextSpacing(m_textSpacing);
    m_item->setBeforeIconSpacing(m_beforeIconSpacing);
    m_item->setFixedHeight(m_itemHeight);
    m_item->setHasSubmenu(m_submenu != nullptr);

    if (m_itemWidth > 0)
    {
        m_item->setFixedWidth(m_itemWidth);
    }
    return m_item;
}

void MegaMenuItemAction::deleteWidget(QWidget* widget)
{
    if (MegaMenuItem* item = qobject_cast<MegaMenuItem*>(widget))
    {
        item->deleteLater();
    }
}

bool MegaMenuItemAction::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Paint)
    {
        if (MegaMenuItem* item = qobject_cast<MegaMenuItem*>(watched))
        {
            paintItemBackground(item);
            // Don't return true - let the widget paint its children
        }
    }
    return QWidgetAction::eventFilter(watched, event);
}

void MegaMenuItemAction::paintItemBackground(MegaMenuItem* item)
{
    QStyleOption opt;
    opt.initFrom(item);

    // Check if this action is active in the menu
    QMenu* menu = qobject_cast<QMenu*>(item->parentWidget());
    if (menu && menu->activeAction() == this)
    {
        opt.state |= QStyle::State_Selected;

        // Check for pressed state
        if (QApplication::mouseButtons() & Qt::LeftButton)
        {
            opt.state |= QStyle::State_Sunken;
        }
    }

    // Determine color based on state
    QColor bgColor = Qt::transparent;
    if (opt.state & QStyle::State_Enabled)
    {
        if (opt.state & QStyle::State_Sunken)
        {
            bgColor = TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-2"));
        }
        else if (opt.state & QStyle::State_Selected)
        {
            bgColor = TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-1"));
        }
    }

    // Draw background before children are painted
    QPainter painter(item);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(item->rect(), 6, 6);
}

void MegaMenuItemAction::setIconSpacing(int spacing)
{
    m_iconSpacing = spacing;
    if (m_item)
    {
        m_item->setIconSpacing(spacing);
    }
}

void MegaMenuItemAction::setTextSpacing(int spacing)
{
    m_textSpacing = spacing;
    if (m_item)
    {
        m_item->setTextSpacing(spacing);
    }
}

void MegaMenuItemAction::setBeforeIconSpacing(int spacing)
{
    m_beforeIconSpacing = spacing;
    if (m_item)
    {
        m_item->setBeforeIconSpacing(spacing);
    }
}

void MegaMenuItemAction::setItemHeight(int height)
{
    m_itemHeight = height;
    if (m_item)
    {
        m_item->setFixedHeight(height);
    }
}

void MegaMenuItemAction::setItemWidth(int width)
{
    m_itemWidth = width;
    if (m_item)
    {
        m_item->setFixedWidth(width);
    }
}

void MegaMenuItemAction::setTreeDepth(int depth)
{
    m_treeDepth = depth;
    if (m_item)
    {
        m_item->setTreeDepth(depth);
    }
}

void MegaMenuItemAction::setLabelText(const QString& text)
{
    m_text = text;
    if (m_item)
    {
        m_item->setText(text);
    }
}

void MegaMenuItemAction::setActionIcon(const QString& icon)
{
    m_iconName = icon;
    if (m_item)
    {
        m_item->setIcon(icon);
    }
}

void MegaMenuItemAction::setMenu(QMenu* menu)
{
    QWidgetAction::setMenu(menu);
    m_submenu = menu;
    if (m_item)
    {
        m_item->setHasSubmenu(menu != nullptr);
    }
}
