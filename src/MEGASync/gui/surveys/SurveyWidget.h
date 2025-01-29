#ifndef SURVEY_WIDGET_H
#define SURVEY_WIDGET_H

#include "QmlWidgetWrapper.h"
#include "SurveyComponent.h"

class SurveyWidget: public QmlWidgetWrapper<SurveyComponent>
{
    Q_OBJECT

public:
    SurveyWidget(QWidget* parent, int type);
    virtual ~SurveyWidget() = default;

protected:
    virtual void resizeEvent(QResizeEvent* event);
};

#endif // SURVEY_WIDGET_H
