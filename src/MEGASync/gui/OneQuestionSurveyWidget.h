#ifndef ONE_QUESTION_SURVEY_WIDGET_H
#define ONE_QUESTION_SURVEY_WIDGET_H

#include <QQuickWidget>

class OneQuestionSurveyWidget: public QQuickWidget
{
public:
    OneQuestionSurveyWidget(QWidget* parent);

protected:
    virtual void resizeEvent(QResizeEvent* event);
};

#endif // ONE_QUESTION_SURVEY_WIDGET_H
