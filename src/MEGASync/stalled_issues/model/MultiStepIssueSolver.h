#ifndef SOLVEISSUEINSEVERALSTEPS_H
#define SOLVEISSUEINSEVERALSTEPS_H

#include <MegaApplication.h>
#include <StalledIssue.h>

#include <QTimer>
#include <QDeadlineTimer>

const int REQUEST_THRESHOLD = 60000; /*60 seconds*/

class MultiStepIssueSolverBase : public QObject
{
    Q_OBJECT
public:
    MultiStepIssueSolverBase(StalledIssueSPtr issue);
    virtual ~MultiStepIssueSolverBase() = default;

    virtual void resetDeadlineIfNeeded(const StalledIssueVariant& issue) = 0;
    virtual void start();
    bool isActive() const;

    virtual bool checkIssue(const mega::MegaSyncStall*) const {return false;}

    void sendStartNotification();
    void sendFinishNotification();

    StalledIssueSPtr getIssue() const
    {
        return mIssue;
    }

    void setFailed();
    void setFinished();

signals:
    void solverFinished(MultiStepIssueSolverBase*);

protected slots:
    virtual void onDeadLineFinished() = 0;

protected:
    std::unique_ptr<QTimer> mDeadline;
    StalledIssueSPtr mIssue;
    bool mFailed;
    static int mSolversFixedInTheSameNotification;
    static int mSolversFailedInTheSameNotification;
    static int mSolversBeingFixedInTheSameNotification;
};

template <class ISSUE_TYPE>
class MultiStepIssueSolver : public MultiStepIssueSolverBase
{
public:
    MultiStepIssueSolver(std::shared_ptr<ISSUE_TYPE> issue)
        : MultiStepIssueSolverBase(issue){}
    virtual ~MultiStepIssueSolver() override
    {
    }

    virtual void resetDeadlineIfNeeded(const StalledIssueVariant& issue) override
    {
        auto issueType = issue.convert<ISSUE_TYPE>();
        if(issueType)
        {
            auto found(ISSUE_TYPE::findIssue(issueType));
            //Don´t reset the deadline if it was reset less than one second ago
            if(found && ((REQUEST_THRESHOLD - mDeadline->remainingTime()) > 1000))
            {
                start();
            }
        }
    }

    std::shared_ptr<ISSUE_TYPE> getIssue() {return std::dynamic_pointer_cast<ISSUE_TYPE>(mIssue);}

protected:
    void onDeadLineFinished() override
    {
        if(mIssue)
        {
            mIssue->finishAsyncIssueSolving();
        }

        setFinished();

        emit solverFinished(this);
    }
};


#endif // SOLVEISSUEINSEVERALSTEPS_H
