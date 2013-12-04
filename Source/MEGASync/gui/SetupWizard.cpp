#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include "MegaApplication.h"

SetupWizard::SetupWizard(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupWizard)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->bBack->setVisible(false);
    this->app = app;
    megaApi = app->getMegaApi();
    preferences = app->getPreferences();
    selectedMegaFolderHandle = UNDEF;
    ui->bNext->setFocus();
}

SetupWizard::~SetupWizard()
{
    delete ui;
}

void SetupWizard::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    cout << "Request finished!" << endl;
    this->request = request->copy();
    this->error = e->copy();
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

bool SetupWizard::event(QEvent *event)
{
    if(event->type() != QEvent::User)
        return QDialog::event(event);

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
                QMessageBox::warning(this, tr("Error"), error->getErrorString(), QMessageBox::Ok);
            }
            break;
        }
        case MegaRequest::TYPE_LOGIN:
        {
            if(error->getErrorCode() == MegaError::API_OK)
            {
				preferences->setEmail(ui->eLoginEmail->text().toLower().trimmed());
				preferences->setPassword(ui->eLoginPassword->text());

                ui->lProgress->setText(tr("Fetching file list..."));
                megaApi->fetchNodes(this);
                megaApi->getAccountDetails();
            }
            else if(error->getErrorCode() == MegaError::API_ENOENT)
            {
                ui->bBack->setEnabled(true);
                ui->bNext->setEnabled(true);
                ui->sPages->setCurrentWidget(ui->pLogin);
                QMessageBox::warning(this, tr("Error"), tr("Incorrect email and/or password.") + " " + tr("Have you verified your account?"), QMessageBox::Ok);
            }
            else
            {
                ui->bBack->setEnabled(true);
                ui->bNext->setEnabled(true);
                ui->sPages->setCurrentWidget(ui->pLogin);
                QMessageBox::warning(this, tr("Error"), error->getErrorString(), QMessageBox::Ok);
            }
            break;
        }
        case MegaRequest::TYPE_MKDIR:
        {
           if(error->getErrorCode() == MegaError::API_OK)
           {
               Node *node = megaApi->getNodeByPath("/MegaSync");
               if(!node)
               {
                   QMessageBox::warning(this, tr("Error"), tr("Mega folder doesn't exist"), QMessageBox::Ok);
                   return true;
               }

               selectedMegaFolderHandle = node->nodehandle;
               ui->bBack->setVisible(false);
               ui->bNext->setVisible(false);
               ui->bCancel->setText(tr("Finish"));
               ui->lFinalLocalFolder->setText(ui->eLocalFolder->text());
               ui->lFinalMegaFolder->setText(ui->eMegaFolder->text());
			   ui->sPages->setCurrentWidget(ui->pWelcome);
           }
           else
           {
               QMessageBox::warning(this, tr("Error"), error->getErrorString(), QMessageBox::Ok);
           }
		   break;
        }
        case MegaRequest::TYPE_FETCH_NODES:
        {
            ui->bBack->setEnabled(true);
            ui->bNext->setEnabled(true);
            ui->sPages->setCurrentWidget(ui->pSetupType);
            break;
        }
    }

    delete request;
    delete error;
    return true;
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
            QMessageBox::warning(this, tr("Warning"), tr("Please, enter your e-mail address"), QMessageBox::Ok);
            return;
        }
        if(password.length()==0)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Please, enter your password"), QMessageBox::Ok);
            return;
        }

        preferences->setEmail(email);
        megaApi->logout();
        megaApi->login(email.toUtf8().constData(), password.toUtf8().constData(), this);
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
            QMessageBox::warning(this, tr("Warning"), tr("Please, enter your name"), QMessageBox::Ok);
            return;
        }

        if(email.length()==0)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Please, enter your e-mail address"), QMessageBox::Ok);
            return;
        }

        if(password.length()==0)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Please, enter your password"), QMessageBox::Ok);
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
                                         name.toUtf8().constData(), this);
        ui->lProgress->setText(tr("Creating account..."));
        ui->sPages->setCurrentWidget(ui->pProgress);
        ui->bBack->setEnabled(false);
        ui->bNext->setEnabled(false);
    }
    else if(w == ui->pSetupType)
    {

    #if QT_VERSION < 0x050000
		QDir defaultFolder(QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/MEGAsync");
    #else
		QDir defaultFolder(QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation)[0] + "/MEGAsync");
	#endif

        defaultFolder.mkpath(".");
        QString defaultFolderPath = defaultFolder.absolutePath();

    #ifdef WIN32
       defaultFolderPath = defaultFolderPath.replace("/","\\");
    #endif

         ui->eLocalFolder->setText(defaultFolderPath);
		ui->eMegaFolder->setText("/MEGAsync");
        if(ui->rAdvancedSetup->isChecked())
        {
            ui->sPages->setCurrentWidget(ui->pAdvanced);
        }
        else
        {
            Node *node = megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData());
            if(!node || (node->type==FILENODE))
            {
				megaApi->createFolder("MEGAsync", megaApi->getRootNode(), this);
            }
            else
            {
                selectedMegaFolderHandle = node->nodehandle;
                ui->bBack->setVisible(false);
                ui->bNext->setVisible(false);
                ui->bCancel->setText(tr("Finish"));
                ui->bCancel->setFocus();
                ui->lFinalLocalFolder->setText(ui->eLocalFolder->text());
                ui->lFinalMegaFolder->setText(ui->eMegaFolder->text());
				ui->sPages->setCurrentWidget(ui->pWelcome);
			}
        }
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

        Node *node = megaApi->getNodeByPath(ui->eMegaFolder->text().toUtf8().constData());
        if(!node)
        {
            megaApi->createFolder("MegaSync", megaApi->getRootNode(), this);
        }
        else
        {
            selectedMegaFolderHandle = node->nodehandle;
            ui->bBack->setVisible(false);
            ui->bNext->setVisible(false);
            ui->bCancel->setText(tr("Finish"));
            ui->bCancel->setFocus();
            ui->lFinalLocalFolder->setText(ui->eLocalFolder->text());
            ui->lFinalMegaFolder->setText(ui->eMegaFolder->text());
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
        app->getMegaApi()->logout();
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
        preferences->addSyncedFolder(ui->lFinalLocalFolder->text(), ui->lFinalMegaFolder->text(), selectedMegaFolderHandle);
        preferences->setSetupWizardCompleted(true);
        this->close();
		megaApi->syncFolder(ui->lFinalLocalFolder->text().toUtf8().constData(), megaApi->getNodeByHandle(selectedMegaFolderHandle));
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
        ui->eLocalFolder->setText(path);
}

void SetupWizard::on_bMegaFolder_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(app->getMegaApi());
    nodeSelector->nodesReady();
    int result = nodeSelector->exec();

    if(result != QDialog::Accepted)
        return;

    selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    ui->eMegaFolder->setText(megaApi->getNodePath(megaApi->getNodeByHandle(selectedMegaFolderHandle)));
}
