#ifndef ONE_QUESTION_SURVEY_WIDGET_H
#define ONE_QUESTION_SURVEY_WIDGET_H

#include <QQuickWidget>

class SurveyController;

class OneQuestionSurveyWidget: public QQuickWidget
{
    Q_OBJECT

public:
    OneQuestionSurveyWidget(QWidget* parent);
    ~OneQuestionSurveyWidget() = default;

public slots:
    void submitButtonClicked(int response, const QString& comment);

signals:
    void surveySubmitted();

protected:
    virtual void resizeEvent(QResizeEvent* event);

private:
    std::shared_ptr<SurveyController> mController;
};

#endif // ONE_QUESTION_SURVEY_WIDGET_H
