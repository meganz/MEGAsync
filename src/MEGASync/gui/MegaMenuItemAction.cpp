#include "MegaMenuItemAction.h"

#include "MegaMenuItem.h"

#include <QMenu>
#include <QTimer>

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
{
    setLabelText(text);
    setActionIcon(iconName);
}

QWidget* MegaMenuItemAction::createWidget(QWidget* parent)
{
    m_item = new MegaMenuItem(m_text, m_iconName, m_treeDepth, hasSubmenu(), parent);

    // Apply all spacing settings
    m_item->setIconSpacing(m_iconSpacing);
    m_item->setTextSpacing(m_textSpacing);
    m_item->setBeforeIconSpacing(m_beforeIconSpacing);
    m_item->setFixedHeight(m_itemHeight);

    if (m_itemWidth > 0)
    {
        m_item->setFixedWidth(m_itemWidth);
    }

    connect(m_item, &MegaMenuItem::clicked, this, &MegaMenuItemAction::onItemClicked);

    return m_item;
}

void MegaMenuItemAction::deleteWidget(QWidget* widget)
{
    if (MegaMenuItem* item = qobject_cast<MegaMenuItem*>(widget))
    {
        item->deleteLater();
    }
}

void MegaMenuItemAction::onItemClicked()
{
    if (m_submenu)
    {
        m_item->resetPressedState();
        if (QWidget* widget = qobject_cast<QWidget*>(sender()))
        {
            QPoint pos = widget->mapToGlobal(QPoint(widget->width(), 0));
            m_submenu->popup(pos);
        }
    }
    else
    {
        trigger();
    }
}

void MegaMenuItemAction::setIconSpacing(int spacing)
{
    m_iconSpacing = spacing;
}

void MegaMenuItemAction::setTextSpacing(int spacing)
{
    m_textSpacing = spacing;
}

void MegaMenuItemAction::setBeforeIconSpacing(int spacing)
{
    m_beforeIconSpacing = spacing;
}

void MegaMenuItemAction::setItemHeight(int height)
{
    m_itemHeight = height;
}

void MegaMenuItemAction::setItemWidth(int width)
{
    m_itemWidth = width;
}

void MegaMenuItemAction::setTreeDepth(int depth)
{
    m_treeDepth = depth;
}

void MegaMenuItemAction::setSubmenu(QMenu* submenu)
{
    m_submenu = submenu;
    const auto widgets = createdWidgets();
    for (QWidget* widget: widgets)
    {
        if (MegaMenuItem* item = qobject_cast<MegaMenuItem*>(widget))
        {
            item->setHasSubmenu(hasSubmenu());
        }
    }
}

void MegaMenuItemAction::setLabelText(const QString& text)
{
    m_text = text;
    if (m_item)
    {
        m_item->setText(m_text);
    }
}

void MegaMenuItemAction::setActionIcon(const QString& icon)
{
    m_iconName = icon;
    if (m_item)
    {
        m_item->setIcon(m_iconName);
    }
}
