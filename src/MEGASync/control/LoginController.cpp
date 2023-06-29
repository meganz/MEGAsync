#include "LoginController.h"
#include "MegaApplication.h"
#include "Preferences/Preferences.h"
#include "ConnectivityChecker.h"
#include "Platform.h"
#include "QMegaMessageBox.h"

LoginController::LoginController(QObject *parent)
    : QObject{parent}
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new mega::QTMegaRequestListener(MegaSyncApp->getMegaApi(), this))
    , mPreferences(Preferences::instance())
    , mFetchingNodes(false)
    , mEmailConfirmed(false)
{
    mMegaApi->addRequestListener(mDelegateListener.get());
    mConnectivityTimer = new QTimer(this);
    mConnectivityTimer->setSingleShot(true);
    mConnectivityTimer->setInterval(Preferences::MAX_LOGIN_TIME_MS);
    connect(mConnectivityTimer, &QTimer::timeout, this, &LoginController::runConnectivityCheck);

    EphemeralCredentials credentials = mPreferences->getEphemeralCredentials();
    if(credentials.sessionId.size() > 0)
    {
        mMegaApi->resumeCreateAccount(credentials.sessionId.toUtf8().constData());
    }
    new EmailConfirmationListener(this);
}

LoginController::~LoginController()
{
}

void LoginController::login(const QString &email, const QString &password)
{
    mMegaApi->login(email.toUtf8().constData(), password.toUtf8().constData());
}

void LoginController::createAccount(const QString &email, const QString &password,
                                    const QString &name, const QString &lastName)
{
    mMegaApi->createAccount(email.toUtf8().constData(), password.toUtf8().constData(),
                            name.toUtf8().constData(), lastName.toUtf8().constData());
}

void LoginController::changeRegistrationEmail(const QString &email)
{
    QString fullName = mName + QString::fromUtf8(" ") + mLastName;
    mMegaApi->resendSignupLink(email.toUtf8().constData(), fullName.toUtf8().constData());
}

void LoginController::login2FA(const QString &pin)
{
    mMegaApi->multiFactorAuthLogin(mEmail.toUtf8().constData(), mPassword.toUtf8().constData(), pin.toUtf8().constData());
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
            std::unique_ptr<char []> session(mMegaApi->dumpSession());
            if (session)
            {
                qDebug()<<"SESSION:" << QString::fromUtf8(session.get());
                mPreferences->setSession(QString::fromUtf8(session.get()));
            }
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
            emit fetchingNodesProgress(progress);
        }
    }
}

void LoginController::onRequestStart(mega::MegaApi *api, mega::MegaRequest *request)
{
    Q_UNUSED(api)
    if(request->getType() == mega::MegaRequest::TYPE_LOGIN)
    {
        mConnectivityTimer->start();
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
    if(e->getErrorCode() == mega::MegaError::API_OK)
    {
        MegaSyncApp->initLocalServer();
        mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);

        auto email = request->getEmail();
        fetchNodes(QString::fromUtf8(email ? email : ""));
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
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("Please check your e-mail and click the link to confirm your account."), QMessageBox::Ok);
            break;
        }
        case mega::MegaError::API_ETOOMANY:
        {
            QMegaMessageBox::warning(nullptr, tr("Error"),
                                     tr("You have attempted to log in too many times.[BR]Please wait until %1 and try again.")
                                         .replace(QString::fromUtf8("[BR]"), QString::fromUtf8("\n"))
                                         .arg(QTime::currentTime().addSecs(3600).toString(QString::fromUtf8("hh:mm")))
                                     , QMessageBox::Ok);
            break;
        }
        case mega::MegaError::API_EBLOCKED:
        {
            QMegaMessageBox::critical(nullptr, tr("Error"), tr("Your account has been blocked. Please contact support@mega.co.nz"));
            break;
        }
        case mega::MegaError::API_EMFAREQUIRED:
        {
            mPassword = QString::fromUtf8(request->getPassword());
            mEmail = QString::fromUtf8(request->getEmail());
            break;
        }
        default:
            if(e->getErrorCode() != mega::MegaError::API_ESSL)
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), QCoreApplication::translate("MegaError", e->getErrorString()), QMessageBox::Ok);
            }
            break;
        }
    }

    MegaSyncApp->onGlobalSyncStateChanged(mMegaApi);
    emit loginFinished(e->getErrorCode());
}

void LoginController::onFetchNodesSuccess()
{
    std::unique_ptr<char[]> email(mMegaApi->getMyEmail());

    // We will proceed with a new login
    mPreferences->setEmailAndGeneralSettings(QString::fromUtf8(email.get()));
    SyncInfo::instance()->rewriteSyncSettings(); //write sync settings into user's preferences

    MegaSyncApp->loggedIn(true);
}

void LoginController::onAccountCreation(mega::MegaRequest *request, mega::MegaError *e)
{
    if(e->getErrorCode() == mega::MegaError::API_OK)
    {
        mEmail = QString::fromUtf8(request->getEmail());
        emit emailChanged();
        mPassword = QString::fromUtf8(request->getPassword());
        emit passwordChanged();
        mName = QString::fromUtf8(request->getName());
        mLastName = QString::fromUtf8(request->getText());
        EphemeralCredentials credentials;
        credentials.email = mEmail;
        credentials.password = mPassword;
        credentials.sessionId = QString::fromUtf8(request->getSessionKey());
        mPreferences->setEphemeralCredentials(credentials);
        //qDebug()<<"SESSION:" << QString::fromUtf8(request->getSessionKey()); //ephemeral session id and final session id are different, create new key for ephemeral session
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
        mPassword = credentials.password;
        emit emailChanged();
        emit accountCreationResumed();
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

        // TODO: check with sdk team if this case is possible
        if (!MegaSyncApp->getRootNode())
        {
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("Unable to get the filesystem.\n"
                                                              "Please, try again. If the problem persists "
                                                              "please contact bug@mega.co.nz"), QMessageBox::Ok);
            MegaSyncApp->rebootApplication(false);
            return;
        }

        onFetchNodesSuccess();
        emit fetchingNodesFinished();
    }
    else
    {
        mPreferences->setAccountStateInGeneral(Preferences::STATE_FETCHNODES_FAILED);
        mPreferences->setNeedsFetchNodesInGeneral(true);
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error fetching nodes: %1")
                                                               .arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
    }
}

void LoginController::onLogout(mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(e)

    //This message is shown every time the user tries to login and the SSL fails
    if(request->getParamType() == mega::MegaError::API_ESSL)
    {
        QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                                  tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                                      + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")));
        megaApi()->localLogout();
        // TODO: to login page??
    }
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
            megaApi()->fetchNodes();
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

        megaApi()->copyCachedStatus(cachedStorageState, cachedBlockedState, cachedBusinessState);
    }

    foreach(SyncData osd, oldCachedSyncs)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Copying sync data to SDK cache: %1. Name: %2")
                                                               .arg(osd.mLocalFolder).arg(osd.mName).toUtf8().constData());

        megaApi()->copySyncDataToCache(osd.mLocalFolder.toUtf8().constData(), osd.mName.toUtf8().constData(),
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
                                                                            megaApi()->fetchNodes();
                                                                        }
                                                                    }));
    }

    if (*oldCacheSyncsCount == 0)//No syncs to be copied to sdk, proceed with fetchnodes
    {
        megaApi()->fetchNodes();
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

    QStringList exclusions = mPreferences->getExcludedSyncNames();
    std::vector<std::string> vExclusions;
    for (int i = 0; i < exclusions.size(); i++)
    {
        vExclusions.push_back(exclusions[i].toUtf8().constData());
    }
    megaApi()->setExcludedNames(&vExclusions);

    QStringList exclusionPaths = mPreferences->getExcludedSyncPaths();
    std::vector<std::string> vExclusionPaths;
    for (int i = 0; i < exclusionPaths.size(); i++)
    {
        vExclusionPaths.push_back(exclusionPaths[i].toUtf8().constData());
    }
    megaApi()->setExcludedPaths(&vExclusionPaths);

    if (mPreferences->lowerSizeLimit())
    {
        megaApi()->setExclusionLowerSizeLimit(computeExclusionSizeLimit(mPreferences->lowerSizeLimitValue(), mPreferences->lowerSizeLimitUnit()));
    }
    else
    {
        megaApi()->setExclusionLowerSizeLimit(0);
    }

    if (mPreferences->upperSizeLimit())
    {
        megaApi()->setExclusionUpperSizeLimit(computeExclusionSizeLimit(mPreferences->upperSizeLimitValue(), mPreferences->upperSizeLimitUnit()));
    }
    else
    {
        megaApi()->setExclusionUpperSizeLimit(0);
    }

    if (temporarilyLoggedPrefs)
    {
        mPreferences->leaveUser();
    }
}

long long LoginController::computeExclusionSizeLimit(const long long sizeLimitValue, const int unit)
{
    const double sizeLimitPower = pow(static_cast<double>(1024), static_cast<double>(unit));
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
        mega::MegaProxy* autoProxy = megaApi()->getAutoProxySettings();
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
        delete autoProxy;
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
        megaApi()->fastLogin(session.toUtf8().constData());
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
        else if (errorCode == mega::MegaError::API_EBLOCKED)
        {
            QMegaMessageBox::critical(nullptr, tr("MEGAsync"), tr("Your account has been blocked. Please contact support@mega.co.nz"));
        }
        else if (errorCode != mega::MegaError::API_ESID && errorCode != mega::MegaError::API_ESSL)
        //Invalid session or public key, already managed in TYPE_LOGOUT
        {
            QMegaMessageBox::warning(nullptr, tr("MEGAsync"), tr("Login error: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString())));
        }

        //Wrong login -> logout
        MegaSyncApp->unlink(true);
        MegaSyncApp->onGlobalSyncStateChanged(megaApi());
    }
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
}

void LogoutController::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(api)
    if(request->getType() != mega::MegaRequest::TYPE_LOGOUT)
    {
        return;
    }
    int errorCode = e->getErrorCode();
    if (errorCode)
    {
        if (errorCode == mega::MegaError::API_EINCOMPLETE && request->getParamType() == mega::MegaError::API_ESSL)
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

        if (errorCode == mega::MegaError::API_ESID)
        {
            QMegaMessageBox::information(nullptr, QString::fromUtf8("MEGAsync"), tr("You have been logged out on this computer from another location"));
        }
        else if (errorCode == mega::MegaError::API_ESSL)
        {
            QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                                      tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                                          + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")));
        }
        else if (errorCode != mega::MegaError::API_EACCESS)
        {
            QMegaMessageBox::information(nullptr, QString::fromUtf8("MEGAsync"), tr("You have been logged out because of this error: %1")
                                                                                     .arg(QCoreApplication::translate("MegaError", e->getErrorString())));
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
    emit onLogoutFinished();
}


EmailConfirmationListener::EmailConfirmationListener(LoginController* parent)
    : QObject(parent)
    , mGlobalListener(new mega::QTMegaGlobalListener(MegaSyncApp->getMegaApi(), this))
{
    MegaSyncApp->getMegaApi()->addGlobalListener(mGlobalListener.get());
}

void EmailConfirmationListener::onEvent(mega::MegaApi *, mega::MegaEvent *event)
{
    if(event->getType() == mega::MegaEvent::EVENT_CONFIRM_USER_EMAIL)
    {
        static_cast<LoginController*>(parent())->emailConfirmation(QString::fromLatin1(event->getText()));
    }
}
