#include "DiscountStateMachine.h"

#include "DialogOpener.h"
#include "DiscountPolicy.h"
#include "Onboarding.h"
#include "Platform.h"
#include "QmlDialogManager.h"
#include "QmlDialogWrapper.h"

#include <QAbstractTransition>
#include <QDebug>
#include <QState>
#include <QTimer>

#include <algorithm>
#include <functional>

constexpr long long TIMER_DISABLED = -1;
constexpr long long REQUEST_DISCOUNT_AFTER_DEAL_GRABBED_MS = 1000 * 60 * 15; // 15 minutes
constexpr long long SHOWABLE_TIMER_MS = 1000 * 5; // 5 seconds
constexpr long long MEANINGFUL_INTERACTION_DELAY_MS = 300;

// Helpers

// Sets targetTimedState duration for the *next entry* when the transition fires.
// durationProvider returns the duration in ms (TIMER_DISABLED [-1] disables).
inline void setTargetStateTimerDurationOnTransition(QAbstractTransition* transition,
                                                    std::function<long long()> durationProvider)
{
    Q_ASSERT(transition);
    TimedState* target = qobject_cast<TimedState*>(transition->targetState());
    if (target)
    {
        QObject::connect(transition,
                         &QAbstractTransition::triggered,
                         target,
                         [target, durationProvider]
                         {
                             const long long ms =
                                 durationProvider ? durationProvider() : TIMER_DISABLED;
                             target->setAssignedDurationMs(ms);
                         });
    }
}

// TimedState class

TimedState::TimedState(QState* parent):
    TimedState(ChildMode::ExclusiveStates, parent)
{}

TimedState::TimedState(QState::ChildMode childMode, QState* parent):
    QState(childMode, parent),
    mTimer(new QTimer(this)),
    mDuration(TIMER_DISABLED),
    mRemainingDuration(0)
{
    mTimer->setSingleShot(true);
    mTimer->setTimerType(Qt::VeryCoarseTimer);

    // Stop timer on exit as a safety net
    connect(this, &QState::exited, mTimer, &QTimer::stop);
    connect(mTimer, &QTimer::timeout, this, &TimedState::setNextStep);
}

// Set duration applied on next entry
void TimedState::setAssignedDurationMs(long long ms)
{
    mDuration = ms;
}

long long TimedState::getDurationMs() const
{
    return mDuration;
}

QSignalTransition* TimedState::addTimeoutTransition(QState* target)
{
    return addTransition(this, &TimedState::timeoutTransition, target);
}

void TimedState::onEntry(QEvent* event)
{
    Q_UNUSED(event);

    mTimer->stop(); // defensive reset
    mRemainingDuration = mDuration;

    if (mDuration >= 0)
    {
        setNextStep();
    }
}

void TimedState::onExit(QEvent* event)
{
    Q_UNUSED(event);
    mTimer->stop();
    mRemainingDuration = 0;
}

void TimedState::setNextStep()
{
    if (mRemainingDuration == 0)
    {
        emit timeoutTransition();
    }
    else
    {
        static const long long maxInt = std::numeric_limits<int>::max();
        const auto step = std::min(maxInt, mRemainingDuration);
        mRemainingDuration -= step;
        mTimer->start(int(step)); // Safe conversion from long long to int
    }
}

// DiscountStateMachine class

DiscountStateMachine::DiscountStateMachine(DiscountPolicy* policy, QObject* parent):
    QObject(parent),
    mPolicy(policy),
    mOnBoardingDialog(nullptr)
{
    connect(this,
            &DiscountStateMachine::onboardingStarted,
            this,
            &DiscountStateMachine::onOnboardingStarted,
            Qt::UniqueConnection);

    // Connect to onboarding start
    connect(QmlDialogManager::instance().get(),
            &QmlDialogManager::openOnboardingDialogSignal,
            this,
            &DiscountStateMachine::onboardingStarted,
            Qt::UniqueConnection);
    build();
}

void DiscountStateMachine::start()
{
    mStateMachine.start();
}

bool DiscountStateMachine::isInCooldownState() const
{
    return mCooldown->active();
}

bool DiscountStateMachine::showTrayIconAnimation() const
{
    return mStateMachine.configuration().contains(mWaiting) ||
           mStateMachine.configuration().contains(mShowable);
}

void DiscountStateMachine::onDiscountButtonClicked()
{
    emit externalDiscountDialogRequest();
}

void DiscountStateMachine::onMeaningfulInteraction()
{
    QTimer::singleShot(MEANINGFUL_INTERACTION_DELAY_MS,
                       this,
                       &DiscountStateMachine::meaningfulInteraction);
}

void DiscountStateMachine::build()
{
    // States --------------------------------------------------------------------------------------
    QState* rootState = new QState(&mStateMachine);

    // - Inactive states
    mCampaignInactive = new QState(rootState);
    mCampaignInactive->setObjectName(QLatin1String("Campaign inactive"));
    // ++ Idle
    mIdle = new QState(mCampaignInactive);
    mIdle->setObjectName(QLatin1String("Idle"));

    // ++ InactiveOnboarding
    mInactiveOnboarding = new QState(mCampaignInactive);
    mInactiveOnboarding->setObjectName(QLatin1String("Campaign inactive onboarding"));

    // - Active states
    mCampaignActive = new TimedState(rootState);
    mCampaignActive->setObjectName(QLatin1String("Campaign active"));

    // ++ ActiveOnboarding
    mActiveOnboarding = new QState(mCampaignActive);
    mActiveOnboarding->setObjectName(QLatin1String("Campaign active onboarding"));

    // ++ Waiting
    mWaiting = new TimedState(QState::ParallelStates, mCampaignActive);
    mWaiting->setObjectName(QLatin1String("Waiting"));

    // *** WaitingForMeaningfulInteraction
    mWaitingForMeaningfulInteraction = new QState(mWaiting);
    mWaitingForMeaningfulInteraction->setObjectName(
        QLatin1String("Waiting for meaningful interaction"));

    // *** WaitingForOverquota
    mWaitingForOverquota = new QState(mWaiting);
    mWaitingForOverquota->setObjectName(QLatin1String("Waiting for overquota"));

    // ++ Showable
    mShowable = new TimedState(QState::ParallelStates, mCampaignActive);
    mShowable->setObjectName(QLatin1String("Showable"));
    mShowable->setAssignedDurationMs(SHOWABLE_TIMER_MS);

    // *** WaitingForNoBlocking
    auto* region1 = new QState(mShowable);
    mWaitingForNoBlocking = new QState(region1);
    mWaitingForNoBlocking->setObjectName(QLatin1String("Waiting for no blocking window"));
    region1->setInitialState(mWaitingForNoBlocking);
    auto* noBlockingFinal = new QFinalState(region1);

    // *** WaitingForUserActive
    auto* region2 = new QState(mShowable);
    mWaitingForUserActive = new QState(region2);
    mWaitingForUserActive->setObjectName(QLatin1String("Waiting for user active"));
    region2->setInitialState(mWaitingForUserActive);
    auto* userActiveFinal = new QFinalState(region2);

    // ++ Showing
    mShowing = new QState(mCampaignActive);
    mShowing->setObjectName(QLatin1String("Showing"));

    // ++ Shown
    mShown = new QState(mCampaignActive);
    mShown->setObjectName(QLatin1String("Shown"));

    // *** Cooldown
    mCooldown = new TimedState(mShown);
    mCooldown->setObjectName(QLatin1String("Cooldown"));

    // *** DealGrabbed
    mDealGrabbed = new TimedState(mShown);
    mDealGrabbed->setObjectName(QLatin1String("Deal grabbed"));

    // Initial state -------------------------------------------------------------------------------
    mCampaignActive->setInitialState(mWaiting);
    mCampaignInactive->setInitialState(mIdle);
    rootState->setInitialState(mCampaignInactive);
    mStateMachine.setInitialState(rootState);
    mElapsedTimeSinceAppStart.start();

    // Transitions ---------------------------------------------------------------------------------

    // Campaign Inactive state
    mCampaignInactive->addTransition(this, &DiscountStateMachine::cooldown, mCooldown);

    // Idle
    connect(mIdle,
            &QState::entered,
            this,
            [this]
            {
                // If the onboarding is open, we want to go to the right state
                if (isOnboardingOpen())
                {
                    emit onboardingStarted();
                }
            });
    mIdle->addTransition(this, &DiscountStateMachine::onboardingStarted, mInactiveOnboarding);
    QAbstractTransition* transition =
        mIdle->addTransition(this, &DiscountStateMachine::campaignActive, mWaiting);

    // Set the time when the start delay waiting time will expire
    setTargetStateTimerDurationOnTransition(transition,
                                            [this]
                                            {
                                                return computeWaitingStateTimer();
                                            });

    // Campaign Inactive Onboarding
    mInactiveOnboarding->addTransition(this, &DiscountStateMachine::onboardingFinished, mIdle);
    mInactiveOnboarding->addTransition(this,
                                       &DiscountStateMachine::campaignActive,
                                       mActiveOnboarding);

    // Campaign Active states
    mCampaignActive->addTransition(mPolicy,
                                   &DiscountPolicy::campaignDeactivated,
                                   mCampaignInactive);

    // End of campaign timeout. Deactivate policy.
    transition = mCampaignActive->addTimeoutTransition(mCampaignInactive);
    QObject::connect(transition,
                     &QAbstractTransition::triggered,
                     this,
                     [this]
                     {
                         mPolicy->deactivateCampaign();
                     });

    connect(mPolicy,
            &DiscountPolicy::campaignActivated,
            this,
            [this]
            {
                // Get the campaign duration to configure the Active State timeout
                const auto msecsToExpiry =
                    QDateTime::currentDateTimeUtc().msecsTo(mPolicy->getExpiryDateUtc());
                // Set to 0 to immediately exit if no time left
                mCampaignActive->setAssignedDurationMs(std::max(msecsToExpiry, 0LL));

                // Check if we are in the cooldown period or not
                auto coolDownEnd = mPolicy->getDialogLastShownDateUtc().addMSecs(
                    Preferences::TARGETED_DISCOUNT_COOLDOWN_MS);
                auto nowUtc = QDateTime::currentDateTimeUtc();
                if (nowUtc > coolDownEnd)
                {
                    emit campaignActive();
                }
                else
                {
                    mCooldown->setAssignedDurationMs(nowUtc.msecsTo(coolDownEnd));
                    emit cooldown();
                }
            });

    // Waiting state
    mWaiting->addTransition(this, &DiscountStateMachine::onboardingStarted, mActiveOnboarding);
    mWaiting->addTimeoutTransition(mShowable);

    mWaiting->addTransition(this, &DiscountStateMachine::onboardingGainedFocus, mActiveOnboarding);
    mWaitingForMeaningfulInteraction->addTransition(this,
                                                    &DiscountStateMachine::meaningfulInteraction,
                                                    mShowable);
    mWaitingForOverquota->addTransition(this, &DiscountStateMachine::enterOverquota, mShowable);
    mWaiting->addTransition(this, &DiscountStateMachine::externalDiscountDialogRequest, mShowing);

    // Campaign Active Onboarding
    mActiveOnboarding->addTransition(this, &DiscountStateMachine::onboardingFinished, mShowable);
    mActiveOnboarding->addTransition(mPolicy,
                                     &DiscountPolicy::campaignDeactivated,
                                     mInactiveOnboarding);
    transition = mActiveOnboarding->addTransition(this,
                                                  &DiscountStateMachine::onboardingLostFocus,
                                                  mWaiting);
    setTargetStateTimerDurationOnTransition(transition,
                                            []
                                            {
                                                return TIMER_DISABLED;
                                            });
    // Showable states
    mShowable->addTimeoutTransition(mShowable);
    mShowable->addTransition(this, &DiscountStateMachine::externalDiscountDialogRequest, mShowing);
    mShowable->addTransition(mShowable, &QState::finished, mShowing);

    mWaitingForNoBlocking->addTransition(this,
                                         &DiscountStateMachine::noBlockingWindow,
                                         noBlockingFinal);
    connect(mWaitingForNoBlocking,
            &QState::entered,
            this,
            [this]
            {
                if (!DialogOpener::isAnyDialogVisible())
                {
                    emit DiscountStateMachine::noBlockingWindow();
                }
            });

    mWaitingForUserActive->addTransition(this, &DiscountStateMachine::userActive, userActiveFinal);
    connect(mWaitingForUserActive,
            &QState::entered,
            this,
            [this]
            {
                if (Platform::getInstance()->isUserActive())
                {
                    emit DiscountStateMachine::userActive();
                }
            });

    // Showing state
    connect(mShowing, &QState::entered, this, &DiscountStateMachine::requestShowDialog);
    transition = mShowing->addTransition(this, &DiscountStateMachine::discountDismissed, mCooldown);
    setTargetStateTimerDurationOnTransition(transition,
                                            []
                                            {
                                                return Preferences::TARGETED_DISCOUNT_COOLDOWN_MS;
                                            });
    transition =
        mShowing->addTransition(this, &DiscountStateMachine::discountAccepted, mDealGrabbed);
    setTargetStateTimerDurationOnTransition(transition,
                                            []
                                            {
                                                return REQUEST_DISCOUNT_AFTER_DEAL_GRABBED_MS;
                                            });

    // Shown states
    mShown->addTransition(this, &DiscountStateMachine::externalDiscountDialogRequest, mShowing);

    // Cooldown
    transition = mCooldown->addTimeoutTransition(mWaiting);
    setTargetStateTimerDurationOnTransition(transition,
                                            [this]
                                            {
                                                return computeWaitingStateTimer();
                                            });

    // Grabbed deal
    // Here we want to wait for some time before requesting discounts again.
    transition = mDealGrabbed->addTimeoutTransition(mCooldown);
    setTargetStateTimerDurationOnTransition(transition,
                                            []
                                            {
                                                return Preferences::TARGETED_DISCOUNT_COOLDOWN_MS;
                                            });
    connect(transition,
            &QAbstractTransition::triggered,
            this,
            [this]
            {
                emit requestUserDiscounts(true);
            });

    // Update signaling on state changes -----------------------------------------------------------
    connect(mCampaignActive,
            &QState::exited,
            this,
            &DiscountStateMachine::updateDiscountCampaignSignaling);
    connect(mWaiting,
            &QState::entered,
            this,
            &DiscountStateMachine::updateDiscountCampaignSignaling);
    connect(mShowable,
            &QState::entered,
            this,
            &DiscountStateMachine::updateDiscountCampaignSignaling);
    connect(mShowing,
            &QState::entered,
            this,
            &DiscountStateMachine::updateDiscountCampaignSignaling);
    connect(mShown, &QState::entered, this, &DiscountStateMachine::updateDiscountCampaignSignaling);

    // Log state changes ---------------------------------------------------------------------------
    logState(mCampaignInactive);
    logState(mIdle);
    logState(mInactiveOnboarding);
    logState(mCampaignActive);
    logState(mActiveOnboarding);
    logState(mWaiting);
    logState(mWaitingForMeaningfulInteraction);
    logState(mWaitingForOverquota);
    logState(mShowable);
    logState(mWaitingForNoBlocking);
    logState(mWaitingForUserActive);
    logState(mShowing);
    logState(mShown);
    logState(mDealGrabbed);
    logState(mCooldown);
}

void DiscountStateMachine::logState(QState* state)
{
    if (auto timedState = qobject_cast<TimedState*>(state))
    {
        QObject::connect(state,
                         &QState::entered,
                         timedState,
                         [timedState]
                         {
                             mega::MegaApi::log(
                                 mega::MegaApi::LOG_LEVEL_DEBUG,
                                 QString::fromLatin1("DSM-ENTER %1 - TIMER: %2s")
                                     .arg(timedState->objectName(),
                                          QString::number(timedState->getDurationMs() / 1000))
                                     .toUtf8()
                                     .constData());
                         });
    }
    else
    {
        QObject::connect(state,
                         &QState::entered,
                         state,
                         [state]
                         {
                             mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,
                                                QString::fromLatin1("DSM-ENTER %1")
                                                    .arg(state->objectName())
                                                    .toUtf8()
                                                    .constData());
                         });
    }
    QObject::connect(
        state,
        &QState::exited,
        state,
        [state]
        {
            mega::MegaApi::log(
                mega::MegaApi::LOG_LEVEL_DEBUG,
                QString::fromLatin1("DSM-EXIT %1").arg(state->objectName()).toUtf8().constData());
        });
}

long long DiscountStateMachine::computeWaitingStateTimer()
{
    // Here we want to wait either for the start delay remaining time, or for the fallback time if
    // the start delay has been reached.
    auto waitingDuration =
        Preferences::TARGETED_DISCOUNT_STARTUP_DELAY_MS - mElapsedTimeSinceAppStart.elapsed();
    return waitingDuration > 0 ? waitingDuration :
                                 Preferences::TARGETED_DISCOUNT_WAITING_FALLBACK_MS;
}

bool DiscountStateMachine::isOnboardingOpen()
{
    return DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>().get() != nullptr;
}

void DiscountStateMachine::onOnboardingStarted()
{
    auto onboardingDialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>();
    if (onboardingDialog)
    {
        const auto newOnboardingDialog = qobject_cast<QObject*>(onboardingDialog->getDialog());
        if (newOnboardingDialog != mOnBoardingDialog)
        {
            mOnBoardingDialog = newOnboardingDialog;
            connect(mOnBoardingDialog,
                    &QObject::destroyed,
                    this,
                    &DiscountStateMachine::onboardingFinished,
                    Qt::UniqueConnection);
        }
    }
}
