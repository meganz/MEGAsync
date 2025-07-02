#include "MegaQuickWidget.h"

#include "QmlManager.h"

MegaQuickWidget::MegaQuickWidget(QWidget* parent):
    QQuickWidget(QmlManager::instance()->getEngine(), parent)
{
    // This allows for rendering of edges properly, but will no qt widgets can be drawn on top of
    // this
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setClearColor(Qt::transparent);
}
