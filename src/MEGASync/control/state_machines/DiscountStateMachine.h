#pragma once

#include "DiscountPolicy.h"

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QStateMachine>
#include <QTimer>

class TimedState: public QState
{
    Q_OBJECT

public:
    explicit TimedState(QState* parent = nullptr);

    explicit TimedState(QState::ChildMode childMode, QState* parent = nullptr);

    // Set duration applied on next entry
    void setAssignedDurationMs(long long ms);
    long long getDurationMs() const;

    QSignalTransition* addTimeoutTransition(QState* target);

signals:
    void timeoutTransition();

public slots:
    void setNextStep();

protected:
    void onEntry(QEvent* event) override;
    void onExit(QEvent* event) override;

private:
    QTimer* mTimer;
    long long mDuration;
    long long mRemainingDuration;
};

class DiscountStateMachine: public QObject
{
    Q_OBJECT

public:
    explicit DiscountStateMachine(DiscountPolicy* policy, QObject* parent = nullptr);

    void start();
    bool isInCooldownState() const;
    bool showTrayIconAnimation() const;

signals:
    void onboardingStarted();
    void onboardingFinished();
    void meaningfulInteraction();
    void enterOverquota();
    void discountDismissed();
    void discountAccepted();
    void cooldown();
    void campaignActive();
    void updateDiscountCampaignSignaling();
    void requestShowDialog();
    void onboardingLostFocus();
    void onboardingGainedFocus();
    void noBlockingWindow();
    void userActive();
    void requestUserDiscounts(bool force);

private:
    void build();
    void logState(QState* state);
    long long computeWaitingStateTimer();

    QElapsedTimer mElapsedTimeSinceAppStart;
    DiscountPolicy* mPolicy;
    QStateMachine mStateMachine;

    // States
    QState* mCampaignInactive;
    QState* mIdle;
    QState* mInactiveOnboarding;
    TimedState* mCampaignActive;
    QState* mActiveOnboarding;
    TimedState* mWaiting;
    QState* mWaitingForMeaningfulInteraction;
    QState* mWaitingForOverquota;
    TimedState* mShowable;
    QState* mWaitingForNoBlocking;
    QState* mWaitingForUserActive;
    QState* mShowing;
    QState* mShown;
    TimedState* mDealGrabbed;
    TimedState* mCooldown;
};
