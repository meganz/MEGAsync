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
    MultiStepIssueSolverBase(std::shared_ptr<StalledIssue> issue)
        : mDeadline(std::make_unique<QTimer>(this))
        , mIssue(issue)
    {
        mDeadline->setInterval(REQUEST_THRESHOLD);
        mDeadline->setSingleShot(true);
        //Auto destroy class when the timer finishes
        connect(mDeadline.get(), &QTimer::timeout, this, [this]()
        {
            deleteLater();
        });
    }
    virtual ~MultiStepIssueSolverBase() = default;

    virtual void resetDeadlineIfNeeded(const StalledIssueVariant& issue) = 0;
    virtual void resetDeadline()
    {
        mDeadline->start();
    }

    virtual bool checkIssue(std::shared_ptr<StalledIssue>) {return false;}

protected:
    std::unique_ptr<QTimer> mDeadline;
    std::shared_ptr<StalledIssue> mIssue;
};

template <class ISSUE_TYPE>
class MultiStepIssueSolver : public MultiStepIssueSolverBase
{
public:
    MultiStepIssueSolver(std::shared_ptr<ISSUE_TYPE> issue)
        : MultiStepIssueSolverBase(issue){}
    ~MultiStepIssueSolver()
    {
        ISSUE_TYPE::solvingIssueInSeveralStepsFinished();
        if(mIssue)
        {
            mIssue->setIsSolved(StalledIssue::SolveType::SOLVED);
        }
    }

    void resetDeadlineIfNeeded(const StalledIssueVariant& issue) override
    {
        auto issueType = issue.convert<ISSUE_TYPE>();
        if(issueType)
        {
            auto found(ISSUE_TYPE::findIssue(issueType));
            //Don´t reset the deadline if it was reset less than one second ago
            if(found && ((REQUEST_THRESHOLD - mDeadline->remainingTime()) > 1000))
            {
                resetDeadline();
            }
        }
    }

    std::shared_ptr<ISSUE_TYPE> getIssue() {return std::dynamic_pointer_cast<ISSUE_TYPE>(mIssue);}
};


#endif // SOLVEISSUEINSEVERALSTEPS_H
