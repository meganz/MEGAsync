#include "MegaQuickWidget.h"

#include "QmlManager.h"
#include "ThemeManager.h"

#include <QStyle>

MegaQuickWidget::MegaQuickWidget(QWidget* parent):
    QQuickWidget(QmlManager::instance()->getEngine(), parent)
{
    // This allows for rendering of edges properly, but will no qt widgets can be drawn on top of
    // this
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setClearColor(Qt::transparent);
}

bool MegaQuickWidget::event(QEvent* event)
{
    if (event->type() == ThemeManager::ThemeChanged)
    {
        this->style()->unpolish(this);
        this->style()->polish(this);
    }
    return QQuickWidget::event(event);
}
