#include "LoginController.h"
#include "MegaApplication.h"
#include "Preferences/Preferences.h"
#include "ConnectivityChecker.h"
#include "Platform.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"
#include "onboarding/Onboarding.h"
#include "mega/types.h"

#include <QQmlContext>

LoginController::LoginController(QObject *parent)
    : QObject{parent}
      , mMegaApi(MegaSyncApp->getMegaApi())
      , mPreferences(Preferences::instance())
      , mDelegateListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
      , mGlobalListener(mega::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
      , mFetchingNodes(false)
      , mEmailConfirmed(false)
      , mConfirmationResumed(false)
      , mFirstTime(false)
{
    mMegaApi->addRequestListener(mDelegateListener.get());
    mMegaApi->addGlobalListener(mGlobalListener.get());
    mConnectivityTimer = new QTimer(this);
    mConnectivityTimer->setSingleShot(true);
    mConnectivityTimer->setInterval(static_cast<int>(Preferences::MAX_LOGIN_TIME_MS));
    connect(mConnectivityTimer, &QTimer::timeout, this, &LoginController::runConnectivityCheck);

    EphemeralCredentials credentials = mPreferences->getEphemeralCredentials();
    if(credentials.sessionId.size() > 0)
    {
        mMegaApi->resumeCreateAccount(credentials.sessionId.toUtf8().constData());
    }

    MegaSyncApp->qmlEngine()->rootContext()->setContextProperty(QString::fromUtf8("LoginControllerAccess"), this);
}

LoginController::~LoginController()
{
    MegaSyncApp->qmlEngine()->rootContext()->setContextProperty(QString::fromUtf8("LoginControllerAccess"), nullptr);
}

void LoginController::login(const QString &email, const QString &password)
{
    mMegaApi->login(email.toUtf8().constData(), password.toUtf8().constData());
}

void LoginController::createAccount(const QString &email, const QString &password,
                                const QString &name, const QString &lastName)
{
    mConfirmationResumed = false;
    mMegaApi->createAccount(email.toUtf8().constData(), password.toUtf8().constData(),
                             name.toUtf8().constData(), lastName.toUtf8().constData());
}

void LoginController::changeRegistrationEmail(const QString &email)
{
    QString fullName = mName + QLatin1Char(' ') + mLastName;
    mMegaApi->resendSignupLink(email.toUtf8().constData(), fullName.toUtf8().constData());
}

void LoginController::login2FA(const QString &pin)
{
    mMegaApi->multiFactorAuthLogin(mEmail.toUtf8().constData(), mPassword.toUtf8().constData(), pin.toUtf8().constData());
}

void LoginController::cancelLogin2FA()
{
    emit login2FACancelled();
}

QString LoginController::getEmail() const
{
    return mEmail;
}

QString LoginController::getPassword() const
{
    return mPassword;
}

bool LoginController::getIsEmailConfirmed() const
{
    return mEmailConfirmed;
}

void LoginController::cancelLogin() const
{
    mMegaApi->localLogout();
}

void LoginController::cancelCreateAccount() const
{
    mMegaApi->cancelCreateAccount();
}

void LoginController::guestWindowLoginClicked()
{
    emit goToLoginPage();
}

void LoginController::guestWindowSignupClicked()
{
    emit goToSignupPage();
}

bool LoginController::isAccountConfirmationResumed() const
{
    return mConfirmationResumed;
}

void LoginController::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(api)
    switch(request->getType())
    {
        case mega::MegaRequest::TYPE_LOGIN:
        {
            mConnectivityTimer->stop();
            MegaSyncApp->initLocalServer();
            if(e->getErrorCode() == mega::MegaError::API_OK)
            {
                mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
            }
            onLogin(request, e);
            break;
        }
        case mega::MegaRequest::TYPE_LOGOUT:
        {
            onLogout(request, e);
            break;
        }
        case mega::MegaRequest::TYPE_CREATE_ACCOUNT:
        {
            if(request->getParamType() == mega::MegaApi::RESUME_ACCOUNT)
            {
                onAccountCreationResume(request, e);
            }
            else if(request->getParamType() == mega::MegaApi::CANCEL_ACCOUNT)
            {
                onAccountCreationCancel(request, e);
            }
            else
            {
                onAccountCreation(request, e);
            }
            break;
        }
        case mega::MegaRequest::TYPE_SEND_SIGNUP_LINK:
        {
            onEmailChanged(request, e);
            break;
        }
        case mega::MegaRequest::TYPE_FETCH_NODES:
        {
            onFetchNodes(request, e);
            break;
        }
        case mega::MegaRequest::TYPE_WHY_AM_I_BLOCKED:
        {
            onWhyAmIBlocked(request, e);
            break;
        }
    }
}

void LoginController::onRequestUpdate(mega::MegaApi *api, mega::MegaRequest *request)
{
    Q_UNUSED(api)

    if (request->getType() == mega::MegaRequest::TYPE_FETCH_NODES)
    {
        if (request->getTotalBytes() > 0)
        {
            double total = static_cast<double>(request->getTotalBytes());
            double part = static_cast<double>(request->getTransferredBytes());
            double progress = part/total;
            if(progress > 0.15)
            {
                emit fetchingNodesProgress(progress);
            }
        }
    }
}

void LoginController::onRequestStart(mega::MegaApi *api, mega::MegaRequest *request)
{
    Q_UNUSED(api)
    switch(request->getType())
    {
    case mega::MegaRequest::TYPE_LOGIN:
    {
        mConnectivityTimer->start();
        emit loginStarted();
        break;
    }
    case mega::MegaRequest::TYPE_CREATE_ACCOUNT:
    {
        emit registerStarted();
        break;
    }
    case mega::MegaRequest::TYPE_FETCH_NODES:
    {
        emit fetchingNodesProgress(0.15);
        break;
    }
    }
}

void LoginController::onEvent(mega::MegaApi *, mega::MegaEvent *event)
{
    if(event->getType() == mega::MegaEvent::EVENT_CONFIRM_USER_EMAIL)
    {
        emailConfirmation(QString::fromLatin1(event->getText()));
    }
}

void LoginController::emailConfirmation(const QString& email)
{
    if(mEmail == email)
    {
        mPreferences->removeEphemeralCredentials();
        mEmailConfirmed = true;
        emit emailConfirmed();
    }
}

void LoginController::onLogin(mega::MegaRequest *request, mega::MegaError *e)
{
    QString errorMsg;
    if(e->getErrorCode() == mega::MegaError::API_OK)
    {
        std::unique_ptr<char []> session(mMegaApi->dumpSession());
        if (session)
        {
            mPreferences->setSession(QString::fromUtf8(session.get()));
        }
        bool logged = mPreferences->logged();
        mFirstTime = !logged && !mPreferences->hasEmail(QString::fromUtf8(request->getEmail()));
        // We will proceed with a new login

        auto email = QString::fromUtf8(request->getEmail());
        mPreferences->setEmailAndGeneralSettings(email);

        fetchNodes(email);
        if (!mPreferences->hasLoggedIn())
        {
            mPreferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
        }
    }
    else
    {
        mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_FAILED);
        switch(e->getErrorCode())
        {
            case mega::MegaError::API_EINCOMPLETE:
            {
                errorMsg = tr("Please check your e-mail and click the link to confirm your account.");
                break;
            }
            case mega::MegaError::API_ETOOMANY:
            {
                errorMsg = tr("You have attempted to log in too many times.[BR]Please wait until %1 and try again.")
                               .replace(QString::fromUtf8("[BR]"), QString::fromUtf8("\n"))
                               .arg(QTime::currentTime().addSecs(3600).toString(QString::fromUtf8("hh:mm")));
                break;
            }
            case mega::MegaError::API_EMFAREQUIRED:
            {
                mPassword = QString::fromUtf8(request->getPassword());
                mEmail = QString::fromUtf8(request->getEmail());
                break;
            }
            default:
            {
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
                break;
            }
        }
    }

    MegaSyncApp->onGlobalSyncStateChanged(mMegaApi);
    emit loginFinished(e->getErrorCode(), errorMsg);
}

void LoginController::onFetchNodesSuccess()
{
    SyncInfo::instance()->rewriteSyncSettings(); //write sync settings into user's preferences

    MegaSyncApp->loggedIn(true);
}

void LoginController::onAccountCreation(mega::MegaRequest *request, mega::MegaError *e)
{
    if(e->getErrorCode() == mega::MegaError::API_OK)
    {
        mEmail = QString::fromUtf8(request->getEmail());
        emit emailChanged();
        mName = QString::fromUtf8(request->getName());
        mLastName = QString::fromUtf8(request->getText());
        EphemeralCredentials credentials;
        credentials.email = mEmail;
        credentials.sessionId = QString::fromUtf8(request->getSessionKey());
        mPreferences->setEphemeralCredentials(credentials);
    }
    emit registerFinished(e->getErrorCode() == mega::MegaError::API_OK);
}

void LoginController::onAccountCreationResume(mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(request)
    if(e->getErrorCode() == mega::MegaError::API_OK)
    {
        EphemeralCredentials credentials = mPreferences->getEphemeralCredentials();
        mEmail = credentials.email;
        emit emailChanged();
        emit accountCreationResumed();
        mConfirmationResumed = true;
    }
}

void LoginController::onEmailChanged(mega::MegaRequest *request, mega::MegaError *e)
{
    mEmail = QString::fromUtf8(request->getEmail());
    emit emailChanged();
    emit changeRegistrationEmailFinished(e->getErrorCode() == mega::MegaError::API_OK);
}

void LoginController::onFetchNodes(mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(request)
    if (e->getErrorCode() == mega::MegaError::API_OK)
    {
        //Update/set root node
        MegaSyncApp->getRootNode(true); //TODO: move this to thread pool, notice that mRootNode is used below
        MegaSyncApp->getVaultNode(true);
        MegaSyncApp->getRubbishNode(true);

        mPreferences->setAccountStateInGeneral(Preferences::STATE_FETCHNODES_OK);
        mPreferences->setNeedsFetchNodesInGeneral(false);

        onFetchNodesSuccess();
        emit fetchingNodesFinished(mFirstTime);
    }
    else
    {
        mPreferences->setAccountStateInGeneral(Preferences::STATE_FETCHNODES_FAILED);
        mPreferences->setNeedsFetchNodesInGeneral(true);
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error fetching nodes: %1")
                                                                .arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
    }
}

void LoginController::onWhyAmIBlocked(mega::MegaRequest *request, mega::MegaError *e)
{
    if (e->getErrorCode() == mega::MegaError::API_OK
        && request->getNumber() == mega::MegaApi::ACCOUNT_NOT_BLOCKED)
    {
        // if we received a block before nodes were fetch,
        // we want to try again now that we are no longer blocked
        if (!mFetchingNodes && !MegaSyncApp->getRootNode())
        {
            fetchNodes();
            //show fetchnodes page in new guestwidget
        }
    }
}

void LoginController::onAccountCreationCancel(mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(request)
    Q_UNUSED(e)
    mPreferences->removeEphemeralCredentials();
    mEmail.clear();
    mPassword.clear();
    mName.clear();
    mLastName.clear();
    emit emailChanged();
    emit accountCreateCancelled();
}

void LoginController::onLogout(mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(e)
    Q_UNUSED(request)

    emit logout();
    mFetchingNodes = false;
}

void LoginController::fetchNodes(const QString& email)
{
    assert(!mFetchingNodes);
    mFetchingNodes = true;

           // We need to load exclusions and migrate sync configurations from MEGAsync held cache, to SDK's
           // prior fetching nodes (when the SDK will resume syncing)

           // If we are loging into a new session of an account previously used in MEGAsync,
           // we will use the previous configurations stored in that user mPreferences
           // However, there is a case in which we are not able to do so at this point:
           // we don't know the user email.
           // That should only happen when trying to resume a session (using the session id stored in general mPreferences)
           // that didn't complete a fetch nodes (i.e. does not have mPreferences logged).
           // that can happen for blocked accounts.
           // Fortunately, the SDK can help us get the email of the session
    bool needFindingOutEmail = !mPreferences->logged() && email.isEmpty();

    auto loadMigrateAndFetchNodes = [this](const QString &email)
    {
        if (!mPreferences->logged() && email.isEmpty()) // I still couldn't get the the email: won't be able to access user settings
        {
            mMegaApi->fetchNodes();
        }
        else
        {
            loadSyncExclusionRules(email);
            migrateSyncConfToSdk(email); // this will produce the fetch nodes once done
        }
    };

    if (!needFindingOutEmail)
    {
        loadMigrateAndFetchNodes(email);
    }
    else // we will ask the SDK the email
    {
        mMegaApi->getUserEmail(mMegaApi->getMyUserHandleBinary(),new MegaListenerFuncExecuter(true, [loadMigrateAndFetchNodes](mega::MegaApi*,  mega::MegaRequest* request, mega::MegaError* e) {
                                      QString email;

                                      if (e->getErrorCode() == mega::MegaError::API_OK)
                                      {
                                          auto emailFromRequest = request->getEmail();
                                          if (emailFromRequest)
                                          {
                                              email = QString::fromUtf8(emailFromRequest);
                                          }
                                      }

                                             // in any case, proceed:
                                      loadMigrateAndFetchNodes(email);
                                  }));

    }
}

void LoginController::migrateSyncConfToSdk(const QString& email)
{
    bool needsMigratingFromOldSession = !mPreferences->logged();
    assert(mPreferences->logged() || !email.isEmpty());


    int cachedBusinessState = 999;
    int cachedBlockedState = 999;
    int cachedStorageState = 999;

    auto oldCachedSyncs = mPreferences->readOldCachedSyncs(&cachedBusinessState, &cachedBlockedState, &cachedStorageState, email);
    std::shared_ptr<int>oldCacheSyncsCount(new int(oldCachedSyncs.size()));
    if (*oldCacheSyncsCount > 0)
    {
        if (cachedBusinessState == -2)
        {
            cachedBusinessState = 999;
        }
        if (cachedBlockedState == -2)
        {
            cachedBlockedState = 999;
        }
        if (cachedStorageState == mega::MegaApi::STORAGE_STATE_UNKNOWN)
        {
            cachedStorageState = 999;
        }

        mMegaApi->copyCachedStatus(cachedStorageState, cachedBlockedState, cachedBusinessState);
    }

    foreach(SyncData osd, oldCachedSyncs)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Copying sync data to SDK cache: %1. Name: %2")
                                                                .arg(osd.mLocalFolder).arg(osd.mName).toUtf8().constData());

        mMegaApi->copySyncDataToCache(osd.mLocalFolder.toUtf8().constData(), osd.mName.toUtf8().constData(),
                                         osd.mMegaHandle, osd.mMegaFolder.toUtf8().constData(),
                                         osd.mLocalfp, osd.mEnabled, osd.mTemporarilyDisabled,
                                         new MegaListenerFuncExecuter(true, [this, osd, oldCacheSyncsCount, needsMigratingFromOldSession, email](mega::MegaApi*,  mega::MegaRequest* request, mega::MegaError* e)
                                                                       {
                                                                           if (e->getErrorCode() == mega::MegaError::API_OK)
                                                                           {
                                                                               //preload the model with the restored configuration: that includes info that the SDK does not handle (e.g: syncID)
                                                                               SyncInfo::instance()->pickInfoFromOldSync(osd, request->getParentHandle(), needsMigratingFromOldSession);
                                                                               mPreferences->removeOldCachedSync(osd.mPos, email);
                                                                           }
                                                                           else
                                                                           {
                                                                               mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Failed to copy sync %1: %2").arg(osd.mLocalFolder).arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
                                                                           }

                                                                           --*oldCacheSyncsCount;
                                                                           if (*oldCacheSyncsCount == 0)//All syncs copied to sdk, proceed with fetchnodes
                                                                           {
                                                                               mMegaApi->fetchNodes();
                                                                           }
                                                                       }));
    }

    if (*oldCacheSyncsCount == 0)//No syncs to be copied to sdk, proceed with fetchnodes
    {
        mMegaApi->fetchNodes();
    }
}

void LoginController::loadSyncExclusionRules(const QString& email)
{
    assert(mPreferences->logged() || !email.isEmpty());

           // if not logged in & email provided, read old syncs from that user and load new-cache sync from prev session
    bool temporarilyLoggedPrefs = false;
    if (!mPreferences->logged() && !email.isEmpty())
    {
        temporarilyLoggedPrefs = mPreferences->enterUser(email);
        if (!temporarilyLoggedPrefs) // nothing to load
        {
            return;
        }

        mPreferences->loadExcludedSyncNames(); //to attend the corner case:
                                                // comming from old versions that didn't include some defaults

    }
    assert(mPreferences->logged()); //At this point mPreferences should be logged, just because you enterUser() or it was already logged

    if (!mPreferences->logged())
    {
        return;
    }

    const QStringList exclusions = mPreferences->getExcludedSyncNames();
    std::vector<std::string> vExclusions;
    for (const QString& exclusion : exclusions)
    {
        vExclusions.push_back(exclusion.toStdString());
    }

    mMegaApi->setExcludedNames(&vExclusions);

    const QStringList exclusionPaths = mPreferences->getExcludedSyncPaths();
    std::vector<std::string> vExclusionPaths;
    for (const QString& exclusionPath : exclusionPaths)
    {
        vExclusionPaths.push_back(exclusionPath.toStdString());
    }
    mMegaApi->setExcludedPaths(&vExclusionPaths);

    if (mPreferences->lowerSizeLimit())
    {
        mMegaApi->setExclusionLowerSizeLimit(computeExclusionSizeLimit(mPreferences->lowerSizeLimitValue(), mPreferences->lowerSizeLimitUnit()));
    }
    else
    {
        mMegaApi->setExclusionLowerSizeLimit(0);
    }

    if (mPreferences->upperSizeLimit())
    {
        mMegaApi->setExclusionUpperSizeLimit(computeExclusionSizeLimit(mPreferences->upperSizeLimitValue(), mPreferences->upperSizeLimitUnit()));
    }
    else
    {
        mMegaApi->setExclusionUpperSizeLimit(0);
    }

    if (temporarilyLoggedPrefs)
    {
        mPreferences->leaveUser();
    }
}

long long LoginController::computeExclusionSizeLimit(const long long sizeLimitValue, const int unit)
{
    const double bytesPerKb = 1024;
    const double sizeLimitPower = pow(bytesPerKb, static_cast<double>(unit));
    return sizeLimitValue * static_cast<long long>(sizeLimitPower);
}

void LoginController::runConnectivityCheck()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    if (mPreferences->proxyType() == Preferences::PROXY_TYPE_CUSTOM)
    {
        int proxyProtocol = mPreferences->proxyProtocol();
        switch (proxyProtocol)
        {
            case Preferences::PROXY_PROTOCOL_SOCKS5H:
                proxy.setType(QNetworkProxy::Socks5Proxy);
                break;
            default:
                proxy.setType(QNetworkProxy::HttpProxy);
                break;
        }

        proxy.setHostName(mPreferences->proxyServer());
        proxy.setPort(qint16(mPreferences->proxyPort()));
        if (mPreferences->proxyRequiresAuth())
        {
            proxy.setUser(mPreferences->getProxyUsername());
            proxy.setPassword(mPreferences->getProxyPassword());
        }
    }
    else if (mPreferences->proxyType() == mega::MegaProxy::PROXY_AUTO)
    {
        std::unique_ptr<mega::MegaProxy> autoProxy(mMegaApi->getAutoProxySettings());
        if (autoProxy && autoProxy->getProxyType() == mega::MegaProxy::PROXY_CUSTOM)
        {
            std::string sProxyURL = autoProxy->getProxyURL();
            QString proxyURL = QString::fromUtf8(sProxyURL.data());

            QStringList parts = proxyURL.split(QString::fromUtf8("://"));
            if (parts.size() == 2 && parts[0].startsWith(QString::fromUtf8("socks")))
            {
                proxy.setType(QNetworkProxy::Socks5Proxy);
            }
            else
            {
                proxy.setType(QNetworkProxy::HttpProxy);
            }

            QStringList arguments = parts[parts.size()-1].split(QString::fromUtf8(":"));
            if (arguments.size() == 2)
            {
                proxy.setHostName(arguments[0]);
                proxy.setPort(quint16(arguments[1].toInt()));
            }
        }
    }

    ConnectivityChecker *connectivityChecker = new ConnectivityChecker(Preferences::PROXY_TEST_URL);
    connectivityChecker->setProxy(proxy);
    connectivityChecker->setTestString(Preferences::PROXY_TEST_SUBSTRING);
    connectivityChecker->setTimeout(Preferences::PROXY_TEST_TIMEOUT_MS);

    connect(connectivityChecker, &ConnectivityChecker::testFinished, this,
             &LoginController::onConnectivityCheckFinished, Qt::UniqueConnection);

    connectivityChecker->startCheck();
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Running connectivity test...");
}


void LoginController::onConnectivityCheckFinished(bool success)
{
    sender()->deleteLater();
    if(success)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Connectivity test finished OK");
    }
    else
    {
        MegaSyncApp->showErrorMessage(tr("MEGAsync is unable to connect. Please check your "
                                           "Internet connectivity and local firewall configuration. "
                                           "Note that most antivirus software includes a firewall."));
    }
}

FastLoginController::FastLoginController(QObject *parent)
    : LoginController(parent)
{

}

bool FastLoginController::fastLogin()
{
    QString session = mPreferences->getSession();
    if(session.size())
    {
        mMegaApi->fastLogin(session.toUtf8().constData());
        return true;
    }
    return false;
}

void FastLoginController::onLogin(mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(request)
    //This prevents to handle logins in the initial setup wizard
    if (mPreferences->logged())
    {
        Platform::getInstance()->prepareForSync();
        int errorCode = e->getErrorCode();
        if (errorCode == mega::MegaError::API_OK)
        {
            if (!mPreferences->getSession().isEmpty())
            {
                //Successful login, fetch nodes
                fetchNodes();
                return;
            }
        }
        else if (errorCode != mega::MegaError::API_ESID && errorCode != mega::MegaError::API_ESSL)
        //Invalid session or public key, already managed in TYPE_LOGOUT
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = tr("MEGAsync");
            msgInfo.text = tr("Login error: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString()));

            QMegaMessageBox::warning(msgInfo);
        }

               //Wrong login -> logout
        MegaSyncApp->unlink(true);
    }
    MegaSyncApp->onGlobalSyncStateChanged(mMegaApi);
}

void FastLoginController::onFetchNodesSuccess()
{
    MegaSyncApp->loggedIn(false);
}

LogoutController::LogoutController(QObject *parent)
    : QObject(parent)
      , mMegaApi(MegaSyncApp->getMegaApi())
      , mDelegateListener(new mega::QTMegaRequestListener(MegaSyncApp->getMegaApi(), this))
{
    mMegaApi->addRequestListener(mDelegateListener.get());
    MegaSyncApp->qmlEngine()->rootContext()->setContextProperty(QString::fromUtf8("LogoutControllerAccess"), this);
}

LogoutController::~LogoutController()
{
    if(auto rootContext = MegaSyncApp->qmlEngine()->rootContext())
    {
        rootContext->setContextProperty(QString::fromUtf8("LogoutControllerAccess"), nullptr);
    }
}

void LogoutController::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(api)
    if(request->getType() != mega::MegaRequest::TYPE_LOGOUT)
    {
        return;
    }
    int errorCode = e->getErrorCode();
    int paramType =  request->getParamType();
    if (errorCode || paramType)
    {
        if (errorCode == mega::MegaError::API_EINCOMPLETE && paramType == mega::MegaError::API_ESSL)
        {
            //Typical case: Connecting from a public wifi when the wifi sends you to a landing page
            //SDK cannot connect through SSL securely and asks MEGA Desktop to log out

                   //In previous versions, the user was asked to continue with a warning about a MITM risk.
                   //One of the options was disabling the public key pinning to continue working as usual
                   //This option was to risky and the solution taken was silently retry reconnection

                   // Retry while enforcing key pinning silently
            mMegaApi->retryPendingConnections();
            return;
        }

        if (paramType == mega::MegaError::API_ESID)
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = tr("MEGAsync");
            msgInfo.text = tr("You have been logged out on this computer from another location");
            msgInfo.ignoreCloseAll = true;

            QMegaMessageBox::information(msgInfo);
        }
        else if (paramType == mega::MegaError::API_ESSL)
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = tr("MEGAsync");
            msgInfo.text = tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software "
                               "could be intercepting your communications and causing this problem. Please disable it and try again.")
                           + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown"));
            msgInfo.ignoreCloseAll = true;

            QMegaMessageBox::critical(msgInfo);
        }
        else if (paramType != mega::MegaError::API_EACCESS)
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = tr("MEGAsync");
            msgInfo.text =tr("You have been logged out because of this error: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString()));
            msgInfo.ignoreCloseAll = true;

            QMegaMessageBox::information(msgInfo);
        }
        MegaSyncApp->unlink();
    }

           //Check for any sync disabled by logout to warn user on next login with user&password
    const auto syncSettings (SyncInfo::instance()->getAllSyncSettings());
    auto isErrorLoggedOut = [](std::shared_ptr<SyncSettings> s) {return s->getError() == mega::MegaSync::LOGGED_OUT;};
    if (std::any_of(syncSettings.cbegin(), syncSettings.cend(), isErrorLoggedOut))
    {
        Preferences::instance()->setNotifyDisabledSyncsOnLogin(true);
    }

    emit logout(!request->getFlag());
}
