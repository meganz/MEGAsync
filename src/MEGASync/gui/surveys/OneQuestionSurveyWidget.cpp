#include "OneQuestionSurveyWidget.h"

#include "QmlManager.h"

#include <QQuickWidget>

OneQuestionSurveyWidget::OneQuestionSurveyWidget(QWidget* parent):
    QQuickWidget(QmlManager::instance()->getEngine(), parent)
{
    setSource(QUrl(QStringLiteral("qrc:/surveys/OneQuestionSurveyItem.qml")));
    setClearColor(Qt::transparent);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    resize(parent->size());
}

void OneQuestionSurveyWidget::resizeEvent(QResizeEvent* event)
{
    if (parentWidget())
    {
        resize(parentWidget()->size());
    }
    QQuickWidget::resizeEvent(event);
}
