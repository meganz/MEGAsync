#ifndef SOLVEISSUEINSEVERALSTEPS_H
#define SOLVEISSUEINSEVERALSTEPS_H

#include <MegaApplication.h>
#include <StalledIssue.h>

#include <QTimer>
#include <QDeadlineTimer>

const int REQUEST_THRESHOLD = 20000; /*60 seconds*/

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
        connect(mDeadline.get(), &QTimer::timeout, this, [this]() { deleteLater(); });
    }
    virtual ~MultiStepIssueSolverBase() = default;

    virtual void resetDeadlineIfNeeded(const StalledIssueVariant& issue) = 0;
    virtual void start()
    {
        mDeadline->start();
    }

    bool isActive() const {return mDeadline->isActive();}

    virtual bool checkIssue(const mega::MegaSyncStall*) const {return false;}

    void setStartNotification(const DesktopNotifications::NotificationInfo& newStartNotification);
    void setFinishNotification(const DesktopNotifications::NotificationInfo& newFinishNotification);

    std::shared_ptr<StalledIssue> getIssue() const {return mIssue;}

protected:
    std::unique_ptr<QTimer> mDeadline;
    std::shared_ptr<StalledIssue> mIssue;
    DesktopNotifications::NotificationInfo mFinishNotification;
};

inline void MultiStepIssueSolverBase::setStartNotification(
    const DesktopNotifications::NotificationInfo& newStartNotification)
{
    MegaSyncApp->showInfoMessage(newStartNotification);
}

inline void MultiStepIssueSolverBase::setFinishNotification(
    const DesktopNotifications::NotificationInfo& newFinishNotification)
{
    mFinishNotification = newFinishNotification;
}

template <class ISSUE_TYPE>
class MultiStepIssueSolver : public MultiStepIssueSolverBase
{
public:
    MultiStepIssueSolver(std::shared_ptr<ISSUE_TYPE> issue)
        : MultiStepIssueSolverBase(issue){}
    virtual ~MultiStepIssueSolver() override
    {
        if(mIssue)
        {
            mIssue->finishAsyncIssueSolving();
        }

        //Send notification
        if(mFinishNotification.isValid())
        {
            MegaSyncApp->showInfoMessage(mFinishNotification);
        }
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
};


#endif // SOLVEISSUEINSEVERALSTEPS_H
