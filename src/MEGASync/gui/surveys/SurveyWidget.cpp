#include "SurveyWidget.h"

#include "SurveyController.h"

SurveyWidget::SurveyWidget(QWidget* parent, int type):
    QmlWidgetWrapper<SurveyComponent>(parent, type)
{
    setClearColor(Qt::transparent);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    resize(parent->size());
}

void SurveyWidget::resizeEvent(QResizeEvent* event)
{
    if (parentWidget())
    {
        resize(parentWidget()->size());
    }
    QQuickWidget::resizeEvent(event);
}
