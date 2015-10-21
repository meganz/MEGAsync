#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include "MegaApplication.h"
#include "control/Utilities.h"

using namespace mega;

SetupWizard::SetupWizard(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupWizard)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::WindowModal);

    ui->wAdvancedSetup->installEventFilter(this);
    ui->wTypicalSetup->installEventFilter(this);
    ui->lTermsLink->installEventFilter(this);
    ui->rTypicalSetup->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->rAdvancedSetup->setAttribute(Qt::WA_TransparentForMouseEvents);

    this->app = app;
    this->closing = false;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    delegateListener = new QTMegaRequestListener(megaApi, this);

    ui->lTermsLink->setText(ui->lTermsLink->text().replace(
        QString::fromUtf8("\">"),
        QString::fromUtf8("\" style=\"color:#DC0000\">"))
        .replace(QString::fromUtf8("mega.co.nz"), QString::fromUtf8("mega.nz")));

    page_initial();
}

SetupWizard::~SetupWizard()
{
    delete delegateListener;
    delete ui;
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

    ui->bBack->setEnabled(true);
    ui->bNext->setEnabled(true);
    ui->bSkip->setEnabled(true);
    switch (request->getType())
	{
		case MegaRequest::TYPE_CREATE_ACCOUNT:
		{
            if (error->getErrorCode() == MegaError::API_OK)
			{
                page_login();

                ui->eLoginEmail->setText(ui->eEmail->text().toLower().trimmed());
				ui->lVerify->setVisible(true);
                ui->eName->clear();
                ui->eEmail->clear();
                ui->ePassword->clear();
                ui->eRepeatPassword->clear();

                megaApi->sendEvent(99505, "MEGAsync account creation start");
                if (!preferences->accountCreationTime())
                {
                    preferences->setAccountCreationTime(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                }
                break;
			}

            page_newaccount();

            if (error->getErrorCode() == MegaError::API_EEXIST)
			{
				QMessageBox::warning(this, tr("Error"), tr("User already exists"), QMessageBox::Ok);
			}
            else if (error->getErrorCode() != MegaError::API_ESSL)
			{
                QMessageBox::warning(this, tr("Error"), QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
			}
			break;
		}
		case MegaRequest::TYPE_LOGIN:
		{
            if (error->getErrorCode() == MegaError::API_OK)
			{
                megaApi->fetchNodes(delegateListener);
                ui->lProgress->setText(tr("Fetching file list..."));
                page_progress();

                if (!preferences->hasLoggedIn())
                {
                    preferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                }
                break;
			}

            page_login();

            if (error->getErrorCode() == MegaError::API_ENOENT)
			{
                QMessageBox::warning(this, tr("Error"), tr("Incorrect email and/or password.") + QString::fromUtf8(" ") + tr("Have you verified your account?"), QMessageBox::Ok);
			}
            else if (error->getErrorCode() == MegaError::API_EBLOCKED)
            {
                QMessageBox::critical(NULL, tr("Error"), tr("Your account has been blocked. Please contact support@mega.co.nz"));
            }
            else if (error->getErrorCode() != MegaError::API_ESSL)
			{
                QMessageBox::warning(this, tr("Error"), QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
			}
			break;
		}
        case MegaRequest::TYPE_CREATE_FOLDER:
		{
            if (error->getErrorCode() == MegaError::API_OK)
            {
                MegaNode *node = megaApi->getNodeByPath("/MEGAsync");
                if (!node)
                {
                    QMessageBox::warning(this, tr("Error"), tr("MEGA folder doesn't exist"), QMessageBox::Ok);
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
                QMessageBox::warning(this, tr("Error"),  QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
            }

            break;
		}
        case MegaRequest::TYPE_FETCH_NODES:
		{
            if (error->getErrorCode() != MegaError::API_OK)
            {
                page_login();
                QMessageBox::warning(this, tr("Error"), QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
                break;
            }

            MegaNode *root = megaApi->getRootNode();
            if (!root)
            {
                page_login();
                QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                           "Please, try again. If the problem persists "
                                                           "please contact bug@mega.co.nz"), QMessageBox::Ok);
                done(QDialog::Rejected);
                preferences->setCrashed(true);
                app->rebootApplication(false);
                return;
            }
            delete root;

            QString email = ui->eLoginEmail->text().toLower().trimmed();
            if (preferences->hasEmail(email))
            {
                int proxyType = preferences->proxyType();
                QString proxyServer = preferences->proxyServer();
                int proxyPort = preferences->proxyPort();
                int proxyProtocol = preferences->proxyProtocol();
                bool proxyAuth = preferences->proxyRequiresAuth();
                QString proxyUsername = preferences->getProxyUsername();
                QString proxyPassword = preferences->getProxyPassword();
                QString downloadFolder = preferences->downloadFolder();

                preferences->setEmail(email);
                preferences->setSession(sessionKey);
                preferences->setProxyType(proxyType);
                preferences->setProxyServer(proxyServer);
                preferences->setProxyPort(proxyPort);
                preferences->setProxyProtocol(proxyProtocol);
                preferences->setProxyRequiresAuth(proxyAuth);
                preferences->setProxyUsername(proxyUsername);
                preferences->setProxyPassword(proxyPassword);
                preferences->setDownloadFolder(downloadFolder);

                done(QDialog::Accepted);
                break;
            }

            page_mode();
			break;
		}
        case MegaRequest::TYPE_LOGOUT:
        {
            page_login();
            break;
        }
    }
}

void SetupWizard::onRequestUpdate(MegaApi *, MegaRequest *request)
{
    if (request->getType() == MegaRequest::TYPE_FETCH_NODES)
    {
        if (request->getTotalBytes() > 0)
        {
            ui->progressBar->setMaximum(request->getTotalBytes());
            ui->progressBar->setValue(request->getTransferredBytes());
        }
    }
}

void SetupWizard::goToStep(int page)
{
    switch (page)
    {
        case PAGE_INITIAL:
            page_initial();
            break;

        case PAGE_LOGIN:
            page_login();
            break;

        case PAGE_NEW_ACCOUNT:
            page_newaccount();
            break;
        default:
            return;
    }
}

void SetupWizard::on_bNext_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if (w == ui->pSetup)
    {
        if (ui->rHaveAccount->isChecked())
        {
            page_login();
        }
        else
        {
            page_newaccount();
        }
    }
    else if (w == ui->pLogin)
    {
        QString email = ui->eLoginEmail->text().toLower().trimmed();
        QString password = ui->eLoginPassword->text();

        if (!email.length())
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your e-mail address"), QMessageBox::Ok);
            return;
        }

        if (!email.contains(QChar::fromAscii('@')) || !email.contains(QChar::fromAscii('.')))
        {
           QMessageBox::warning(this, tr("Error"), tr("Please, enter a valid e-mail address"), QMessageBox::Ok);
           return;
        }

        if (!password.length())
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your password"), QMessageBox::Ok);
            return;
        }

		megaApi->login(email.toUtf8().constData(), password.toUtf8().constData(), delegateListener);

        ui->lProgress->setText(tr("Logging in..."));
        page_progress();
    }
    else if (w == ui->pNewAccount)
    {
        QString name = ui->eName->text().trimmed();
        QString email = ui->eEmail->text().toLower().trimmed();
        QString password = ui->ePassword->text();
        QString repeatPassword = ui->eRepeatPassword->text();

        if (!name.length())
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your name"), QMessageBox::Ok);
            return;
        }

        if (!email.length())
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your e-mail address"), QMessageBox::Ok);
            return;
        }

        if (!email.contains(QChar::fromAscii('@')) || !email.contains(QChar::fromAscii('.')))
        {
           QMessageBox::warning(this, tr("Error"), tr("Please, enter a valid e-mail address"), QMessageBox::Ok);
           return;
        }

        if (!password.length())
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your password"), QMessageBox::Ok);
            return;
        }

        if (password.length() < 8)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter a stronger password"), QMessageBox::Ok);
            return;
        }

        if (password.compare(repeatPassword))
        {
            QMessageBox::warning(this, tr("Error"), tr("The entered passwords don't match"), QMessageBox::Ok);
            return;
        }

        if (!ui->cAgreeWithTerms->isChecked())
        {
            QMessageBox::warning(this, tr("Error"), tr("You have to accept our terms of service"), QMessageBox::Ok);
            return;
        }

        megaApi->createAccount(email.toUtf8().constData(),
                                         password.toUtf8().constData(),
										 name.toUtf8().constData(), delegateListener);

        ui->lProgress->setText(tr("Creating account..."));
        page_progress();
    }
    else if (w == ui->pSetupType)
    {
        #ifdef WIN32
            #if QT_VERSION < 0x050000
                QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
            #else
                QString defaultFolderPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
            #endif
        #else
            #if QT_VERSION < 0x050000
                QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
            #else
                QString defaultFolderPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
            #endif
        #endif

        if (ui->rAdvancedSetup->isChecked())
        {
            defaultFolderPath.append(QString::fromUtf8("/MEGAsync"));
            ui->eMegaFolder->setText(QString::fromUtf8("/MEGAsync"));
            ui->lAdvancedLabel->setText(tr("The following folders will be automatically synchronized:"));
            ui->lAdvancedSetup->setText(tr("Selective sync"));
            ui->bMegaFolder->show();
            ui->eMegaFolder->show();
            ui->lMegaFolder->show();
        }
        else
        {
            defaultFolderPath.append(QString::fromUtf8("/MEGA"));
            ui->eMegaFolder->setText(QString::fromUtf8("/"));
            ui->lAdvancedLabel->setText(tr("Your Cloud Drive will be synchronized with this folder:"));
            ui->lAdvancedSetup->setText(tr("Full sync"));
            ui->bMegaFolder->hide();
            ui->eMegaFolder->hide();
            ui->lMegaFolder->hide();
        }

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
            QMessageBox::warning(this, tr("Warning"), tr("Please, select a local folder"), QMessageBox::Ok);
            return;
        }

        if (!ui->eMegaFolder->text().length())
        {
            QMessageBox::warning(this, tr("Warning"), tr("Please, select a MEGA folder"), QMessageBox::Ok);
            return;
        }

        QString localFolderPath = ui->eLocalFolder->text();
        if (!Utilities::verifySyncedFolderLimits(localFolderPath))
        {
            QMessageBox::warning(this, tr("Warning"), tr("You are trying to sync an extremely large folder.\nTo prevent the syncing of entire boot volumes, which is inefficient and dangerous,\nwe ask you to start with a smaller folder and add more data while MEGAsync is running."), QMessageBox::Ok);
            return;
        }

        MegaNode *node = megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData());
        if (!node)
        {
            MegaNode *rootNode = megaApi->getRootNode();
            if (!rootNode)
            {
                page_login();
                QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                           "Please, try again. If the problem persists "
                                                           "please contact bug@mega.co.nz"), QMessageBox::Ok);
                done(QDialog::Rejected);
                preferences->setCrashed(true);
                app->rebootApplication(false);
                return;
            }

            ui->eMegaFolder->setText(QString::fromUtf8("/MEGAsync"));
            megaApi->createFolder("MEGAsync", rootNode, delegateListener);
            delete rootNode;

            ui->lProgress->setText(tr("Creating folder..."));
            page_progress();
        }
        else
        {
            selectedMegaFolderHandle = node->getHandle();
            page_welcome();
            delete node;
        }
    }
}

void SetupWizard::on_bBack_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if (w == ui->pLogin || w == ui->pNewAccount)
    {
        page_initial();
    }
    else if (w == ui->pSetupType)
    {
        page_logout();
    }
    else if (w == ui->pAdvanced)
    {
        page_mode();
    }
}

void SetupWizard::on_bCancel_clicked()
{
    if (ui->sPages->currentWidget() == ui->pWelcome)
    {
        setupPreferences();
        QString syncName;
        MegaNode *rootNode = megaApi->getRootNode();
        if (!rootNode)
        {
            page_login();
            QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                       "Please, try again. If the problem persists "
                                                       "please contact bug@mega.co.nz"), QMessageBox::Ok);
            done(QDialog::Rejected);
            preferences->setCrashed(true);
            app->rebootApplication(false);
            return;
        }

        if (selectedMegaFolderHandle == rootNode->getHandle())
        {
            syncName = QString::fromUtf8("MEGA");
        }

        delete rootNode;

        preferences->addSyncedFolder(ui->eLocalFolder->text(), ui->eMegaFolder->text(), selectedMegaFolderHandle, syncName);
        done(QDialog::Accepted);
    }
    else
    {
        if (closing)
        {
            megaApi->localLogout(delegateListener);
            return;
        }

        QPointer<QMessageBox> msg = new QMessageBox(this);
        msg->setIcon(QMessageBox::Question);
        msg->setWindowTitle(tr("MEGAsync"));
        msg->setText(tr("Are you sure you want to cancel this wizard and undo all changes?"));
        msg->addButton(QMessageBox::Yes);
        msg->addButton(QMessageBox::No);
        msg->setDefaultButton(QMessageBox::No);
        int button = msg->exec();
        if (msg)
        {
            delete msg;
        }

        if (button == QMessageBox::Yes)
        {
            if (megaApi->isLoggedIn())
            {
                closing = true;
                page_logout();
            }
            else
            {
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
    done(QDialog::Accepted);
}

void SetupWizard::on_bLocalFolder_clicked()
{	
    QString defaultPath = ui->eLocalFolder->text().trimmed();
    if (!defaultPath.size())
    {
        #ifdef WIN32
            #if QT_VERSION < 0x050000
                defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
            #else
                defaultPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
            #endif
        #else
            #if QT_VERSION < 0x050000
                defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
            #else
                defaultPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
            #endif
        #endif
    }
    QString path =  QFileDialog::getExistingDirectory(0, tr("Select local folder"),
                                                      defaultPath,
                                                      QFileDialog::ShowDirsOnly
													  | QFileDialog::DontResolveSymlinks);
    if (path.length())
    {
        QDir dir(path);
        if (!dir.exists() && !dir.mkpath(QString::fromUtf8(".")))
        {
            return;
        }

        QTemporaryFile test(path + QDir::separator());
        if (test.open() ||
                QMessageBox::warning(window(), tr("Warning"), tr("You don't have write permissions in this local folder.") +
                    QString::fromUtf8("\n") + tr("MEGAsync won't be able to download anything here.") + QString::fromUtf8("\n") + tr("Do you want to continue?"),
                    QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
            ui->eLocalFolder->setText(path);
        }
    }
}

void SetupWizard::on_bMegaFolder_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(megaApi, NodeSelector::SYNC_SELECT, this);
    int result = nodeSelector->exec();
    if (!nodeSelector || result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *node = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if (!node)
    {
        delete nodeSelector;
        return;
    }

    const char *nPath = megaApi->getNodePath(node);
    if (!nPath)
    {
        delete nodeSelector;
        delete node;
        return;
    }

    ui->eMegaFolder->setText(QString::fromUtf8(nPath));
    delete nodeSelector;
    delete [] nPath;
    delete node;
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
    repaint();
}

void SetupWizard::setupPreferences()
{
    QString email = ui->eLoginEmail->text().toLower().trimmed();

    int proxyType = preferences->proxyType();
    QString proxyServer = preferences->proxyServer();
    int proxyPort = preferences->proxyPort();
    int proxyProtocol = preferences->proxyProtocol();
    bool proxyAuth = preferences->proxyRequiresAuth();
    QString proxyUsername = preferences->getProxyUsername();
    QString proxyPassword = preferences->getProxyPassword();
    preferences->setEmail(email);
    preferences->setSession(sessionKey);

    preferences->setProxyType(proxyType);
    preferences->setProxyServer(proxyServer);
    preferences->setProxyPort(proxyPort);
    preferences->setProxyProtocol(proxyProtocol);
    preferences->setProxyRequiresAuth(proxyAuth);
    preferences->setProxyUsername(proxyUsername);
    preferences->setProxyPassword(proxyPassword);
}

bool SetupWizard::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (obj == ui->wTypicalSetup)
        {
            wTypicalSetup_clicked();
        }
        else if (obj == ui->wAdvancedSetup)
        {
            wAdvancedSetup_clicked();
        }
        else if (obj == ui->lTermsLink)
        {
            lTermsLink_clicked();
        }
    }
    return QObject::eventFilter(obj, event);
}

void SetupWizard::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || closing)
    {
        event->accept();
        return;
    }

    event->ignore();
    QPointer<QMessageBox> msg = new QMessageBox(this);
    msg->setIcon(QMessageBox::Question);
    msg->setWindowTitle(tr("MEGAsync"));
    msg->setText(tr("Are you sure you want to cancel this wizard and undo all changes?"));
    msg->addButton(QMessageBox::Yes);
    msg->addButton(QMessageBox::No);
    msg->setDefaultButton(QMessageBox::No);
    int button = msg->exec();
    if (msg)
    {
        delete msg;
    }

    if (button == QMessageBox::Yes)
    {
        if (megaApi->isLoggedIn())
        {
            closing = true;
            page_logout();
        }
        else
        {
            done(QDialog::Rejected);
        }
    }
}

void SetupWizard::page_login()
{
    ui->eLoginPassword->clear();
    ui->lVerify->hide();
    wTypicalSetup_clicked();

    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(true);
    ui->bNext->setEnabled(true);
    ui->bBack->setVisible(true);
    ui->bBack->setVisible(true);
    ui->bSkip->setVisible(false);
    ui->bSkip->setEnabled(false);
    ui->eLoginEmail->setFocus();
    ui->bNext->setDefault(true);

    ui->sPages->setCurrentWidget(ui->pLogin);
    sessionKey.clear();
}

void SetupWizard::page_logout()
{
    megaApi->logout(delegateListener);
    ui->lProgress->setText(tr("Logging out..."));
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(-1);

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
    sessionKey.clear();
}

void SetupWizard::page_initial()
{
    ui->eLoginPassword->clear();
    ui->lVerify->hide();
    ui->eName->clear();
    ui->eEmail->clear();
    ui->ePassword->clear();
    ui->eRepeatPassword->clear();
    wTypicalSetup_clicked();

    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(true);
    ui->bNext->setEnabled(true);
    ui->bBack->setVisible(false);
    ui->bBack->setVisible(false);
    ui->bSkip->setVisible(false);
    ui->bSkip->setEnabled(false);
    ui->rHaveAccount->setChecked(true);
    ui->rDontHaveAccount->setChecked(false);
    ui->rHaveAccount->setFocus();
    ui->bNext->setDefault(true);

    ui->sPages->setCurrentWidget(ui->pSetup);
    sessionKey.clear();

    selectedMegaFolderHandle = mega::INVALID_HANDLE;
}

void SetupWizard::page_mode()
{
    wTypicalSetup_clicked();

    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(true);
    ui->bNext->setEnabled(true);
    ui->bBack->setVisible(true);
    ui->bBack->setVisible(true);
    ui->bSkip->setVisible(true);
    ui->bSkip->setEnabled(true);
    ui->bNext->setFocus();
    ui->bNext->setDefault(true);

    ui->sPages->setCurrentWidget(ui->pSetupType);
}

void SetupWizard::page_welcome()
{
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

    ui->sPages->setCurrentWidget(ui->pWelcome);
}

void SetupWizard::page_newaccount()
{
    ui->bCancel->setEnabled(true);
    ui->bCancel->setVisible(true);
    ui->bNext->setVisible(true);
    ui->bNext->setEnabled(true);
    ui->bBack->setVisible(true);
    ui->bBack->setVisible(true);
    ui->bSkip->setVisible(false);
    ui->bSkip->setEnabled(false);
    ui->eName->setFocus();
    ui->bNext->setDefault(true);

    ui->sPages->setCurrentWidget(ui->pNewAccount);
}

void SetupWizard::page_progress()
{
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
}

void SetupWizard::lTermsLink_clicked()
{
    ui->cAgreeWithTerms->toggle();
}
