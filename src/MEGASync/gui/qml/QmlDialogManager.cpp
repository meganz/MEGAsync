#include "QmlDialogManager.h"

#include "AccountStatusController.h"
#include "AppState.h"
#include "DialogOpener.h"
#include "GuestContent.h"
#include "GuestQmlDialog.h"
#include "LoginController.h"
#include "Onboarding.h"
#include "OnboardingQmlDialog.h"
#include "QmlDialogWrapper.h"
#include "WhatsNewWindow.h"

std::shared_ptr<QmlDialogManager> QmlDialogManager::instance()
{
    static std::shared_ptr<QmlDialogManager> manager(new QmlDialogManager());
    return manager;
}

QmlDialogManager* QmlDialogManager::getQmlInstance(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance().get();
}

void QmlDialogManager::openGuestDialog()
{
    if(MegaSyncApp->finished())
    {
        return;
    }

    auto dialogWrapper = DialogOpener::findDialog<QmlDialogWrapper<GuestContent>>();
    if(dialogWrapper == nullptr)
    {
        QPointer<QmlDialogWrapper<GuestContent>> guest = new QmlDialogWrapper<GuestContent>();
        DialogOpener::addDialog(guest)->setIgnoreRaiseAllAction(true);
        dialogWrapper = DialogOpener::findDialog<QmlDialogWrapper<GuestContent>>();
    }

    DialogOpener::showDialog(dialogWrapper->getDialog());
}

bool QmlDialogManager::openOnboardingDialog(bool force)
{
    if (MegaSyncApp->finished() || (!force && Preferences::instance()->logged()))
    {
        return false;
    }

    if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
    {
        DialogOpener::showDialog(dialog->getDialog());
    }
    else
    {
        QPointer<QmlDialogWrapper<Onboarding>> onboarding = new QmlDialogWrapper<Onboarding>();
        DialogOpener::showDialog(onboarding)->setIgnoreCloseAllAction(true);
    }
    return true;
}

bool QmlDialogManager::raiseGuestDialog()
{
    bool raisedGuestDialog = false;
    if (MegaSyncApp->getAccountStatusController()->isAccountBlocked() ||
        MegaSyncApp->getLoginController()->getState() != LoginController::FETCH_NODES_FINISHED ||
        AppState::instance()->getAppState() != AppState::NOMINAL)
    {
        openGuestDialog();
        raisedGuestDialog = true;
    }
    return raisedGuestDialog;
}

void QmlDialogManager::raiseOnboardingDialog()
{
    if(MegaSyncApp->getAccountStatusController()->isAccountBlocked()
        || (MegaSyncApp->getLoginController()
                && MegaSyncApp->getLoginController()->getState() != LoginController::FETCH_NODES_FINISHED))
    {
        if (Preferences::instance()->getSession().isEmpty())
        {
            openOnboardingDialog();
        }
        else if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
        {
            DialogOpener::showDialog(dialog->getDialog());
            dialog->getDialog()->raise();
        }
    }
}

void QmlDialogManager::raiseOrHideInfoGuestDialog(QTimer* dialogTimer, int msec)
{
    if(!dialogTimer || msec < 0)
    {
        return;
    }

    auto guestDialogWrapper = DialogOpener::findDialog<QmlDialogWrapper<GuestContent>>();
    if(guestDialogWrapper == nullptr) //dialog still not built
    {
        dialogTimer->start(msec);
        return;
    }

    auto dialog = dynamic_cast<GuestQmlDialog*>(guestDialogWrapper->getDialog()->window());
    if(dialog && dialog->isHiddenForLongTime())
    {
        dialogTimer->start(msec);
    }
}

void QmlDialogManager::forceCloseOnboardingDialog()
{
    if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
    {
        static_cast<OnboardingQmlDialog*>(dialog->getDialog()->window())->forceClose();
    }
}

bool QmlDialogManager::openWhatsNewDialog()
{
    if(MegaSyncApp->finished())
    {
        return false;
    }

    if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<WhatsNewWindow>>())
    {
        DialogOpener::showDialog(dialog->getDialog());
        dialog->getDialog()->raise();
    }
    else
    {
        QPointer<QmlDialogWrapper<WhatsNewWindow>> whatsNew = new QmlDialogWrapper<WhatsNewWindow>();
        DialogOpener::showDialog(whatsNew);
        whatsNew->raise();
    }
    return true;
}
