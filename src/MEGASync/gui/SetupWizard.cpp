#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "control/Utilities.h"
#include "DialogOpener.h"
#include "control/AppStatsEvents.h"
#include "gui/Login2FA.h"
#include "gui/GuiUtilities.h"
#include "platform/Platform.h"
#include "DialogOpener.h"

#include <QKeyEvent>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

SetupWizard::SetupWizard(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupWizard)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::WindowModal);

    animationTimer = new QTimer(this);
    animationTimer->setSingleShot(true);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(animationTimout()));

    connect(ui->ePassword, SIGNAL(textChanged(QString)), this, SLOT(onPasswordTextChanged(QString)));

    ui->wAdvancedSetup->installEventFilter(this);
    ui->wTypicalSetup->installEventFilter(this);
    ui->lTermsLink->installEventFilter(this);
    ui->rTypicalSetup->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->rAdvancedSetup->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->wHelp->hide();
    connect(ui->wHelp, SIGNAL(clicked()), this, SLOT(on_bLearMore_clicked()));

    this->app = app;
    this->closing = false;
    this->loggingStarted = false;
    this->creatingDefaultSyncFolder = false;
    this->closeBlocked = false;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    delegateListener = new QTMegaRequestListener(megaApi, this);
    megaApi->addRequestListener(delegateListener);

    QString agreeLink = ui->lTermsLink->text();
    agreeLink.replace(QLatin1String("[A]"), QLatin1String("<a href=\"https://mega.nz/#terms\" style=\"color:#DC0000\">"));
    agreeLink.replace(QLatin1String("[/A]"), QLatin1String("</a>"));
    ui->lTermsLink->setText(agreeLink);

    m_animation = new QPropertyAnimation(ui->wErrorMessage, "size");
    m_animation->setDuration(400);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    m_animation->setStartValue(QSize(ui->wErrorMessage->minimumWidth(), ui->wErrorMessage->minimumHeight()));
    m_animation->setEndValue(QSize(ui->wErrorMessage->maximumWidth(), ui->wErrorMessage->maximumHeight()));
    connect(m_animation, SIGNAL(finished()), this, SLOT(onErrorAnimationFinished()));

    page_newaccount();

    ui->lError->setText(QString::fromUtf8(""));
    ui->lError->hide();
    ui->wErrorMessage->resize(QSize(ui->wErrorMessage->minimumWidth(),ui->wErrorMessage->minimumHeight()));

#if 0 //Strings for the translation system. These lines don't need to be built
    QT_TR_NOOP("Very Weak");
    QT_TR_NOOP("Weak");
    QT_TR_NOOP("Medium");
    QT_TR_NOOP("Good");
    QT_TR_NOOP("Strong");
#endif
}

SetupWizard::~SetupWizard()
{
    delete delegateListener;
    delete animationTimer;
    delete m_animation;
    delete ui;
}

void SetupWizard::onRequestStart(MegaApi*, MegaRequest* request)
{
    if (request->getType() == MegaRequest::TYPE_LOGIN)
    {
        ui->lProgress->setText(tr("Logging in..."));
        page_progress();
    }
    else if (request->getType() == MegaRequest::TYPE_LOGOUT && request->getFlag())
    {
        closing = true;
        page_logout();
    }
}

void SetupWizard::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *error)
{
    if (closing)
    {
        if (request->getType() == MegaRequest::TYPE_LOGOUT)
        {
            done(QDialog::Rejected);
        }
        return;
    }

    switch (request->getType())
    {
        case MegaRequest::TYPE_CREATE_ACCOUNT:
        {
            if (error->getErrorCode() == MegaError::API_OK)
            {
                page_login();

                ui->eLoginEmail->setText(ui->eEmail->text().toLower().trimmed());
                ui->eName->clear();
                ui->eLastName->clear();
                ui->eEmail->clear();
                ui->ePassword->clear();
                ui->eRepeatPassword->clear();

                megaApi->sendEvent(AppStatsEvents::EVENT_ACC_CREATION_START,
                                   "MEGAsync account creation start");
                if (!preferences->accountCreationTime())
                {
                    preferences->setAccountCreationTime(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                }
                break;
            }

            page_newaccount();

            if (error->getErrorCode() == MegaError::API_EEXIST)
            {
                showErrorMessage(tr("User already exists"));
            }
            else if (error->getErrorCode() != MegaError::API_ESSL)
            {
                showErrorMessage(QCoreApplication::translate("MegaError", error->getErrorString()));
            }
            break;
        }
        case MegaRequest::TYPE_LOGIN:
        {
            if (error->getErrorCode() == MegaError::API_EMFAREQUIRED
                    || error->getErrorCode() == MegaError::API_EFAILED
                    || error->getErrorCode() == MegaError::API_EEXPIRED)
            {
                ui->bCancel->setEnabled(false);
                closeBlocked = true;
            }

            if (error->getErrorCode() == MegaError::API_OK)
            {
                if (loggingStarted)
                {
                    preferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
                    auto email = request->getEmail();
                    static_cast<MegaApplication*>(qApp)->fetchNodes(QString::fromUtf8(email ? email : ""));
                    if (!preferences->hasLoggedIn())
                    {
                        preferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                    }
                }

                ui->lProgress->setText(tr("Fetching file list..."));
                page_progress();
                break;
            }

            if (loggingStarted)
            {
                preferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_FAILED);
                if (error->getErrorCode() == MegaError::API_ENOENT)
                {
                    showErrorMessage(tr("Incorrect email and/or password."));
                }
                else if (error->getErrorCode() == MegaError::API_EMFAREQUIRED)
                {
                    show2FA(request, false);
                    return;
                }
                else if (error->getErrorCode() == MegaError::API_EINCOMPLETE)
                {
                    showErrorMessage(tr("Please check your e-mail and click the link to confirm your account."));
                }
                else if (error->getErrorCode() == MegaError::API_ETOOMANY)
                {
                    showErrorMessage(tr("You have attempted to log in too many times.[BR]Please wait until %1 and try again.")
                                        .replace(QString::fromUtf8("[BR]"), QString::fromUtf8("\n"))
                                        .arg(QTime::currentTime().addSecs(3600).toString(QString::fromUtf8("hh:mm"))));
                }
                else if (error->getErrorCode() == MegaError::API_EBLOCKED)
                {
                    showErrorMessage(tr("Your account has been blocked. Please contact support@mega.co.nz"));
                }
                else if (error->getErrorCode() == MegaError::API_EFAILED || error->getErrorCode() == MegaError::API_EEXPIRED)
                {
                    show2FA(request, true);
                    return;
                }
                else if (error->getErrorCode() != MegaError::API_ESSL)
                {
                    showErrorMessage(QCoreApplication::translate("MegaError", error->getErrorString()));
                }

                loggingStarted = false;
                page_login();
            }
            else if (error->getErrorCode() != MegaError::API_EMFAREQUIRED
                     && error->getErrorCode() != MegaError::API_EFAILED
                     && error->getErrorCode() != MegaError::API_EEXPIRED)
            {
                page_newaccount();
            }
            break;
        }
        case MegaRequest::TYPE_CREATE_FOLDER:
        {
            if (!creatingDefaultSyncFolder)
            {
                break;
            }

            creatingDefaultSyncFolder = false;

            if (error->getErrorCode() == MegaError::API_OK)
            {
                MegaNode *node = megaApi->getNodeByPath("/MEGAsync");
                if (!node)
                {
                    QMegaMessageBox::warning(nullptr, tr("Error"), tr("MEGA folder doesn't exist"), QMessageBox::Ok);
                }
                else
                {
                    selectedMegaFolderHandle = node->getHandle();
                    page_welcome();
                    delete node;
                }
                break;
            }

            if (error->getErrorCode() != MegaError::API_ESSL
                    && error->getErrorCode() != MegaError::API_ESID)
            {
                QMegaMessageBox::warning(nullptr, tr("Error"),  QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
            }

            break;
        }
        case MegaRequest::TYPE_FETCH_NODES:
        {
            if (error->getErrorCode() != MegaError::API_OK)
            {
                if (loggingStarted)
                {
                    if (error->getErrorCode() == MegaError::API_EBLOCKED)
                    {
                        done(QDialog::Rejected);
                    }
                    else
                    {
                        page_login();
                    }

                    loggingStarted = false;
                }
                else
                {
                    page_newaccount();
                }
                break;
            }
            loggingStarted = false;

            break;
        }
        case MegaRequest::TYPE_LOGOUT:
        {
            // If logging was started at GuestWidget logout should forward to create account
            // Other cases should forward to login page
            if (loggingStarted)
            {
                loggingStarted = false;
                page_login();
            }
            else
            {
                page_newaccount();
            }
        }
    }
}

void SetupWizard::show2FA(MegaRequest *request, bool invalidCode)
{
    QPointer<Login2FA> verification(new Login2FA());
    verification->invalidCode(invalidCode);
    std::shared_ptr<MegaRequest> requestCopy(request->copy());
    DialogOpener::showDialog<Login2FA>(verification, [requestCopy, verification,this](){
        if (verification->result() != QDialog::Accepted)
        {
            megaApi->localLogout();
            page_login();
            loggingStarted = false;
        }
        else
        {
            QString pin = verification->pinCode();
            megaApi->multiFactorAuthLogin(requestCopy->getEmail(), requestCopy->getPassword(), pin.toUtf8().constData());
        }
    });
}

void SetupWizard::onRequestUpdate(MegaApi *, MegaRequest *request)
{
    GuiUtilities::updateDataRequestProgressBar(ui->progressBar, request);
}

void SetupWizard::goToStep(int page)
{
    QWidget *wPage = ui->sPages->currentWidget();
    if (wPage != ui->pLogin
            && wPage != ui->pNewAccount
            && wPage != ui->pProgress)
    {
        return;
    }

    switch (page)
    {
        case PAGE_LOGIN:
            page_login();
            break;

        case PAGE_NEW_ACCOUNT:
            page_newaccount();
            break;
        case PAGE_MODE:
            page_mode();
            break;

        default:
            return;
    }
}

void SetupWizard::on_bNext_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if (w == ui->pLogin)
    {
        QString email = ui->eLoginEmail->text().toLower().trimmed();
        QString password = ui->eLoginPassword->text();

        if (!email.length())
        {
            showErrorMessage(tr("Please, enter your e-mail address"));
            return;
        }

        if (!email.contains(QChar::fromAscii('@')) || !email.contains(QChar::fromAscii('.')))
        {
            showErrorMessage(tr("Please, enter a valid e-mail address"));
            return;
        }

        if (!password.length())
        {
            showErrorMessage(tr("Please, enter your password"));
            return;
        }

        megaApi->login(email.toUtf8().constData(), password.toUtf8().constData());
        loggingStarted = true;
    }
    else if (w == ui->pNewAccount)
    {
        QString name = ui->eName->text().trimmed();
        QString lastName = ui->eLastName->text().trimmed();
        QString email = ui->eEmail->text().toLower().trimmed();
        QString password = ui->ePassword->text();
        QString repeatPassword = ui->eRepeatPassword->text();

        if (!name.length())
        {
            showErrorMessage(tr("Please, enter your name"));
            return;
        }

        if (!lastName.length())
        {
            showErrorMessage(tr("Please, enter your last name"));
            return;
        }

        if (!email.length())
        {
            showErrorMessage(tr("Please, enter your e-mail address"));
            return;
        }

        if (!email.contains(QChar::fromAscii('@')) || !email.contains(QChar::fromAscii('.')))
        {
            showErrorMessage(tr("Please, enter a valid e-mail address"));
            return;
        }

        if (!password.length())
        {
            showErrorMessage(tr("Please, enter your password"));
            return;
        }

        if (megaApi->getPasswordStrength(password.toUtf8().constData()) == MegaApi::PASSWORD_STRENGTH_VERYWEAK)
        {
            showErrorMessage(tr("Please, enter a stronger password"));
            return;
        }

        if (password.compare(repeatPassword))
        {
            showErrorMessage(tr("The entered passwords don't match"));
            return;
        }

        if (!ui->cAgreeWithTerms->isChecked())
        {
            showErrorMessage(tr("You have to accept our terms of service"));
            return;
        }

        megaApi->createAccount(email.toUtf8().constData(),
                               password.toUtf8().constData(),
                               name.toUtf8().constData(),
                               lastName.toUtf8().constData());

        ui->lProgress->setText(tr("Creating account..."));
        page_progress();
    }
    else if (w == ui->pSetupType)
    {
        QString defaultFolderPath = Utilities::getDefaultBasePath();
        if (ui->rAdvancedSetup->isChecked())
        {
            defaultFolderPath.append(QString::fromUtf8("/MEGAsync"));
            ui->eMegaFolder->setText(QString::fromUtf8("/MEGAsync"));
            ui->lAdvancedSetup->setText(tr("Selective sync:"));
            ui->lHeader->setText(tr("Set up selective sync"));
            ui->bSyncType->setIcon(QIcon(QString::fromAscii("://images/step_4_selective_sync.png")));
            ui->bSyncType->setIconSize(QSize(94, 94));
            ui->lSyncType->setText(tr("Selective sync"));
            ui->lSyncTypeDesc->setText(tr("Specific folders in your Cloud Drive will be synchronized with a local folder."));
            ui->bMegaFolder->show();
            ui->eMegaFolder->show();
            ui->lMegaFolder->show();
            //TODO: Review usage of lAdditionalSyncs when new onboarding wizard is completed
            ui->lAdditionalSyncs->setText(QString::fromUtf8(""));
            ui->lAdditionalSyncs->show();
        }
        else
        {
            defaultFolderPath.append(QString::fromUtf8("/MEGA"));
            ui->eMegaFolder->setText(QString::fromUtf8("/"));
            ui->lAdvancedSetup->setText(tr("Select Local folder:"));
            ui->lHeader->setText(tr("Set up full sync"));
            ui->bSyncType->setIcon(QIcon(QString::fromAscii("://images/step_4_full_sync.png")));
            ui->bSyncType->setIconSize(QSize(94, 94));
            ui->lSyncType->setText(tr("Full Sync"));
            ui->lSyncTypeDesc->setText(tr("Your entire Cloud Drive will be synchronized with a local folder."));
            ui->bMegaFolder->hide();
            ui->eMegaFolder->hide();
            ui->lMegaFolder->hide();
            ui->lAdditionalSyncs->setText(QString::fromUtf8(""));
            ui->lAdditionalSyncs->show();
        }

        ui->bBack->setVisible(true);
        ui->bBack->setEnabled(true);

        if (!loggingStarted) //Logging started at main dialog
        {
            ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/login_setup_step2.png")));
            ui->bCurrentStep->setIconSize(QSize(512, 44));
        }
        else
        {
            ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/setup_step4.png")));
            ui->bCurrentStep->setIconSize(QSize(512, 44));
        }

        ui->wHelp->hide();
        ui->bNext->show();
        ui->sPages->setCurrentWidget(ui->pAdvanced);

        defaultFolderPath = QDir::toNativeSeparators(defaultFolderPath);
        QDir defaultFolder(defaultFolderPath);
        defaultFolder.mkpath(QString::fromUtf8("."));
        ui->eLocalFolder->setText(defaultFolderPath);
    }
    else if (w == ui->pAdvanced)
    {
        if (!ui->eLocalFolder->text().length())
        {
            showErrorMessage(tr("Please, select a local folder"));
            return;
        }

        if (!ui->eMegaFolder->text().length())
        {
            showErrorMessage(tr("Please, select a MEGA folder"));
            return;
        }

        QString localFolderPath = ui->eLocalFolder->text();
        QString warningMessage;
        auto syncability (SyncController::isLocalFolderAllowedForSync(localFolderPath, MegaSync::TYPE_TWOWAY, warningMessage));
        if (syncability != SyncController::CANT_SYNC)
        {
            syncability = SyncController::areLocalFolderAccessRightsOk(localFolderPath, MegaSync::TYPE_TWOWAY, warningMessage);
        }

        // If OK, check that we can sync the selected remote folder
        QString remoteFolderPath = ui->eMegaFolder->text();
        std::shared_ptr<MegaNode> node (megaApi->getNodeByPath(remoteFolderPath.toUtf8().constData()));

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
                    page_login();
                    QMegaMessageBox::warning(nullptr, tr("Error"), tr("Unable to get the filesystem.\n"
                                                                      "Please, try again. If the problem persists "
                                                                      "please contact bug@mega.co.nz"));
                    done(QDialog::Rejected);
                    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !rootNode (SetupWizard - next)");
                preferences->setCrashed(true);
                    app->rebootApplication(false);
                }
                else
                {
                    ui->eMegaFolder->setText(QString::fromLatin1("/MEGAsync"));
                    megaApi->createFolder("MEGAsync", rootNode.get());
                    creatingDefaultSyncFolder = true;

                    ui->lProgress->setText(tr("Creating folder…"));
                    page_progress();
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
            selectedMegaFolderHandle = node->getHandle();
            page_welcome();
        }
    }
}

void SetupWizard::on_bBack_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if (w == ui->pSetupType)
    {
        megaApi->logout(true, nullptr);
        page_logout();
    }
    else if (w == ui->pAdvanced)
    {
        page_mode();
    }
    else if (w == ui->pLogin)
    {
        page_newaccount();
    }
}

void SetupWizard::on_bCancel_clicked()
{
    if (ui->sPages->currentWidget() == ui->pWelcome)
    {
        setupPreferences();
        auto rootNode = ((MegaApplication*)qApp)->getRootNode();

        if (!rootNode)
        {
            page_login();
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("Unable to get the filesystem.\n"
                                "Please, try again. If the problem persists "
                                "please contact bug@mega.co.nz"));

            done(QDialog::Rejected);
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !rootNode (SetupWizard - cancel)");
            preferences->setCrashed(true);
            app->rebootApplication(false);
            return;
        }

        QString localPath (QDir::toNativeSeparators(QDir(ui->eLocalFolder->text()).canonicalPath()));
        QString syncName (SyncController::getSyncNameFromPath(localPath));
        mPreconfiguredSyncs.append(PreConfiguredSync(localPath, selectedMegaFolderHandle, syncName));
        done(QDialog::Accepted);
    }
    else
    {
        if (closing)
        {
            megaApi->localLogout();
            return;
        }

        auto button = QMegaMessageBox::question(this, tr("MEGAsync"), tr("Are you sure you want to cancel this wizard and undo all changes?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

        if (button == QMessageBox::Yes)
        {
            loggingStarted = false;

            if (megaApi->isLoggedIn())
            {
                closing = true;
                megaApi->logout(true, nullptr);
                page_logout();
            }
            else
            {
                megaApi->localLogout();
                done(QDialog::Rejected);
            }
        }
    }
}

void SetupWizard::on_bSkip_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if (w == ui->pSetupType || w == ui->pAdvanced)
    {
        setupPreferences();
    }

    if (w == ui->pNewAccount)
    {
        app->showInfoDialog();
        done(QDialog::Rejected);
    }
    else
    {
        done(QDialog::Accepted);
    }
}

void SetupWizard::on_bLocalFolder_clicked()
{
    QString defaultPath = ui->eLocalFolder->text().trimmed();
    if (!defaultPath.size())
    {
        defaultPath = Utilities::getDefaultBasePath();
    }

    defaultPath = QDir::toNativeSeparators(defaultPath);

    Platform::getInstance()->folderSelector(tr("Select local folder"), defaultPath, false, this, [this](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            onLocalFolderSet(fPath);
        }
    });
}

void SetupWizard::onLocalFolderSet(const QString& path)
{
    QDir dir(path);
    if (dir.mkpath(QString::fromLatin1(".")))
    {
        ui->eLocalFolder->setText(path);
    }
}

void SetupWizard::on_bMegaFolder_clicked()
{
    QPointer<SyncNodeSelector> nodeSelector = new SyncNodeSelector(this);
    std::shared_ptr<MegaNode> defaultNode(megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData()));
    nodeSelector->setSelectedNodeHandle(defaultNode);
#ifdef Q_OS_LINUX
    nodeSelector->setWindowFlags(nodeSelector->windowFlags() | (Qt::Tool));
#endif
    DialogOpener::showDialog<SyncNodeSelector>(nodeSelector, [nodeSelector, this](){
        if (nodeSelector->result() == QDialog::Accepted)
        {
            selectedMegaFolderHandle = nodeSelector->getSelectedNodeHandle();
            std::shared_ptr<MegaNode> node(megaApi->getNodeByHandle(selectedMegaFolderHandle));
            if (node)
            {
                std::unique_ptr<const char[]> nPath(megaApi->getNodePath(node.get()));
                if (nPath)
                {
                    ui->eMegaFolder->setText(QString::fromUtf8(nPath.get()));
                }
            }}
    });
}

void SetupWizard::initModeSelection()
{
    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif
    if (ratio < 2)
    {
        ui->wTypicalSetup->setStyleSheet(QString::fromUtf8("#wTypicalSetup { border-image: url(\":/images/select_sync_bt.png\"); }"));
        ui->wAdvancedSetup->setStyleSheet(QString::fromUtf8("#wAdvancedSetup { border-image: url(\":/images/select_sync_bt.png\"); }"));
    }
    else
    {
        ui->wTypicalSetup->setStyleSheet(QString::fromUtf8("#wTypicalSetup { border-image: url(\":/images/select_sync_bt@2x.png\"); }"));
        ui->wAdvancedSetup->setStyleSheet(QString::fromUtf8("#wAdvancedSetup { border-image: url(\":/images/select_sync_bt@2x.png\"); }"));
    }

    ui->rTypicalSetup->setChecked(false);
    ui->rAdvancedSetup->setChecked(false);
    repaint();
}

void SetupWizard::wTypicalSetup_clicked()
{
    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif
    if (ratio < 2)
    {
        ui->wTypicalSetup->setStyleSheet(QString::fromUtf8("#wTypicalSetup { border-image: url(\":/images/selected_sync_bt.png\"); }"));
        ui->wAdvancedSetup->setStyleSheet(QString::fromUtf8("#wAdvancedSetup { border-image: url(\":/images/select_sync_bt.png\"); }"));
    }
    else
    {
        ui->wTypicalSetup->setStyleSheet(QString::fromUtf8("#wTypicalSetup { border-image: url(\":/images/selected_sync_bt@2x.png\"); }"));
        ui->wAdvancedSetup->setStyleSheet(QString::fromUtf8("#wAdvancedSetup { border-image: url(\":/images/select_sync_bt@2x.png\"); }"));
    }

    ui->rTypicalSetup->setChecked(true);
    ui->rAdvancedSetup->setChecked(false);
    ui->bNext->setEnabled(true);
    ui->bNext->setDefault(true);
    ui->bNext->setFocus();
    repaint();
}

void SetupWizard::wAdvancedSetup_clicked()
{
    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif
    if (ratio < 2)
    {
        ui->wTypicalSetup->setStyleSheet(QString::fromUtf8("#wTypicalSetup { border-image: url(\":/images/select_sync_bt.png\"); }"));
        ui->wAdvancedSetup->setStyleSheet(QString::fromUtf8("#wAdvancedSetup { border-image: url(\":/images/selected_sync_bt.png\"); }"));
    }
    else
    {
        ui->wTypicalSetup->setStyleSheet(QString::fromUtf8("#wTypicalSetup { border-image: url(\":/images/select_sync_bt@2x.png\"); }"));
        ui->wAdvancedSetup->setStyleSheet(QString::fromUtf8("#wAdvancedSetup { border-image: url(\":/images/selected_sync_bt@2x.png\"); }"));
    }

    ui->rTypicalSetup->setChecked(false);
    ui->rAdvancedSetup->setChecked(true);
    ui->bNext->setEnabled(true);
    ui->bNext->setDefault(true);
    ui->bNext->setFocus();
    repaint();
}

void SetupWizard::setupPreferences()
{
    std::unique_ptr<char[]> email(megaApi->getMyEmail());
    preferences->setEmailAndGeneralSettings(QString::fromUtf8(email.get()));
}

bool SetupWizard::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->wTypicalSetup)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* key = static_cast<QKeyEvent*>(event);
            if (key->key() == Qt::Key_Space)
            {
                wTypicalSetup_clicked();
            }
        }

        if (event->type() == QEvent::MouseButtonPress)
        {
            wTypicalSetup_clicked();
        }
    }
    else if (obj == ui->wAdvancedSetup)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* key = static_cast<QKeyEvent*>(event);
            if (key->key() == Qt::Key_Space)
            {
                wAdvancedSetup_clicked();
            }
        }

        if (event->type() == QEvent::MouseButtonPress)
        {
            wAdvancedSetup_clicked();
        }
    }
    else if (obj == ui->lTermsLink)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            lTermsLink_clicked();
        }
    }
    return QObject::eventFilter(obj, event);
}

void SetupWizard::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous())
    {
        event->accept();
        return;
    }

    if (closing)
    {
        megaApi->localLogout();
        done(QDialog::Rejected);
        return;
    }

    if (closeBlocked)
    {
        event->ignore();
        return;
    }

    event->ignore();
    auto button = QMegaMessageBox::question(this, tr("MEGAsync"),tr("Are you sure you want to cancel this wizard and undo all changes?"),QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (button == QMessageBox::Yes)
    {
        if (megaApi->isLoggedIn())
        {
            closing = true;
            megaApi->logout(true, nullptr);
            page_logout();
        }
        else
        {
            megaApi->localLogout();
            done(QDialog::Rejected);
        }
    }
}

void SetupWizard::page_login()
{
    closeBlocked = false;
    ui->eLoginPassword->clear();
    ui->wHelp->hide();
    initModeSelection();

    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(true);
    ui->bNext->setEnabled(true);
    ui->bBack->setVisible(true);
    ui->bBack->setEnabled(true);
    ui->bSkip->setVisible(false);
    ui->bSkip->setEnabled(false);
    ui->eLoginEmail->setFocus();
    ui->bNext->setDefault(true);

    ui->lHeader->setText(tr("Log in to your MEGA account"));
    ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/setup_step2.png")));
    ui->bCurrentStep->setIconSize(QSize(512, 44));

    ui->sPages->setCurrentWidget(ui->pLogin);
    emit pageChanged(PAGE_LOGIN);
    sessionKey.clear();
}

void SetupWizard::page_logout()
{
    closeBlocked = false;
    ui->lProgress->setText(tr("Logging out..."));
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(-1);
    ui->wHelp->hide();

    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(false);
    ui->bNext->setEnabled(false);
    ui->bBack->setVisible(false);
    ui->bBack->setVisible(false);
    ui->bSkip->setVisible(false);
    ui->bSkip->setEnabled(false);
    ui->bCancel->setFocus();
    ui->bCancel->setDefault(true);

    ui->sPages->setCurrentWidget(ui->pProgress);
    emit pageChanged(PAGE_LOGOUT);
    sessionKey.clear();
}

void SetupWizard::page_mode()
{
    closeBlocked = false;
    initModeSelection();

    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(true);
    ui->bNext->setEnabled(false);

    ui->bSkip->setVisible(true);
    ui->bSkip->setEnabled(true);
    ui->bSkip->setText(tr("Skip"));
    ui->bNext->setDefault(false);
    ui->bCancel->setDefault(false);
    ui->wHelp->show();

    ui->lHeader->setText(tr("Choose install type"));

    if (!loggingStarted) //Logging started at main dialog
    {
        ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/login_setup_step1.png")));
        ui->bCurrentStep->setIconSize(QSize(512, 44));
        ui->bBack->setVisible(false);
        ui->bBack->setEnabled(false);
    }
    else
    {
        ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/setup_step3.png")));
        ui->bCurrentStep->setIconSize(QSize(512, 44));
        ui->bBack->setVisible(true);
        ui->bBack->setEnabled(true);
    }

    emit pageChanged(PAGE_MODE);
    ui->sPages->setCurrentWidget(ui->pSetupType);
}

void SetupWizard::page_welcome()
{
    closeBlocked = false;
    ui->wHelp->hide();
    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bCancel->setText(tr("Finish"));
    ui->bNext->setVisible(false);
    ui->bNext->setEnabled(false);
    ui->bBack->setVisible(false);
    ui->bBack->setVisible(false);
    ui->bSkip->setVisible(false);
    ui->bSkip->setEnabled(false);
    ui->bCancel->setFocus();
    ui->bCancel->setDefault(true);

    ui->lHeader->setText(tr("We are all done"));

    if (!loggingStarted) //Logging started at main dialog
    {
        ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/login_setup_step3.png")));
        ui->bCurrentStep->setIconSize(QSize(512, 44));
    }
    else
    {
        ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/setup_step5.png")));
        ui->bCurrentStep->setIconSize(QSize(512, 44));
    }

    ui->sPages->setCurrentWidget(ui->pWelcome);
    emit pageChanged(PAGE_MODE);
    ui->wButtons->hide();
    ui->bFinish->setFocus();
}

void SetupWizard::page_newaccount()
{
    closeBlocked = false;
    ui->eLoginPassword->clear();
    ui->eName->clear();
    ui->eLastName->clear();
    ui->eEmail->clear();
    ui->ePassword->clear();
    ui->eRepeatPassword->clear();
    ui->wHelp->hide();
    initModeSelection();
    sessionKey.clear();
    selectedMegaFolderHandle = mega::INVALID_HANDLE;

    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(true);
    ui->bNext->setEnabled(true);
    ui->bBack->setVisible(false);
    ui->bBack->setEnabled(false);
    ui->bSkip->setVisible(true);
    ui->bSkip->setEnabled(true);
    ui->bSkip->setText(tr("Log in"));
    ui->eName->setFocus();
    ui->bNext->setDefault(true);
    ui->cAgreeWithTerms->setChecked(false);

    ui->lHeader->setText(tr("Create a new MEGA account"));
    ui->bCurrentStep->setIcon(QIcon(QString::fromAscii("://images/setup_step1.png")));
    ui->bCurrentStep->setIconSize(QSize(512, 44));

    ui->sPages->setCurrentWidget(ui->pNewAccount);
    emit pageChanged(PAGE_NEW_ACCOUNT);
}

void SetupWizard::page_progress()
{
    closeBlocked = false;
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(-1);

    ui->bBack->setEnabled(false);
    ui->bNext->setEnabled(false);
    ui->bSkip->setEnabled(false);
    ui->bCancel->setVisible(true);
    ui->bCancel->setEnabled(true);
    ui->bCancel->setFocus();
    ui->bCancel->setDefault(true);

    ui->sPages->setCurrentWidget(ui->pProgress);
    emit pageChanged(PAGE_PROGRESS);
}

void SetupWizard::setLevelStrength(int level)
{
    switch (level)
    {
        case MegaApi::PASSWORD_STRENGTH_VERYWEAK:
            ui->wVeryWeak->setStyleSheet(QString::fromUtf8("background-color:#FD684C;"));
            ui->wWeak->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wMedium->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wGood->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wStrong->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            break;
        case MegaApi::PASSWORD_STRENGTH_WEAK:
            ui->wVeryWeak->setStyleSheet(QString::fromUtf8("background-color:#FFA500;"));
            ui->wWeak->setStyleSheet(QString::fromUtf8("background-color:#FFA500;"));
            ui->wMedium->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wGood->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wStrong->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            break;
        case MegaApi::PASSWORD_STRENGTH_MEDIUM:
            ui->wVeryWeak->setStyleSheet(QString::fromUtf8("background-color:#FFD300;"));
            ui->wWeak->setStyleSheet(QString::fromUtf8("background-color:#FFD300;"));
            ui->wMedium->setStyleSheet(QString::fromUtf8("background-color:#FFD300;"));
            ui->wGood->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wStrong->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            break;
        case MegaApi::PASSWORD_STRENGTH_GOOD:
            ui->wVeryWeak->setStyleSheet(QString::fromUtf8("background-color:#81D522;"));
            ui->wWeak->setStyleSheet(QString::fromUtf8("background-color:#81D522;"));
            ui->wMedium->setStyleSheet(QString::fromUtf8("background-color:#81D522;"));
            ui->wGood->setStyleSheet(QString::fromUtf8("background-color:#81D522;"));
            ui->wStrong->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            break;
        case MegaApi::PASSWORD_STRENGTH_STRONG:
            ui->wVeryWeak->setStyleSheet(QString::fromUtf8("background-color:#00BFA5;"));
            ui->wWeak->setStyleSheet(QString::fromUtf8("background-color:#00BFA5;"));
            ui->wMedium->setStyleSheet(QString::fromUtf8("background-color:#00BFA5;"));
            ui->wGood->setStyleSheet(QString::fromUtf8("background-color:#00BFA5;"));
            ui->wStrong->setStyleSheet(QString::fromUtf8("background-color:#00BFA5;"));
            break;
        default:
            ui->wVeryWeak->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wWeak->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wMedium->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wGood->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            ui->wStrong->setStyleSheet(QString::fromUtf8("background-color:rgba(0,0,0,0.2);"));
            break;
    }
}

QList<PreConfiguredSync> SetupWizard::preconfiguredSyncs() const
{
    return mPreconfiguredSyncs;
}

void SetupWizard::lTermsLink_clicked()
{
    ui->cAgreeWithTerms->toggle();
}

void SetupWizard::on_lTermsLink_linkActivated(const QString& /*link*/)
{
    Utilities::openUrl(QUrl(Preferences::BASE_URL + QString::fromUtf8("/terms")));
}

void SetupWizard::on_bLearMore_clicked()
{
    QString helpUrl = QString::fromAscii("https://help.mega.io/installs-apps/desktop-syncing");
    Utilities::openUrl(QUrl(helpUrl));
}

void SetupWizard::on_bFinish_clicked()
{
    on_bCancel_clicked();
}

void SetupWizard::showErrorMessage(QString error)
{
    animationTimer->stop();

    ui->lError->setText(error);
    ui->lError->hide();
    m_animation->setDirection(QAbstractAnimation::Forward);

    m_animation->start();
}

void SetupWizard::onErrorAnimationFinished()
{
    if (QAbstractAnimation::Forward == m_animation->direction())
    {
        m_animation->setDirection(QAbstractAnimation::Backward);
        ui->lError->show();
        animationTimer->start(5000);
    }
    else
    {
        m_animation->setDirection(QAbstractAnimation::Forward);
    }
}

void SetupWizard::animationTimout()
{
    ui->lError->setText(QString::fromUtf8(""));
    ui->lError->hide();
    m_animation->start();
}

void SetupWizard::onPasswordTextChanged(QString text)
{
    int strength = megaApi->getPasswordStrength(text.toUtf8().constData());
    text.isEmpty() ? setLevelStrength(-1) : setLevelStrength(strength);
}

PreConfiguredSync::PreConfiguredSync(QString localFolder, MegaHandle megaFolderHandle, QString syncName):
    mMegaFolderHandle(megaFolderHandle), mLocalFolder(localFolder), mSyncName(syncName)
{

}

QString PreConfiguredSync::localFolder() const
{
    return mLocalFolder;
}

QString PreConfiguredSync::syncName() const
{
    return mSyncName;
}

mega::MegaHandle PreConfiguredSync::megaFolderHandle() const
{
    return mMegaFolderHandle;
}
