#include "Surveys.h"

// ************************************************************************************************
// * Surveys
// ************************************************************************************************

Surveys::Surveys(QObject* parent):
    QObject(parent),
    mCurrentSurveyId(-1)
{}

void Surveys::addSurvey(unsigned int triggerActionId)
{
    mSurveys.insert(triggerActionId, nullptr);
}

void Surveys::addSurvey(unsigned int triggerActionId, std::shared_ptr<Data> survey)
{
    auto it = mSurveys.find(triggerActionId);
    if (it == mSurveys.end())
    {
        mSurveys.insert(triggerActionId, survey);
    }
    else
    {
        it.value() = survey;
    }
}

std::shared_ptr<Surveys::Data> Surveys::currentSurvey() const
{
    return survey(mCurrentSurveyId);
}

std::shared_ptr<Surveys::Data> Surveys::survey(unsigned int triggerActionId) const
{
    auto it = mSurveys.find(triggerActionId);
    if (it != mSurveys.end())
    {
        return it.value();
    }
    return nullptr;
}

QString Surveys::getQuestion() const
{
    if (mCurrentSurveyId == -1)
    {
        return QString();
    }
    return mSurveys.value(mCurrentSurveyId)->question();
}

int Surveys::getCommentMaxLength() const
{
    if (mCurrentSurveyId == -1)
    {
        return -1;
    }
    return mSurveys.value(mCurrentSurveyId)->answer().maximumCommentLength();
}

int Surveys::currentSurveyId() const
{
    return mCurrentSurveyId;
}

void Surveys::setCurrentSurveyId(int newCurrentSurveyId)
{
    if (mCurrentSurveyId == newCurrentSurveyId)
    {
        return;
    }

    mCurrentSurveyId = newCurrentSurveyId;
    emit questionChanged();
    emit commentMaxLenghtChanged();
}

// ************************************************************************************************
// * Surveys::Data
// ************************************************************************************************

Surveys::Data::Data():
    mHandle(mega::INVALID_HANDLE)
{}

mega::MegaHandle Surveys::Data::handle() const
{
    return mHandle;
}

QString Surveys::Data::question() const
{
    return mQuestion;
}

Surveys::Data::Answer Surveys::Data::answer() const
{
    return mAnswer;
}

void Surveys::Data::setData(mega::MegaHandle handle,
                            const QString& question,
                            int maximumCommentLength)
{
    mHandle = handle;
    mQuestion = question;
    mAnswer.setMaximumCommentLength(maximumCommentLength);
}

void Surveys::Data::setAnswerData(int response, const QString& comment)
{
    mAnswer.setResponse(response);
    mAnswer.setComment(comment);
}

// ************************************************************************************************
// * Surveys::Data::Answer
// ************************************************************************************************

Surveys::Data::Answer::Answer():
    mResponse(-1),
    mMaximumCommentLength(-1)
{}

void Surveys::Data::Answer::setResponse(int newResponse)
{
    mResponse = newResponse;
}

void Surveys::Data::Answer::setMaximumCommentLength(int newMaximumCommentLength)
{
    mMaximumCommentLength = newMaximumCommentLength;
}

void Surveys::Data::Answer::setComment(const QString& newComment)
{
    mComment = newComment;
}

int Surveys::Data::Answer::response() const
{
    return mResponse;
}

int Surveys::Data::Answer::maximumCommentLength() const
{
    return mMaximumCommentLength;
}

QString Surveys::Data::Answer::comment() const
{
    return mComment;
}
