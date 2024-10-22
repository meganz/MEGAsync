#include "OneQuestionSurveyWidget.h"

#include "QmlManager.h"
#include "SurveyController.h"

#include <QQuickWidget>

OneQuestionSurveyWidget::OneQuestionSurveyWidget(QWidget* parent):
    QQuickWidget(QmlManager::instance()->getEngine(), parent),
    mController(std::make_shared<SurveyController>())
{
    QmlManager::instance()->setRootContextProperty(this);
    mController->registerQmlRootContextProperties();

    setSource(QUrl(QStringLiteral("qrc:/surveys/OneQuestionSurveyItem.qml")));
    setClearColor(Qt::transparent);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    resize(parent->size());

    connect(mController.get(),
            &SurveyController::surveySubmitted,
            this,
            &OneQuestionSurveyWidget::surveySubmitted,
            Qt::UniqueConnection);
}

void OneQuestionSurveyWidget::resizeEvent(QResizeEvent* event)
{
    if (parentWidget())
    {
        resize(parentWidget()->size());
    }
    QQuickWidget::resizeEvent(event);
}

void OneQuestionSurveyWidget::submitButtonClicked(int response, const QString& comment)
{
    mController->updateCurrentAnwer(response, comment);
    mController->submitSurvey();
}
