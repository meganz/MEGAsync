#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include "MegaApplication.h"
#include "control/Utilities.h"

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

    ui->sPages->setCurrentWidget(ui->pSetup);
    ui->bBack->setVisible(false);
    this->app = app;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    selectedMegaFolderHandle = mega::UNDEF;
    ui->bNext->setFocus();
    delegateListener = new QTMegaRequestListener(megaApi, this);

    ui->bNext->setDefault(true);
    ui->lTermsLink->setText(ui->lTermsLink->text().replace(
        QString::fromUtf8("\">"),
        QString::fromUtf8("\" style=\"color:#DC0000\">")));

#ifdef __APPLE__
    setWindowTitle(tr("Setup Assistant - MEGAsync"));

    ((QBoxLayout *)ui->wButtons->layout())->removeWidget(ui->bCancel);
    ((QBoxLayout *)ui->wButtons->layout())->insertWidget(1, ui->bCancel);
#endif
}

SetupWizard::~SetupWizard()
{
    delete delegateListener;
    delete ui;
}

void SetupWizard::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *error)
{
	switch(request->getType())
	{
		case MegaRequest::TYPE_CREATE_ACCOUNT:
		{
			ui->bBack->setEnabled(true);
			ui->bNext->setEnabled(true);
			if(error->getErrorCode() == MegaError::API_OK)
			{
				ui->sPages->setCurrentWidget(ui->pLogin);
                ui->eLoginEmail->setText(ui->eEmail->text().toLower().trimmed());
				ui->lVerify->setVisible(true);
                ui->eName->clear();
                ui->eEmail->clear();
                ui->ePassword->clear();
                ui->eRepeatPassword->clear();
			}
			else if (error->getErrorCode() == MegaError::API_EEXIST)
			{
				ui->sPages->setCurrentWidget(ui->pNewAccount);
				QMessageBox::warning(this, tr("Error"), tr("User already exists"), QMessageBox::Ok);
			}
			else
			{
				ui->sPages->setCurrentWidget(ui->pNewAccount);
                QMessageBox::warning(this, tr("Error"), error->QgetErrorString(), QMessageBox::Ok);
			}
			break;
		}
		case MegaRequest::TYPE_LOGIN:
        case MegaRequest::TYPE_FAST_LOGIN:
		{
			if(error->getErrorCode() == MegaError::API_OK)
			{
                const char *session = megaApi->dumpSession();
                if(session)
                {
                    sessionKey = QString::fromUtf8(session);
                    delete session;
                    ui->lProgress->setText(tr("Fetching file list..."));
                    megaApi->fetchNodes(delegateListener);
                }
                else
                {
                    ui->bBack->setEnabled(true);
                    ui->bNext->setEnabled(true);
                    ui->sPages->setCurrentWidget(ui->pLogin);
                    QMessageBox::warning(this, tr("Error"), tr("Error getting session key"), QMessageBox::Ok);
                }
			}
			else if(error->getErrorCode() == MegaError::API_ENOENT)
			{
				ui->bBack->setEnabled(true);
				ui->bNext->setEnabled(true);
				ui->sPages->setCurrentWidget(ui->pLogin);
                QMessageBox::warning(this, tr("Error"), tr("Incorrect email and/or password.") + QString::fromAscii(" ") + tr("Have you verified your account?"), QMessageBox::Ok);
			}
			else
			{
				ui->bBack->setEnabled(true);
				ui->bNext->setEnabled(true);
				ui->sPages->setCurrentWidget(ui->pLogin);
                QMessageBox::warning(this, tr("Error"), error->QgetErrorString(), QMessageBox::Ok);
			}
			break;
		}
		case MegaRequest::TYPE_MKDIR:
		{
           ui->bNext->setEnabled(true);
           ui->bBack->setEnabled(true);
		   if(error->getErrorCode() == MegaError::API_OK)
		   {
               MegaNode *node = megaApi->getNodeByPath("/MEGAsync");
			   if(!node)
			   {
                   QMessageBox::warning(this, tr("Error"), tr("MEGA folder doesn't exist"), QMessageBox::Ok);
			   }
			   else
			   {
                   selectedMegaFolderHandle = node->getHandle();
				   ui->bBack->setVisible(false);
				   ui->bNext->setVisible(false);
				   ui->bCancel->setText(tr("Finish"));
#ifndef __APPLE__
                   ui->bFinalLocalFolder->setText(ui->eLocalFolder->text());
                   ui->bFinalMegaFolder->setText(ui->eMegaFolder->text());
#endif
				   ui->sPages->setCurrentWidget(ui->pWelcome);
                   delete node;
			   }
		   }
		   else
		   {
               QMessageBox::warning(this, tr("Error"),  error->QgetErrorString(), QMessageBox::Ok);
		   }
		   break;
		}
        case MegaRequest::TYPE_FETCH_NODES:
		{
            if(error->getErrorCode() != MegaError::API_OK)
            {
                ui->bBack->setEnabled(true);
                ui->bNext->setEnabled(true);
                ui->sPages->setCurrentWidget(ui->pLogin);
                sessionKey.clear();
                QMessageBox::warning(this, tr("Error"), error->QgetErrorString(), QMessageBox::Ok);
            }
            else if(megaApi->getRootNode() == NULL)
            {
                ui->bBack->setEnabled(true);
                ui->bNext->setEnabled(true);
                ui->sPages->setCurrentWidget(ui->pLogin);
                sessionKey.clear();
                QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                           "Please, try again. If the problem persists "
                                                           "please contact bug@mega.co.nz"), QMessageBox::Ok);
                preferences->setCrashed(true);
                app->rebootApplication(false);
            }
            else
            {
                QString email = ui->eLoginEmail->text().toLower().trimmed();
                if(preferences->hasEmail(email))
                {
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

                    close();
                    return;
                }

                ui->bBack->setEnabled(true);
                ui->bNext->setEnabled(true);
                ui->sPages->setCurrentWidget(ui->pSetupType);
            }

			break;
		}
    }
}

void SetupWizard::onRequestUpdate(MegaApi *api, MegaRequest *request)
{
    if(request->getType() == MegaRequest::TYPE_FETCH_NODES)
    {
        if(request->getTotalBytes()>0)
        {
            ui->progressBar->setMaximum(request->getTotalBytes());
            ui->progressBar->setValue(request->getTransferredBytes());
        }
    }
}


void SetupWizard::on_bNext_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if(w == ui->pSetup)
    {
        if(ui->rHaveAccount->isChecked())
        {
            ui->sPages->setCurrentWidget(ui->pLogin);
            ui->lVerify->setVisible(false);
            ui->eLoginEmail->setFocus();
        }
        else
        {
            ui->sPages->setCurrentWidget(ui->pNewAccount);
            ui->eName->setFocus();
        }

        ui->bBack->setVisible(true);
    }
    else if(w == ui->pLogin)
    {
        QString email = ui->eLoginEmail->text().toLower().trimmed();
        QString password = ui->eLoginPassword->text();
        if(email.length()==0)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your e-mail address"), QMessageBox::Ok);
            return;
        }

        if(!email.contains(QChar::fromAscii('@')) || !email.contains(QChar::fromAscii('.')))
        {
           QMessageBox::warning(this, tr("Error"), tr("Please, enter a valid e-mail address"), QMessageBox::Ok);
           return;
        }

        if(password.length()==0)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your password"), QMessageBox::Ok);
            return;
        }

		megaApi->logout();
		megaApi->login(email.toUtf8().constData(), password.toUtf8().constData(), delegateListener);
        ui->lProgress->setText(tr("Logging in..."));

        ui->progressBar->setMaximum(0);
        ui->progressBar->setValue(-1);
        ui->sPages->setCurrentWidget(ui->pProgress);
        ui->bBack->setEnabled(false);
        ui->bNext->setEnabled(false);
    }
    if(w == ui->pNewAccount)
    {
        QString name = ui->eName->text().trimmed();
        QString email = ui->eEmail->text().toLower().trimmed();
        QString password = ui->ePassword->text();
        QString repeatPassword = ui->eRepeatPassword->text();

        if(name.length()==0)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your name"), QMessageBox::Ok);
            return;
        }

        if(email.length()==0)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your e-mail address"), QMessageBox::Ok);
            return;
        }

        if(!email.contains(QChar::fromAscii('@')) || !email.contains(QChar::fromAscii('.')))
        {
           QMessageBox::warning(this, tr("Error"), tr("Please, enter a valid e-mail address"), QMessageBox::Ok);
           return;
        }

        if(password.length()==0)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter your password"), QMessageBox::Ok);
            return;
        }

        if(password.length()<8)
        {
            QMessageBox::warning(this, tr("Error"), tr("Please, enter a stronger password"), QMessageBox::Ok);
            return;
        }

        if(password.compare(repeatPassword)!=0)
        {
            QMessageBox::warning(this, tr("Error"), tr("The entered passwords don't match"), QMessageBox::Ok);
            return;
        }

        if(!ui->cAgreeWithTerms->isChecked())
        {
            QMessageBox::warning(this, tr("Error"), tr("You have to accept our terms of service"), QMessageBox::Ok);
            return;
        }

        megaApi->logout();
        megaApi->createAccount(email.toUtf8().constData(),
                                         password.toUtf8().constData(),
										 name.toUtf8().constData(), delegateListener);
        ui->lProgress->setText(tr("Creating account..."));
        ui->sPages->setCurrentWidget(ui->pProgress);
        ui->bBack->setEnabled(false);
        ui->bNext->setEnabled(false);
    }
    else if(w == ui->pSetupType)
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

        if(ui->rAdvancedSetup->isChecked())
        {
            defaultFolderPath.append(QString::fromAscii("/MEGAsync"));
            ui->eMegaFolder->setText(QString::fromAscii("/MEGAsync"));
            ui->sPages->setCurrentWidget(ui->pAdvanced);
        }
        else
        {
            defaultFolderPath.append(QString::fromAscii("/MEGA"));
            MegaNode *rootNode = megaApi->getRootNode();
            long long totalSize = megaApi->getSize(rootNode);
            delete rootNode;
            if(totalSize > 2147483648)
            {
                int res = QMessageBox::warning(this, tr("Warning"), tr("You have %1 in your Cloud Drive.\n"
                                                             "Are you sure you want to sync your entire Cloud Drive?")
                                                            .arg(Utilities::getSizeString(totalSize)),
                                     QMessageBox::Yes, QMessageBox::No);
                if(res != QMessageBox::Yes)
                {
                    this->wAdvancedSetup_clicked();
                    return;
                }
            }

            ui->eMegaFolder->setText(QString::fromAscii("/"));
            MegaNode *node = megaApi->getRootNode();
            selectedMegaFolderHandle = node->getHandle();
            ui->bBack->setVisible(false);
            ui->bNext->setVisible(false);
            ui->bCancel->setText(tr("Finish"));
            ui->bCancel->setFocus();

#ifndef __APPLE__
            ui->bFinalMegaFolder->setText(ui->eMegaFolder->text());
            ui->lFinalMegaFolderIntro->setText(tr("and your MEGA Cloud Drive"));
            ui->bFinalMegaFolder->hide();
#endif
            ui->sPages->setCurrentWidget(ui->pWelcome);

            delete node;
        }

        defaultFolderPath = QDir::toNativeSeparators(defaultFolderPath);
        QDir defaultFolder(defaultFolderPath);
        defaultFolder.mkpath(QString::fromAscii("."));
        ui->eLocalFolder->setText(defaultFolderPath);

#ifndef __APPLE__
        ui->bFinalLocalFolder->setText(ui->eLocalFolder->text());
#endif
    }
    else if(w == ui->pAdvanced)
    {
        if(ui->eLocalFolder->text().length()==0)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Please, select a local folder"), QMessageBox::Ok);
            return;
        }

        if(ui->eMegaFolder->text().length()==0)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Please, select a MEGA folder"), QMessageBox::Ok);
            return;
        }

        QString localFolderPath = ui->eLocalFolder->text();
        if(!Utilities::verifySyncedFolderLimits(localFolderPath))
        {
            QMessageBox::warning(this, tr("Warning"), tr("You are trying to sync an extremely large folder.\nTo prevent the syncing of entire boot volumes, which is inefficient and dangerous,\nwe ask you to start with a smaller folder and add more data while MEGAsync is running."), QMessageBox::Ok);
            return;
        }

        MegaNode *node = megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData());
        if(!node)
        {
            MegaNode *rootNode = megaApi->getRootNode();
            megaApi->createFolder("MEGAsync", rootNode, delegateListener);
            delete rootNode;
            ui->bNext->setEnabled(false);
            ui->bBack->setEnabled(false);
        }
        else
        {
            selectedMegaFolderHandle = node->getHandle();
            ui->bBack->setVisible(false);
            ui->bNext->setVisible(false);
            ui->bCancel->setText(tr("Finish"));
            ui->bCancel->setFocus();

#ifndef __APPLE__
            ui->bFinalLocalFolder->setText(ui->eLocalFolder->text());
            ui->bFinalMegaFolder->setText(ui->eMegaFolder->text());
#endif
            ui->sPages->setCurrentWidget(ui->pWelcome);
            delete node;
        }
    }
}

void SetupWizard::on_bBack_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if(w == ui->pLogin)
    {
        ui->sPages->setCurrentWidget(ui->pSetup);
        ui->bBack->setVisible(false);
        ui->eLoginPassword->clear();
    }
    if(w == ui->pNewAccount)
    {
        ui->sPages->setCurrentWidget(ui->pSetup);
        ui->bBack->setVisible(false);
        ui->eName->clear();
        ui->eEmail->clear();
        ui->ePassword->clear();
        ui->eRepeatPassword->clear();
    }
    else if(w == ui->pSetupType)
    {
        ui->sPages->setCurrentWidget(ui->pLogin);
        ui->eLoginPassword->clear();
        ui->lVerify->hide();
        sessionKey.clear();
        wTypicalSetup_clicked();
        megaApi->logout();
    }
    else if(w == ui->pAdvanced)
    {
        ui->sPages->setCurrentWidget(ui->pSetupType);
    }
}

void SetupWizard::on_bCancel_clicked()
{
    if(ui->sPages->currentWidget() == ui->pWelcome)
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

        QString syncName;
        MegaNode *rootNode = megaApi->getRootNode();
        if(selectedMegaFolderHandle == rootNode->getHandle()) syncName = QString::fromAscii("MEGA");
        preferences->addSyncedFolder(ui->eLocalFolder->text(), ui->eMegaFolder->text(), selectedMegaFolderHandle, syncName);
        delete rootNode;

        preferences->setProxyType(proxyType);
        preferences->setProxyServer(proxyServer);
        preferences->setProxyPort(proxyPort);
        preferences->setProxyProtocol(proxyProtocol);
        preferences->setProxyRequiresAuth(proxyAuth);
        preferences->setProxyUsername(proxyUsername);
        preferences->setProxyPassword(proxyPassword);

        this->close();
        return;
    }

    ::exit(0);
}

void SetupWizard::on_bLocalFolder_clicked()
{	
    QString defaultPath = ui->eLocalFolder->text().trimmed();
    if(!defaultPath.size())
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
	QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
                                                      defaultPath,
                                                      QFileDialog::ShowDirsOnly
													  | QFileDialog::DontResolveSymlinks);
	if(path.length())
    {
        QDir dir(path);
        if(!dir.exists() && !dir.mkpath(QString::fromAscii("."))) return;
        ui->eLocalFolder->setText(path);
    }
}

void SetupWizard::on_bMegaFolder_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, true, this);
    nodeSelector->nodesReady();
    int result = nodeSelector->exec();
    if(result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *node = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if(!node)
    {
        delete nodeSelector;
        return;
    }

    const char *nPath = megaApi->getNodePath(node);
    if(!nPath)
    {
        delete nodeSelector;
        delete node;
        return;
    }

    ui->eMegaFolder->setText(QString::fromUtf8(nPath));
    delete nodeSelector;
    delete nPath;
    delete node;
}

void SetupWizard::wTypicalSetup_clicked()
{
    ui->wTypicalSetup->setStyleSheet(QString::fromAscii(
                                     "#wTypicalSetup {background: rgb(242,242,242);"
                                     "border: 2px solid rgb(217, 217, 217);"
                                     "border-radius: 10px;}"));
    ui->wAdvancedSetup->setStyleSheet(QString::fromAscii(
                                     "#wAdvancedSetup {background: transparent;"
                                     "border: none;}"));
    ui->rTypicalSetup->setChecked(true);
    ui->rAdvancedSetup->setChecked(false);
}

void SetupWizard::wAdvancedSetup_clicked()
{
    ui->wTypicalSetup->setStyleSheet(QString::fromAscii(
                                     "#wTypicalSetup {background: transparent;"
                                     "border: none;}"));
    ui->wAdvancedSetup->setStyleSheet(QString::fromAscii(
                                     "#wAdvancedSetup {background: rgb(242,242,242);"
                                     "border: 2px solid rgb(217, 217, 217);"
                                     "border-radius: 10px;}"));
    ui->rTypicalSetup->setChecked(false);
    ui->rAdvancedSetup->setChecked(true);
}

bool SetupWizard::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(obj == ui->wTypicalSetup) wTypicalSetup_clicked();
        else if(obj == ui->wAdvancedSetup) wAdvancedSetup_clicked();
        else if(obj == ui->lTermsLink) lTermsLink_clicked();
    }
    return QObject::eventFilter(obj, event);
}

void SetupWizard::lTermsLink_clicked()
{
    ui->cAgreeWithTerms->toggle();
}

void SetupWizard::on_bFinalLocalFolder_clicked()
{
#ifndef __APPLE__
    QString localFolderPath = ui->bFinalLocalFolder->text();
    QDesktopServices::openUrl(QUrl::fromLocalFile(localFolderPath));
#endif
}

void SetupWizard::on_bFinalMegaFolder_clicked()
{
#ifndef __APPLE__
    QString megaFolderPath = ui->bFinalMegaFolder->text();
    MegaNode *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
    if(node)
    {
        const char *handle = node->getBase64Handle();
        QString url = QString::fromAscii("https://mega.co.nz/#fm/") + QString::fromAscii(handle);
        QDesktopServices::openUrl(QUrl(url));
        delete handle;
        delete node;
    }
#endif
}
