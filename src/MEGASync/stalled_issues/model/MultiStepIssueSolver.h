#ifndef SOLVEISSUEINSEVERALSTEPS_H
#define SOLVEISSUEINSEVERALSTEPS_H

#include <MegaApplication.h>
#include <StalledIssue.h>

#include <QTimer>
#include <QDeadlineTimer>

class MultiStepIssueSolverBase : public QObject
{
    Q_OBJECT
public:
    MultiStepIssueSolverBase() = default;
    virtual ~MultiStepIssueSolverBase() = default;

    bool hasExpired() const {return mDeadline.hasExpired();}
    virtual void resetDeadlineIfNeeded(const StalledIssueVariant& issue) = 0;
    virtual void resetDeadline() = 0;
    virtual void remove() = 0;

protected:
    QDeadlineTimer mDeadline;
};

const int REQUEST_THRESHOLD = 60000; /*60 seconds*/

template <class ISSUE_TYPE>
class MultiStepIssueSolver : public MultiStepIssueSolverBase
{
public:
    MultiStepIssueSolver(){}

    ~MultiStepIssueSolver(){}

    void resetDeadlineIfNeeded(const StalledIssueVariant& issue) override
    {
        auto issueType = issue.convert<ISSUE_TYPE>();
        if(issueType)
        {
            auto found(ISSUE_TYPE::findIssue(issueType));
            //Don´t reset the deadline if it was reset less than one second ago
            if(found && ((mDeadline.deadline() - mDeadline.remainingTime()) > 1000))
            {
                resetDeadline();
            }
        }
    }

    void resetDeadline() { mDeadline = QDeadlineTimer(REQUEST_THRESHOLD); }

    void remove() override
    {
        ISSUE_TYPE::solvingIssueInSeveralStepsFinished();
        delete this;
    }
};


#endif // SOLVEISSUEINSEVERALSTEPS_H
