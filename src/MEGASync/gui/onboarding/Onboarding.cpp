#include "Preferences.h"
#include "Onboarding.h"
#include "MegaApplication.h"
#include "UserAttributesRequests/DeviceName.h"
#include "TextDecorator.h"
#include "QMegaMessageBox.h"
#include <QQmlEngine>

using namespace mega;

Onboarding::Onboarding(QObject *parent)
    : QMLComponent(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new QTMegaRequestListener(mMegaApi, this))
    , mGlobalListener(new QTMegaGlobalListener(mMegaApi, this))
    , mPreferences(Preferences::instance())
    , mSyncController(new SyncController())
    , mBackupController(new SyncController())
    , mPassword(QString())
    , mLastName(QString())
    , mFirstName(QString())
    , mEmail(QString())
{
    mMegaApi->addGlobalListener(mGlobalListener.get());
    mMegaApi->addRequestListener(mDelegateListener.get());

    qmlRegisterUncreatableType<Onboarding>("Onboarding", 1, 0, "RegisterForm", QString::fromUtf8("Cannot create WarningLevel in QML"));
    qmlRegisterUncreatableType<Onboarding>("Onboarding", 1, 0, "PasswordStrength", QString::fromUtf8("Cannot create WarningLevel in QML"));

    qmlRegisterModule("Onboard", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/OnboardingDialog.qml")), "Onboard", 1, 0, "OnboardingDialog");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/content/onboard/OnboardingStrings.qml")), "Onboard", 1, 0, "OnboardingStrings");

    qmlRegisterModule("Onboard.Syncs_types", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsFlow.qml")), "Onboard.Syncs_types", 1, 0, "SyncsFlow");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/Header.qml")), "Onboard.Syncs_types", 1, 0, "Header");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/Footer.qml")), "Onboard.Syncs_types", 1, 0, "Footer");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/InfoAccount.qml")), "Onboard.Syncs_types", 1, 0, "InfoAccount");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/ResumePage.qml")), "Onboard.Syncs_types", 1, 0, "ResumePage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsPage.qml")), "Onboard.Syncs_types", 1, 0, "SyncsPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/InstallationType.qml")), "Onboard.Syncs_types", 1, 0, "InstallationType");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/ResumeButton.qml")), "Onboard.Syncs_types", 1, 0, "ResumeButton");

    qmlRegisterModule("Onboard.Syncs_types.Syncs", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/SyncTypePage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "SyncTypePage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/FullSyncPage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "FullSyncPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/SelectiveSyncPage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "SelectiveSyncPage");

    qmlRegisterModule("Onboard.Syncs_types.Left_panel", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/left_panel/StepPanel.qml")), "Onboard.Syncs_types.Left_panel", 1, 0, "StepPanel");

    qmlRegisterModule("Onboard.Syncs_types.Backups", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/backups/ConfirmFoldersPage.qml")), "Onboard.Syncs_types.Backups", 1, 0, "ConfirmFoldersPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/backups/SelectFoldersPage.qml")), "Onboard.Syncs_types.Backups", 1, 0, "SelectFoldersPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/backups/RenameBackupFolderPage.qml")), "Onboard.Syncs_types.Backups", 1, 0, "RenameBackupFolderPage");

    connect(mSyncController, &SyncController::syncAddStatus,
            this, &Onboarding::onSyncAddRequestStatus);

    connect(mBackupController, &SyncController::syncAddStatus,
            this, &Onboarding::onBackupAddRequestStatus);
}

QUrl Onboarding::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/main.qml"));
}

QString Onboarding::contextName()
{
    return QString::fromUtf8("Onboarding");
}

void Onboarding::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* error)
{
    switch(request->getType())
    {
    case MegaRequest::TYPE_LOGIN:
    {
        if (error->getErrorCode() == MegaError::API_OK)
        {
            if(mPreferences->logged())
            {
                return;
            }
            qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN API_OK";
            mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
            mEmail = QString::fromUtf8(request->getEmail());
            emit emailChanged(mEmail);
            mPreferences->setEmailAndGeneralSettings(mEmail);
            if (!mPreferences->hasLoggedIn())
            {
                mPreferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
            }
            emit loginFinished();
        }
        else
        {
            mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_FAILED);
            if (error->getErrorCode() == MegaError::API_EMFAREQUIRED)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN API_EMFAREQUIRED";
                mEmail = QString::fromUtf8(request->getEmail());
                mPassword = QString::fromUtf8(request->getPassword());
                emit twoFARequired();
            }
            else if (error->getErrorCode() == MegaError::API_EFAILED)
            {
                qDebug() << "Onboarding::onRequestFinish -> API_EFAILED";
                emit twoFAFailed();
            }
            else
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN Error code -> "
                         << error->getErrorCode() << " Error string -> " << error->getErrorString();
                emit userPassFailed();
            }
        }
        break;
    }
    case MegaRequest::TYPE_CREATE_ACCOUNT:
    {
        if(error->getErrorCode() == MegaError::API_OK)
        {
            mEmail = QString::fromUtf8(request->getEmail());
            mPassword = QString::fromUtf8(request->getPassword());
            mFirstName = QString::fromUtf8(request->getName());
            mLastName = QString::fromUtf8(request->getText());
            emit emailChanged(mEmail);
        }
        emit registerFinished(error->getErrorCode() == MegaError::API_OK); //only fail is email already exist
        qDebug() << QString::fromUtf8("Onboarding::onRequestFinish -> TYPE_CREATE_ACCOUNT Error code: %1").arg(error->getErrorCode());
        break;
    }
    case MegaRequest::TYPE_SEND_SIGNUP_LINK:
    {
        if(error->getErrorCode() == MegaError::API_OK)
        {
            mEmail = QString::fromUtf8(request->getEmail());
            emit emailChanged(mEmail);
        }
        emit changeRegistrationEmailFinished(error->getErrorCode() == MegaError::API_OK);
    }
    case MegaRequest::TYPE_FETCH_NODES:
    {
        if (error->getErrorCode() == MegaError::API_OK)
        {
            emit fetchingNodesProgress(1);
        }
//        if (error->getErrorCode() != MegaError::API_OK)
//        {
//            loggingStarted = false;

//            if (error->getErrorCode() != MegaError::API_EBLOCKED) //TODO: review this case API_EBLOCKED
//            {
//                page_login();
//            }
//            break;
//        }

//        if (loggingStarted)
//        {
//            if (!megaApi->isFilesystemAvailable())
//            {
//                page_login();
//                return;
//            }
//        }
        break;
    }
    }
}

void Onboarding::onRequestUpdate(mega::MegaApi *, mega::MegaRequest *request)
{
    if (request->getType() == MegaRequest::TYPE_FETCH_NODES)
    {
        if (request->getTotalBytes() > 0)
        {
            double total = static_cast<double>(request->getTotalBytes());
            double part = static_cast<double>(request->getTransferredBytes());
            double progress = part/total;
            if(progress > 0.6)
            {
                progress = 0.6;
            }
            emit fetchingNodesProgress(progress);
        }
    }
}

void Onboarding::onEvent(mega::MegaApi *, mega::MegaEvent *event)
{
    if(event->getType() == MegaEvent::EVENT_ACCOUNT_CONFIRMATION)
    {
        emit accountConfirmed();
    }
}

void Onboarding::onLoginClicked(const QVariantMap& data)
{
    qDebug() << "Onboarding::onLoginClicked" << data;

    std::string email = data.value(QString::number(EMAIL)).toString().toStdString();
    std::string password = data.value(QString::number(PASSWORD)).toString().toStdString();
    mMegaApi->login(email.c_str(), password.c_str(), this->mDelegateListener.get());
}

void Onboarding::onRegisterClicked(const QVariantMap& data)
{
    qDebug() << "Onboarding::onRegisterClicked" << data;
    QString firstName = data.value(QString::number(FIRST_NAME)).toString();
    QString lastName = data.value(QString::number(LAST_NAME)).toString();
    QString email = data.value(QString::number(EMAIL)).toString();
    QString password = data.value(QString::number(PASSWORD)).toString();

    mMegaApi->createAccount(email.toUtf8().constData(),
                            password.toUtf8().constData(),
                            firstName.toUtf8().constData(),
                            lastName.toUtf8().constData(),
                            this->mDelegateListener.get());
}

void Onboarding::onTwoFARequested(const QString& pin)
{
    mMegaApi->multiFactorAuthLogin(mEmail.toUtf8().constData(),
                                   mPassword.toUtf8().constData(),
                                   pin.toUtf8().constData(),
                                   this->mDelegateListener.get());
}

QString Onboarding::convertUrlToNativeFilePath(const QUrl &urlStylePath) const
{
    return QDir::toNativeSeparators(urlStylePath.toLocalFile());
}

void Onboarding::addSync(const QString &localPath, mega::MegaHandle remoteHandle)
{
    if(remoteHandle == mega::INVALID_HANDLE)
    {
        remoteHandle = MegaSyncApp->getRootNode()->getHandle();
    }

    QString warningMessage;
    auto syncability (SyncController::isLocalFolderAllowedForSync(localPath, MegaSync::TYPE_TWOWAY, warningMessage));
    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = SyncController::areLocalFolderAccessRightsOk(localPath, MegaSync::TYPE_TWOWAY, warningMessage);
    }

    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = SyncController::isLocalFolderSyncable(localPath, MegaSync::TYPE_TWOWAY, warningMessage);
    }

    // If OK, check that we can sync the selected remote folder
    std::shared_ptr<MegaNode> node (mMegaApi->getNodeByHandle(remoteHandle));

    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = std::max(SyncController::isRemoteFolderSyncable(node, warningMessage), syncability);
    }

    if (syncability == SyncController::CANT_SYNC)
    {
        // If can't sync because remote node does not exist, try to create it
        if (!node)
        {
            auto rootNode = MegaSyncApp->getRootNode();
            if (!rootNode)
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("Unable to get the filesystem.\n"
                                                                  "Please, try again. If the problem persists "
                                                                  "please contact bug@mega.co.nz"));
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !rootNode (Onboarding)");
                mPreferences->setCrashed(true);
                MegaSyncApp->rebootApplication(false);
            }
        }
        else
        {
            QMegaMessageBox::warning(nullptr, tr("Warning"), warningMessage, QMessageBox::Ok);
        }
    }
    else if (syncability == SyncController::CAN_SYNC
             || (syncability == SyncController::WARN_SYNC
                 && QMegaMessageBox::warning(nullptr, tr("Warning"), warningMessage
                                             + QLatin1Char('\n')
                                             + tr("Do you want to continue?"),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                 == QMessageBox::Yes))
    {
        mSyncController->addSync(localPath, remoteHandle);
    }
    qDebug()<<localPath<<":"<<remoteHandle;
}

void Onboarding::addBackups(const QStringList& localPathList)
{
    if(localPathList.size() <= 0)
    {
        return;
    }

    mBackupsToDoList.clear();
    for(const QString& path : localPathList)
    {
        mBackupsToDoList.append(QPair<QString, QString>(path, QString::fromUtf8("")));
    }
    createNextBackup();
}

bool Onboarding::setDeviceName(const QString &deviceName)
{
    return UserAttributes::DeviceName::requestDeviceName()->setDeviceName(deviceName);
}

Onboarding::PasswordStrength Onboarding::getPasswordStrength(const QString &password)
{
    return static_cast<Onboarding::PasswordStrength>(mMegaApi->getPasswordStrength(password.toUtf8().constData()));
}

void Onboarding::changeRegistrationEmail(const QString &email)
{
    QString fullName = mFirstName + QString::fromUtf8(" ") + mLastName;
    mMegaApi->sendSignupLink(email.toUtf8().constData(), fullName.toUtf8().constData(), mPassword.toUtf8().constData(), mDelegateListener.get());
}

QString Onboarding::getEmail()
{
    return mEmail;
}

void Onboarding::createNextBackup(const QString& name)
{
    if(mBackupsToDoList.size() <= 0)
    {
        return;
    }

    QString backupName(name);
    if(name.isEmpty())
    {
        backupName = mBackupController->getSyncNameFromPath(mBackupsToDoList.first().first);
    }
    else
    {
        mBackupsToDoList.first().second = name;
    }
    mBackupController->addBackup(mBackupsToDoList.first().first, backupName);
}

void Onboarding::exitLoggedIn()
{
    std::unique_ptr<char[]> email(mMegaApi->getMyEmail());
    mPreferences->setEmailAndGeneralSettings(QString::fromUtf8(email.get()));
    emit exitLoggedInFinished();
}

void Onboarding::getComputerName()
{
    auto request = UserAttributes::DeviceName::requestDeviceName();
    connect(request.get(), &UserAttributes::DeviceName::attributeReady, this, &Onboarding::deviceNameReady, Qt::UniqueConnection);
    if(request->isAttributeReady())
    {
        emit deviceNameReady(request->getDeviceName());
    }
}

void Onboarding::openPreferences(bool sync) const
{
    int tab = SettingsDialog::BACKUP_TAB;
    if(sync)
    {
        tab = SettingsDialog::SYNCS_TAB;
    }
    MegaSyncApp->openSettings(tab);
}

void Onboarding::onSyncAddRequestStatus(int errorCode,
                                        const QString &errorMsg,
                                        const QString &name)
{
    Q_UNUSED(name)
    if (errorCode != MegaError::API_OK)
    {
        Text::Link link(Utilities::SUPPORT_URL);
        Text::Decorator dec(&link);
        QString msg = errorMsg;
        dec.process(msg);
        QMegaMessageBox::warning(nullptr, tr("Error adding sync"), msg, QMessageBox::Ok, QMessageBox::NoButton, QMap<QMessageBox::StandardButton, QString>(), Qt::RichText);
    }
    else
    {
        emit syncSetupSuccess();
    }
}

void Onboarding::onBackupAddRequestStatus(int errorCode,
                                          const QString& errorMsg,
                                          const QString& name)
{
    qDebug() << "Onboarding::onBackupAddRequestStatus -> path = " << name
             << " - errorCode = " << errorCode << " - errorMsg = " << errorMsg;

    if(errorCode == 0)
    {
        mBackupsToDoList.removeFirst();
        if(mBackupsToDoList.size() > 0)
        {
            createNextBackup();
        }
    }
    else
    {
        bool found = false;
        auto it = mBackupsToDoList.constBegin();
        QString backupName(QString::fromUtf8(""));
        while(!found && it != mBackupsToDoList.constEnd())
        {
            if((found = (it->first == name)))
            {
                backupName = it->second;
            }
            it++;
        }

        if(found)
        {
            // Wait until the rename conflict has been resolved
            emit backupConflict(mBackupController->getSyncNameFromPath(name),
                                backupName,
                                backupName == QString::fromUtf8(""));
        }
    }
    emit backupsUpdated(name, errorCode, mBackupsToDoList.size() == 0);
}
