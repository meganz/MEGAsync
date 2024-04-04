#include "ThemeIconManager.h"

#include <QMouseEvent>
#include <QPushButton>

ThemeIconManager& ThemeIconManager::instance()
{
    static ThemeIconManager _instance;
    return _instance;
}

void ThemeIconManager::registerWidget(QWidget* widget, const IconStateInfo& icons)
{
    iconPaths[widget] = icons;

    widget->installEventFilter(this);
}

bool ThemeIconManager::eventFilter(QObject* watched, QEvent* event)
{
    auto widget = qobject_cast<QWidget*>(watched);
    if (!widget || !iconPaths.contains(widget))
    {
        return QObject::eventFilter(watched, event);
    }

    auto button = qobject_cast<QPushButton*>(widget);
    if (!button)
    {
        return QObject::eventFilter(watched, event);
    }

    switch (event->type())
    {
    case QEvent::Enter:
        button->setIcon(iconPaths[widget].hover);
        break;
    case QEvent::Leave:
        button->setIcon(iconPaths[widget].normal);
        break;
    case QEvent::MouseButtonPress:
        button->setIcon(iconPaths[widget].pressed);
        break;
    case QEvent::MouseButtonRelease:
        button->setIcon(widget->rect().contains(static_cast<QMouseEvent*>(event)->pos()) ? iconPaths[widget].hover : iconPaths[widget].normal);
        break;
    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}
