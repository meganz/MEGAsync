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

template <class ISSUE_TYPE>
class AutoRefreshByCondition : public AutoRefreshByConditionBase
{
public:
    AutoRefreshByCondition(int refreshFrequency, int refreshDeadline)
        : mRefreshFrequency(refreshFrequency),
        mDeadlineValue(refreshDeadline)
    {
        mRefreshFrequency = refreshFrequency;
        mRefreshTimer.setInterval(mRefreshFrequency);

        mDeadlineValue = refreshDeadline;
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

    void setRefreshDeadline(const int& deadline)
    {
        mDeadlineValue = deadline;
    }

    void refresh()
    {
        Utilities::queueFunctionInAppThread(
            [this]()
            {
                qDebug() << "REFRESH TIMERS";
                mRefreshTimer.start();
                mDeadline = QDeadlineTimer(mDeadlineValue);
                mNeedsAutoRefresh = false;
            });
    }

protected:
    void onRefreshTimerTimeout() override
    {
        qDebug() << "ASK FOR LIST";
        MegaSyncApp->getMegaApi()->getMegaSyncStallList(nullptr);
    }

private:
    int mRefreshFrequency;
    int mDeadlineValue;
};


#endif // AUTOREFRESHSTALLEDISSUESBYCONDITION_H
