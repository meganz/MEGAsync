#include <MultiStepIssueSolver.h>

#include <StalledIssuesModel.h>

uint MultiStepIssueSolverBase::mSolversFixedInTheSameNotification = 0;
uint MultiStepIssueSolverBase::mSolversFailedInTheSameNotification = 0;
uint MultiStepIssueSolverBase::mSolversBeingFixedInTheSameNotification = 0;

MultiStepIssueSolverBase::MultiStepIssueSolverBase(std::shared_ptr<StalledIssue> issue)
    : mDeadline(std::make_unique<QTimer>(this))
    , mIssue(issue)
    , mFailed(false)
{
    mDeadline->setInterval(REQUEST_THRESHOLD);
    mDeadline->setSingleShot(true);

    //Auto destroy class when the timer finishes
    connect(mDeadline.get(), &QTimer::timeout, this, &MultiStepIssueSolverBase::onDeadLineFinished);

    //Not always needed
    sendStartNotification();

    mSolversBeingFixedInTheSameNotification++;
}

void MultiStepIssueSolverBase::start()
{
    mDeadline->start();
}

bool MultiStepIssueSolverBase::isActive() const
{
    return mDeadline->isActive();
}

void MultiStepIssueSolverBase::sendStartNotification()
{
    //Don´t send a new notification if a previous one was sent
    if(mSolversBeingFixedInTheSameNotification == 0)
    {
        DesktopNotifications::NotificationInfo startNotificationInfo;
        startNotificationInfo.title = tr("Solve issues");
        startNotificationInfo.message = StalledIssuesModel::processingIssuesString();
        MegaSyncApp->showInfoMessage(startNotificationInfo);
    }
}

void MultiStepIssueSolverBase::sendFinishNotification()
{
    DesktopNotifications::NotificationInfo finishNotificationInfo;
    finishNotificationInfo.title = tr("Solve issues");

    if(mSolversFailedInTheSameNotification > 0)
    {
        finishNotificationInfo.message = tr(
            "Some issues couldn't be resolved.[BR]Check the Issues screen for resolution options, "
            "and try to resolve the issues again.").replace(QString::fromUtf8("[BR]"), QString::fromUtf8("\n"));
        //Add action to run the widget to show the errors (future feature)
    }
    else
    {
        StalledIssuesCreator::IssuesCount count;
        count.issuesFixed = mSolversFixedInTheSameNotification;
        finishNotificationInfo.message = StalledIssuesModel::issuesFixedString(count);
    }

    MegaSyncApp->showInfoMessage(finishNotificationInfo);

    mSolversFailedInTheSameNotification = 0;
    mSolversFixedInTheSameNotification = 0;
}

void MultiStepIssueSolverBase::setFailed()
{
    mFailed = true;
}

void MultiStepIssueSolverBase::setFinished()
{
    mFailed ? mSolversFailedInTheSameNotification++ : mSolversFixedInTheSameNotification++;
    mSolversBeingFixedInTheSameNotification--;
}
