#include "AccountStatusController.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"
#include "onboarding/Onboarding.h"
#include <QDebug>

AccountStatusController::AccountStatusController(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new mega::QTMegaListener(MegaSyncApp->getMegaApi(), this))
    , mPreferences(Preferences::instance())
    , mQueringWhyAmIBlocked(false)
{
    mBlockedAccount = new BlockedAccountLinux(parent);
    mMegaApi->addListener(mDelegateListener);
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
            mBlockedAccount->setAccountBlocked(event->getNumber());

            if (!mWhyamiblockedPeriodicPetition) //Do not force show on periodic whyamiblocked call
            {
                //show QML guestwidget in account blocked pane
                DialogOpener::closeAllDialogs();

                showVerifyAccountInfo();
                whyamiblockedPeriodicPetition = false;
            }
            break;
        }
        case mega::MegaApi::ACCOUNT_BLOCKED_SUBUSER_DISABLED:
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = getMEGAString();
            msgInfo.text = tr("Your account has been disabled by your administrator. Please contact your business account administrator for further details.");
            msgInfo.ignoreCloseAll = true;
            QMegaMessageBox::warning(msgInfo);
            break;
        }
        default:
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = getMEGAString();
            msgInfo.text = QCoreApplication::translate("MegaError", event->getText());
            msgInfo.ignoreCloseAll = true;
            QMegaMessageBox::critical(msgInfo);
            break;
        }
        }

        if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
        {
            emit dialog->getDialog()->wrapper()->accountBlocked(event->getNumber());
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
            mBlockedAccount->setAccountBlocked(mega::MegaApi::ACCOUNT_NOT_BLOCKED);
            MegaSyncApp->requestUserData(); // querying some user attributes might have been rejected: we query them again

            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("no longer blocked").toUtf8().constData());

            //in any case we reflect the change in the InfoDialog
        }

        MegaSyncApp->updateTrayIconMenu();
        mQueringWhyAmIBlocked = false;
    }
}

void AccountStatusController::whyAmIblocked()
{
    if(!mQueringWhyAmIBlocked)
    {
        mMegaApi->whyAmIBlocked();
        mQueringWhyAmIBlocked = true;
    }
}


BlockedAccount::BlockedAccount(QObject *parent)
    : QObject(parent)
    , mBlockState(mega::MegaApi::ACCOUNT_NOT_BLOCKED)
{

}

void BlockedAccount::setAccountBlocked(int blockState)
{
    mBlockState = blockState;
    if (Preferences::instance()->logged())
    {
        Preferences::instance()->setBlockedState(blockState);
    }
}

BlockedAccountLinux::BlockedAccountLinux(QObject *parent)
    : BlockedAccount(parent)
{
    mTimer = new QTimer(this);
    mTimer->setInterval(Preferences::STATE_REFRESH_INTERVAL_MS / 10);
    connect(mTimer, &QTimer::timeout, this, &BlockedAccountLinux::timeout);
}

void BlockedAccountLinux::setAccountBlocked(int blockState)
{
    BlockedAccount::setAccountBlocked(blockState);
    if(blockState > mega::MegaApi::ACCOUNT_NOT_BLOCKED)
    {
        mTimer->start();
    }
}

void BlockedAccountLinux::timeout()
{

}
