#ifndef AUTOREFRESHSTALLEDISSUESBYCONDITION_H
#define AUTOREFRESHSTALLEDISSUESBYCONDITION_H

#include <MegaApplication.h>
#include <StalledIssue.h>

#include <QTimer>
#include <QDeadlineTimer>

class AutoRefreshByConditionBase : public QObject
{
    Q_OBJECT
public:
    AutoRefreshByConditionBase()
    {
        connect(&mRefreshTimer, &QTimer::timeout, this, &AutoRefreshByConditionBase::onRefreshTimerTimeout);
    }

    virtual ~AutoRefreshByConditionBase() = default;

    bool hasExpired(){return mDeadline.hasExpired();}
    bool needsAutoRefresh() const { return mNeedsAutoRefresh;}

    virtual void meetsConditionToRefresh(const StalledIssueVariant& issue) = 0;
    virtual void refresh() = 0;
    void remove()
    {
        Utilities::queueFunctionInAppThread([this]() { deleteLater(); });
    }

protected:
    QTimer mRefreshTimer;
    QDeadlineTimer mDeadline;
    bool mNeedsAutoRefresh = false;

protected slots:
    virtual void onRefreshTimerTimeout() = 0;
};


const int REQUEST_FREQUENCY = 5000; /*5 seconds*/
const int REQUEST_THRESHOLD = 15000; /*15 seconds*/

template <class ISSUE_TYPE>
class AutoRefreshByCondition : public AutoRefreshByConditionBase
{
public:
    AutoRefreshByCondition()
    {
        mRefreshTimer.setInterval(REQUEST_FREQUENCY);
    }

    ~AutoRefreshByCondition()
    {
        ISSUE_TYPE::automaticRefreshFinished();
    }

    void meetsConditionToRefresh(const StalledIssueVariant& issue) override
    {
        auto issueType = issue.convert<ISSUE_TYPE>();
        if(issueType)
        {
            mNeedsAutoRefresh = ISSUE_TYPE::needsAutomaticRefresh(issueType);
        }
    }

    void refresh()
    {
        Utilities::queueFunctionInAppThread(
            [this]()
            {
                mRefreshTimer.start();
                mDeadline = QDeadlineTimer(REQUEST_THRESHOLD);
                mNeedsAutoRefresh = false;
            });
    }

protected:
    void onRefreshTimerTimeout() override
    {
        MegaSyncApp->getMegaApi()->getMegaSyncStallList(nullptr);
    }
};


#endif // AUTOREFRESHSTALLEDISSUESBYCONDITION_H
