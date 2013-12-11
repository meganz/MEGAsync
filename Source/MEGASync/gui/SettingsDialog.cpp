#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>

#include "MegaApplication.h"
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#ifdef WIN32
    #include <utils/WindowsUtils.h>
#endif

SettingsDialog::SettingsDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    this->app = app;
    this->megaApi = app->getMegaApi();
    this->preferences = app->getPreferences();
	syncsChanged = false;

    ui->eProxyPort->setValidator(new QIntValidator(this));
    ui->eLimit->setValidator(new QDoubleValidator(this));

    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::stateChanged()
{
    ui->bApply->setEnabled(true);
}

void SettingsDialog::on_bGeneral_clicked()
{
    ui->bGeneral->setChecked(true);
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
	//ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pGeneral);
}

void SettingsDialog::on_bAccount_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(true);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
	//ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pAccount);
}

void SettingsDialog::on_bSyncs_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(true);
    ui->bBandwidth->setChecked(false);
	//ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pSyncs);
    ui->tSyncs->horizontalHeader()->setVisible( true );
}

void SettingsDialog::on_bBandwidth_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(true);
	//ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pBandwidth);
}

void SettingsDialog::on_bProxies_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
	//ui->bProxies->setChecked(true);
    ui->wStack->setCurrentWidget(ui->pProxies);
}

void SettingsDialog::on_bCancel_clicked()
{
    this->close();
}

void SettingsDialog::on_bOk_clicked()
{
    saveSettings();
    this->close();
}

void SettingsDialog::on_bHelp_clicked()
{
    QString helpUrl("https://mega.co.nz/#help");
    QDesktopServices::openUrl(QUrl(helpUrl));
}

void SettingsDialog::on_rProxyManual_clicked()
{
    ui->cProxyType->setEnabled(true);
    ui->eProxyServer->setEnabled(true);
    ui->eProxyPort->setEnabled(true);
    ui->cProxyRequiresPassword->setEnabled(true);
    if(ui->cProxyRequiresPassword->isChecked())
    {
        ui->eProxyUsername->setEnabled(true);
        ui->eProxyPassword->setEnabled(true);
    }
    else
    {
        ui->eProxyUsername->setEnabled(false);
        ui->eProxyPassword->setEnabled(false);
    }
}

void SettingsDialog::on_rProxyAuto_clicked()
{
    ui->cProxyType->setEnabled(false);
    ui->eProxyServer->setEnabled(false);
    ui->eProxyPort->setEnabled(false);
    ui->eProxyUsername->setEnabled(false);
    ui->eProxyPassword->setEnabled(false);
    ui->cProxyRequiresPassword->setEnabled(false);
}

void SettingsDialog::on_rNoProxy_clicked()
{
    ui->cProxyType->setEnabled(false);
    ui->eProxyServer->setEnabled(false);
    ui->eProxyPort->setEnabled(false);
    ui->eProxyUsername->setEnabled(false);
    ui->eProxyPassword->setEnabled(false);
    ui->cProxyRequiresPassword->setEnabled(false);
}

void SettingsDialog::on_bUpgrade_clicked()
{
	QString upgradeUrl("https://mega.co.nz/#pro");
	QDesktopServices::openUrl(QUrl(upgradeUrl));
}

void SettingsDialog::on_bUpgradeBandwidth_clicked()
{
	QString upgradeUrl("https://mega.co.nz/#pro");
	QDesktopServices::openUrl(QUrl(upgradeUrl));
}

void SettingsDialog::on_rNoLimit_clicked()
{
    ui->eLimit->setEnabled(false);
}

void SettingsDialog::on_rLimit_clicked()
{
    ui->eLimit->setEnabled(true);
}

void SettingsDialog::on_cProxyRequiresPassword_clicked()
{
    if(ui->cProxyRequiresPassword->isChecked())
    {
        ui->eProxyUsername->setEnabled(true);
        ui->eProxyPassword->setEnabled(true);
    }
    else
    {
        ui->eProxyUsername->setEnabled(false);
        ui->eProxyPassword->setEnabled(false);
    }
}

void SettingsDialog::loadSettings()
{
    //General
    ui->cShowNotifications->setChecked(preferences->showNotifications());
    ui->cAutoUpdate->setChecked(preferences->updateAutomatically());
    ui->cStartOnStartup->setChecked(preferences->startOnStartup());

    //Account
    ui->lEmail->setText(preferences->email());
	if(preferences->totalStorage()==0)
	{
		ui->pStorage->setValue(0);
		ui->lStorage->setText(tr("Data temporarily unavailable"));
	}
	else
	{
		int percentage = 100*((double)preferences->usedStorage()/preferences->totalStorage());
		ui->pStorage->setValue(percentage);
		ui->lStorage->setText(WindowsUtils::getSizeString(preferences->usedStorage()) + " (" +
			  QString::number(percentage) + "%) of " +
			  WindowsUtils::getSizeString(preferences->totalStorage()) + " used");
	}
    switch(preferences->accountType())
    {
        case Preferences::ACCOUNT_TYPE_FREE:
			ui->lAccountImage->setPixmap(QPixmap("://images/Free.ico"));
            ui->lAccountType->setText(tr("FREE"));
            break;
        case Preferences::ACCOUNT_TYPE_PROI:
			ui->lAccountImage->setPixmap(QPixmap("://images/Pro I.ico"));
            ui->lAccountType->setText(tr("PRO I"));
            break;
        case Preferences::ACCOUNT_TYPE_PROII:
			ui->lAccountImage->setPixmap(QPixmap("://images/Pro II.ico"));
            ui->lAccountType->setText(tr("PRO II"));
            break;
        case Preferences::ACCOUNT_TYPE_PROIII:
			ui->lAccountImage->setPixmap(QPixmap("://images/Pro III.ico"));
            ui->lAccountType->setText(tr("PRO III"));
            break;
    }

    //Syncs
    loadSyncSettings();

    //Bandwidth
    ui->rLimit->setChecked(preferences->uploadLimitKB()>=0);
    ui->rNoLimit->setChecked(preferences->uploadLimitKB()<0);
    ui->eLimit->setText((preferences->uploadLimitKB()<=0)? "0" : QString::number(preferences->uploadLimitKB()));

	if(preferences->totalBandwidth() == 0)
	{
		ui->pUsedBandwidth->setValue(0);
		ui->lBandwidth->setText(tr("Data temporarily unavailable"));
	}
	else
	{
		int bandwidthPercentage = 100*((double)preferences->usedBandwidth()/preferences->totalBandwidth());
		ui->pUsedBandwidth->setValue(bandwidthPercentage);
		ui->lBandwidth->setText(WindowsUtils::getSizeString(preferences->usedBandwidth()) + " (" +
			QString::number(bandwidthPercentage) + "%) of " +
			WindowsUtils::getSizeString(preferences->totalBandwidth()) + " used");
	}

    //Proxies
    ui->rNoProxy->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_NONE);
    ui->rProxyAuto->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_AUTO);
    ui->rProxyManual->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_CUSTOM);
    ui->cProxyType->setCurrentIndex(preferences->proxyProtocol());
    ui->eProxyServer->setText(preferences->proxyServer());
    ui->eProxyPort->setText(QString::number(preferences->proxyPort()));
    ui->cProxyRequiresPassword->setChecked(preferences->proxyRequiresAuth());
    ui->eProxyUsername->setText(preferences->getProxyUsername());
    ui->eProxyPassword->setText(preferences->getProxyPassword());

    ui->bApply->setEnabled(false);
    this->update();
}

void SettingsDialog::saveSettings()
{
    //General
    preferences->setShowNotifications(ui->cShowNotifications->isChecked());
    preferences->setUpdateAutomatically(ui->cAutoUpdate->isChecked());
    preferences->setStartOnStartup(ui->cStartOnStartup->isChecked());

    #ifdef WIN32
		WindowsUtils::startOnStartup(ui->cStartOnStartup->isChecked());
    #endif

    //Syncs
	if(syncsChanged)
	{
		preferences->removeAllFolders();

		for(int i=0; i<ui->tSyncs->rowCount(); i++)
		{
			QString localFolderPath = ui->tSyncs->item(i, 0)->text().trimmed();
			QString megaFolderPath = ui->tSyncs->item(i, 1)->text().trimmed();
			Node *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
			cout << "adding sync: " << localFolderPath.toUtf8().constData() << " - " <<
					megaFolderPath.toUtf8().constData() << endl;
			preferences->addSyncedFolder(localFolderPath.toUtf8().constData(),
										 megaFolderPath.toUtf8().constData(),
										 node->nodehandle);
		}

		app->reloadSyncs();
		syncsChanged = false;
	}

    //Bandwidth
    if(ui->rNoLimit->isChecked() || ui->lLimit->text().trimmed().length()==0) preferences->setUploadLimitKB(-1);
    else preferences->setUploadLimitKB(ui->eLimit->text().trimmed().toInt());

    //Proxies
    if(ui->rNoProxy->isChecked()) preferences->setProxyType(Preferences::PROXY_TYPE_NONE);
    else if(ui->rProxyAuto->isChecked()) preferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
    else preferences->setProxyType(Preferences::PROXY_TYPE_CUSTOM);

    preferences->setProxyProtocol(ui->cProxyType->currentIndex());
    preferences->setProxyServer(ui->eProxyServer->text().trimmed());
    preferences->setProxyPort(ui->eProxyPort->text().toInt());
    preferences->setProxyRequiresAuth(ui->cProxyRequiresPassword->isChecked());
    preferences->setProxyUsername(ui->eProxyUsername->text());
    preferences->setProxyPassword(ui->eProxyPassword->text());

    ui->bApply->setEnabled(false);
}

void SettingsDialog::on_bDelete_clicked()
{
    cout << "bDelete clicked" << endl;
    QList<QTableWidgetSelectionRange> selected = ui->tSyncs->selectedRanges();
    if(selected.size()==0)
    {
        cout << "No items selected" << endl;
        return;
    }
    int index = selected.first().topRow();
    cout << "Selected index: " << index << endl;

	ui->tSyncs->removeRow(index);

	syncsChanged = true;
	stateChanged();

	//preferences->removeSyncedFolder(index);
	//loadSyncSettings();
}

void SettingsDialog::loadSyncSettings()
{
    ui->tSyncs->clearContents();

    ui->tSyncs->horizontalHeader()->setVisible(true);
    int numFolders = preferences->getNumSyncedFolders();
    ui->tSyncs->setRowCount(numFolders);
    ui->tSyncs->setColumnCount(2);
    for(int i=0; i<numFolders; i++)
    {
        QTableWidgetItem *localFolder = new QTableWidgetItem();
        localFolder->setText("  " + preferences->getLocalFolder(i) + "  ");
        QTableWidgetItem *megaFolder = new QTableWidgetItem();
        megaFolder->setText("  " + preferences->getMegaFolder(i) + "  ");
        ui->tSyncs->setItem(i, 0, localFolder);
        ui->tSyncs->setItem(i, 1, megaFolder);
    }
}


void SettingsDialog::on_bAdd_clicked()
{
    BindFolderDialog *dialog = new BindFolderDialog(this);
    int result = dialog->exec();
    if(result != QDialog::Accepted)
        return;

   QString localFolderPath = dialog->getLocalFolder();
   if(!WindowsUtils::verifySyncedFolderLimits(localFolderPath))
   {
       QMessageBox::warning(this, tr("Warning"), tr("Too many files or folders (+%1 folders or +%2 files).\n"
            "Please, select another folder.").arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
            .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER), QMessageBox::Ok);
	   return;
   }

   long long handle = dialog->getMegaFolder();
   Node *node = megaApi->getNodeByHandle(handle);
   if(!localFolderPath.length() || !node)
       return;

   /*for(int i=0; i<preferences->getNumSyncedFolders(); i++)
   {
	   QString s = preferences->getLocalFolder(i);
	   if(!s.compare(localFolderPath))
	   {
		   cout << "Local folder already synced" << endl;
		   return;
	   }
   }*/

   QTableWidgetItem *localFolder = new QTableWidgetItem();
   localFolder->setText("  " + localFolderPath + "  ");
   QTableWidgetItem *megaFolder = new QTableWidgetItem();
   megaFolder->setText("  " +  QString(megaApi->getNodePath(node)) + "  ");
   int pos = ui->tSyncs->rowCount();
   ui->tSyncs->setRowCount(pos+1);
   ui->tSyncs->setItem(pos, 0, localFolder);
   ui->tSyncs->setItem(pos, 1, megaFolder);

   syncsChanged = true;
   stateChanged();

   //preferences->addSyncedFolder(localFolderPath, megaApi->getNodePath(node), handle);
   //megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
   //loadSyncSettings();
}

void SettingsDialog::on_bApply_clicked()
{
    saveSettings();
}

void SettingsDialog::on_bUnlink_clicked()
{
    if(QMessageBox::question(this, tr("Unlink account"),
            tr("All your settings will be deleted.") + " " + tr("Are you sure?"),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
    {
        this->close();
        app->unlink();
    }
}
