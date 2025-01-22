#ifndef SURVEY_CONTROLLER_H
#define SURVEY_CONTROLLER_H

#include <QObject>

#include <memory>

namespace mega
{
class MegaRequest;
class MegaError;
}

class Surveys;

class SurveyController: public QObject
{
    Q_OBJECT

public:
    SurveyController(int type, QObject* parent = nullptr);
    virtual ~SurveyController() = default;

    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

    void updateCurrentAnwer(int response, const QString& comment);
    void submitSurvey();

    std::shared_ptr<Surveys> surveys() const;

signals:
    void surveySubmitted();

private:
    std::shared_ptr<Surveys> mSurveys;
};

#endif // SURVEY_CONTROLLER_H
