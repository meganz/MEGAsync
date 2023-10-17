#include "AccountStatusController.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"
#include "onboarding/GuestContent.h"
#include <QDebug>

AccountStatusController::AccountStatusController(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new mega::QTMegaListener(MegaSyncApp->getMegaApi(), this))
    , mPreferences(Preferences::instance())
    , mQueringWhyAmIBlocked(false)
    , mBlockedState(mega::MegaApi::ACCOUNT_NOT_BLOCKED)
    , mBlockedStateSet(false)
{
    mMegaApi->addListener(mDelegateListener);
    MegaSyncApp->qmlEngine()->rootContext()->setContextProperty(QString::fromUtf8("AccountStatusControllerAccess"), this);
}

void AccountStatusController::onEvent(mega::MegaApi*, mega::MegaEvent* event)
{
    if (event->getType() == mega::MegaEvent::EVENT_ACCOUNT_BLOCKED)
    {
        switch (event->getNumber())
        {
            case mega::MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL:
            case mega::MegaApi::ACCOUNT_BLOCKED_VERIFICATION_SMS:
            {
                mBlockedStateSet = true;
                int blockState = static_cast<int>(event->getNumber());
                if(mBlockedState == blockState)
                {
                    return;
                }

                mBlockedState = blockState;
                DialogOpener::closeAllDialogs();
                showVerifyAccountInfo();

                if (Preferences::instance()->logged())
                {
                    Preferences::instance()->setBlockedState(blockState);
                }

                emit blockedStateChanged(static_cast<int>(event->getNumber()));
                break;
            }
            case mega::MegaApi::ACCOUNT_BLOCKED_SUBUSER_DISABLED:
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = MegaSyncApp->getMEGAString();
                msgInfo.text = tr("Your account has been disabled by your administrator. Please contact your business account administrator for further details.");
                msgInfo.ignoreCloseAll = true;
                QMegaMessageBox::warning(msgInfo);
                break;
            }
            default:
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = MegaSyncApp->getMEGAString();
                msgInfo.text = QCoreApplication::translate("MegaError", event->getText());
                msgInfo.ignoreCloseAll = true;
                QMegaMessageBox::critical(msgInfo);
                break;
            }
        }
    }
}

void AccountStatusController::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(api)
    if(request->getType() == mega::MegaRequest::TYPE_WHY_AM_I_BLOCKED)
    {
        if (e->getErrorCode() == mega::MegaError::API_OK
            && request->getNumber() == mega::MegaApi::ACCOUNT_NOT_BLOCKED)
        {
            mBlockedStateSet = true;
            int blockState = mega::MegaApi::ACCOUNT_NOT_BLOCKED;
            if(mBlockedState == blockState)
            {
                return;
            }
            mBlockedState = blockState;
            if (Preferences::instance()->logged())
            {
                Preferences::instance()->setBlockedState(blockState);
            }
            MegaSyncApp->requestUserData(); // querying some user attributes might have been rejected: we query them again

            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("no longer blocked").toUtf8().constData());

            //in any case we reflect the change in the InfoDialog
            emit blockedStateChanged(blockState);

            if(auto lockDialog = DialogOpener::findDialog<VerifyLockMessage>())
            {
                lockDialog->close();
            }
            if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<GuestContent>>())
            {
                dialog->close();
            }
            MegaSyncApp->showInfoDialog();
        }

        MegaSyncApp->updateTrayIconMenu();
        mQueringWhyAmIBlocked = false;
    }
}

void AccountStatusController::whyAmIBlocked(bool force)
{
    if((!mQueringWhyAmIBlocked && isAccountBlocked()) || force)
    {
        mQueringWhyAmIBlocked = true;
        mMegaApi->whyAmIBlocked();
    }
}

bool AccountStatusController::isAccountBlocked() const
{
    if(mMegaApi->isLoggedIn())
    {
        return (mBlockedState != mega::MegaApi::ACCOUNT_NOT_BLOCKED);
    }
    return false;
}

int AccountStatusController::getBlockedState() const
{
    return mBlockedState;
}

void AccountStatusController::loggedIn()
{
    auto cachedBlockedState = mPreferences->getBlockedState();
    if (mBlockedStateSet && cachedBlockedState != mBlockedState) // blockstate received and needs to be updated in cache
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("cached blocked states %1 differs from applied blockedStatus %2. Overriding cache")
                                                   .arg(cachedBlockedState).arg(mBlockedState).toUtf8().constData());
        mPreferences->setBlockedState(mBlockedState);
    }
    else if (!mBlockedStateSet && cachedBlockedState != -2 && cachedBlockedState) //block state not received in this execution, and cached says we were blocked last time
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("cached blocked states %1 reports blocked, and no block state has been received before, lets query the block status")
                                                   .arg(cachedBlockedState).toUtf8().constData());

        whyAmIBlocked(true);// lets query again, to trigger transition and restoreSyncs
    }
}

void AccountStatusController::reset()
{
    mBlockedState = mega::MegaApi::ACCOUNT_NOT_BLOCKED;
    mQueringWhyAmIBlocked = false;
    mBlockedStateSet = false;
}

void AccountStatusController::showVerifyAccountInfo()
{
    QPointer<VerifyLockMessage> verifyEmail = new VerifyLockMessage(mBlockedState);
    connect(verifyEmail.data(), SIGNAL(logout()), MegaSyncApp, SLOT(unlink()));

    DialogOpener::showDialog(verifyEmail);
}

