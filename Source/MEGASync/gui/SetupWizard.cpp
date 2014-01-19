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
    ui->wAdvancedSetup->installEventFilter(this);
    ui->wTypicalSetup->installEventFilter(this);
    ui->lTermsLink->installEventFilter(this);
    ui->rTypicalSetup->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->rAdvancedSetup->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->bBack->setVisible(false);
    this->app = app;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    selectedMegaFolderHandle = UNDEF;
    ui->bNext->setFocus();
	delegateListener = new QTMegaRequestListener(this);
}

SetupWizard::~SetupWizard()
{
    delete ui;
}

void SetupWizard::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *error)
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
				ui->lVerify->setVisible(true);
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
				ui->lProgress->setText(tr("Fetching file list..."));
                megaApi->fetchNodes(delegateListener);
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
		   if(error->getErrorCode() == MegaError::API_OK)
		   {
			   Node *node = megaApi->getNodeByPath("/MEGAsync");
			   if(!node)
			   {
                   QMessageBox::warning(this, tr("Error"), tr("MEGA folder doesn't exist"), QMessageBox::Ok);
			   }
			   else
			   {
				   selectedMegaFolderHandle = node->nodehandle;
				   ui->bBack->setVisible(false);
				   ui->bNext->setVisible(false);
				   ui->bCancel->setText(tr("Finish"));
                   ui->bFinalLocalFolder->setText(ui->eLocalFolder->text());
                   ui->bFinalMegaFolder->setText(ui->eMegaFolder->text());
				   ui->sPages->setCurrentWidget(ui->pWelcome);
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
            QString email = ui->eLoginEmail->text().toLower().trimmed();
            if(preferences->hasEmail(email))
            {
                QString privatePw = QString::fromUtf8(megaApi->getBase64PwKey(ui->eLoginPassword->text().toUtf8().constData()));
                QString emailHash = QString::fromUtf8(megaApi->getStringHash(privatePw.toUtf8().constData(), email.toUtf8().constData()));
                preferences->setEmail(email);
                preferences->setCredentials(emailHash, privatePw);
                close();
                return;
            }

			ui->bBack->setEnabled(true);
			ui->bNext->setEnabled(true);
			ui->sPages->setCurrentWidget(ui->pSetupType);

			break;
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

#if QT_VERSION < 0x050000
    QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#else
    QString defaultFolderPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
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
            long long totalSize = megaApi->getSize(megaApi->getRootNode());
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
            Node *node = megaApi->getRootNode();
            selectedMegaFolderHandle = node->nodehandle;
            ui->bBack->setVisible(false);
            ui->bNext->setVisible(false);
            ui->bCancel->setText(tr("Finish"));
            ui->bCancel->setFocus();
            ui->bFinalMegaFolder->setText(ui->eMegaFolder->text());
            ui->sPages->setCurrentWidget(ui->pWelcome);
            ui->lFinalMegaFolderIntro->setText(tr("and your MEGA Cloud Drive"));
            ui->bFinalMegaFolder->hide();
        }

        defaultFolderPath = QDir::toNativeSeparators(defaultFolderPath);
        QDir defaultFolder(defaultFolderPath);
        defaultFolder.mkpath(QString::fromAscii("."));
        ui->eLocalFolder->setText(defaultFolderPath);
        ui->bFinalLocalFolder->setText(ui->eLocalFolder->text());
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
            QMessageBox::warning(this, tr("Warning"), tr("Local folder too large (this beta is limited to %1 folders or %2 files.\n"
                 "Please, select another folder.").arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
                 .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER), QMessageBox::Ok);
            return;
        }

        Node *node = megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData());
        if(!node)
        {
			megaApi->createFolder("MEGAsync", megaApi->getRootNode(), delegateListener);
        }
        else
        {
            selectedMegaFolderHandle = node->nodehandle;
            ui->bBack->setVisible(false);
            ui->bNext->setVisible(false);
            ui->bCancel->setText(tr("Finish"));
            ui->bCancel->setFocus();
            ui->bFinalLocalFolder->setText(ui->eLocalFolder->text());
            ui->bFinalMegaFolder->setText(ui->eMegaFolder->text());
            ui->sPages->setCurrentWidget(ui->pWelcome);
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
    }
    if(w == ui->pNewAccount)
    {
        ui->sPages->setCurrentWidget(ui->pSetup);
        ui->bBack->setVisible(false);
    }
    else if(w == ui->pSetupType)
    {
        ui->sPages->setCurrentWidget(ui->pLogin);
        ui->lVerify->hide();
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
        QString privatePw = QString::fromUtf8(megaApi->getBase64PwKey(ui->eLoginPassword->text().toUtf8().constData()));
        QString emailHash = QString::fromUtf8(megaApi->getStringHash(privatePw.toUtf8().constData(), email.toUtf8().constData()));

        preferences->setEmail(email);
        preferences->setCredentials(emailHash, privatePw);
        QString syncName;
        if(selectedMegaFolderHandle == megaApi->getRootNode()->nodehandle) syncName = QString::fromAscii("MEGA");
        preferences->addSyncedFolder(ui->bFinalLocalFolder->text(), ui->bFinalMegaFolder->text(), selectedMegaFolderHandle, syncName);
        this->close();
        return;
    }

    ::exit(0);
}

void SetupWizard::on_bLocalFolder_clicked()
{	
	QString path =  QFileDialog::getExistingDirectory(this, tr("Select local folder"),
                                                      ui->eLocalFolder->text(),
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
        return;

    selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    ui->eMegaFolder->setText(QString::fromUtf8(megaApi->getNodePath(megaApi->getNodeByHandle(selectedMegaFolderHandle))));
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
    QString localFolderPath = ui->bFinalLocalFolder->text();
    QDesktopServices::openUrl(QUrl::fromLocalFile(localFolderPath));
}

void SetupWizard::on_bFinalMegaFolder_clicked()
{
    QString megaFolderPath = ui->bFinalMegaFolder->text();
    Node *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
    if(node)
    {
        const char *handle = MegaApi::getBase64Handle(node);
        QString url = QString::fromAscii("https://mega.co.nz/#fm/") + QString::fromAscii(handle);
        QDesktopServices::openUrl(QUrl(url));
        delete handle;
    }
}
