#ifndef SURVEY_COMPONENT_H
#define SURVEY_COMPONENT_H

#include "QmlWidgetWrapper.h"

#include <memory>

class SurveyController;

class SurveyComponent: public QMLWidgetComponent
{
    Q_OBJECT

public:
    SurveyComponent(QObject* parent, int type);
    ~SurveyComponent() = default;

    QUrl getQmlUrl() override;
    QList<QObject*> getInstances() override;

public slots:
    void submitButtonClicked(int response, const QString& comment);

signals:
    void surveySubmitted();

private:
    std::shared_ptr<SurveyController> mController;
};

#endif // SURVEY_COMPONENT_H
