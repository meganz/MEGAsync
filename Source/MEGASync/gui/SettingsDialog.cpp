#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>

#include "MegaApplication.h"
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <utils/Utils.h>

SettingsDialog::SettingsDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->app = app;
    this->megaApi = app->getMegaApi();
    this->preferences = app->getPreferences();
	syncsChanged = false;
    excludedNamesChanged = false;

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
    ui->bAdvanced->setChecked(false);
	//ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pGeneral);
}

void SettingsDialog::on_bAccount_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(true);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
	//ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pAccount);
}

void SettingsDialog::on_bSyncs_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(true);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
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
    ui->bAdvanced->setChecked(false);
	//ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pBandwidth);
}

void SettingsDialog::on_bAdvanced_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(true);
    //ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pAdvanced);
}

/*
void SettingsDialog::on_bProxies_clicked()
{
    ui->bGeneral->setChecked(false);
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
	//ui->bProxies->setChecked(true);
    ui->wStack->setCurrentWidget(ui->pProxies);
}
*/

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
    QString helpUrl = QString::fromAscii("https://mega.co.nz/#help/sync");
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
    QString upgradeUrl = QString::fromAscii("https://mega.co.nz/#pro");
	QDesktopServices::openUrl(QUrl(upgradeUrl));
}

void SettingsDialog::on_bUpgradeBandwidth_clicked()
{
    QString upgradeUrl = QString::fromAscii("https://mega.co.nz/#pro");
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

    //TODO: Enable autoUpdate again in the next release
    //ui->cAutoUpdate->setChecked(preferences->updateAutomatically());

    //ui->cStartOnStartup->setChecked(preferences->startOnStartup());

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
        ui->lStorage->setText(Utils::getSizeString(preferences->usedStorage()) + QString::fromAscii(" (") +
              QString::number(percentage) + tr("%) of ") +
              Utils::getSizeString(preferences->totalStorage()) + tr(" used"));
	}
    switch(preferences->accountType())
    {
        case Preferences::ACCOUNT_TYPE_FREE:
            ui->lAccountImage->setPixmap(QPixmap(QString::fromAscii("://images/Free.ico")));
            ui->lAccountType->setText(tr("FREE"));
            break;
        case Preferences::ACCOUNT_TYPE_PROI:
            ui->lAccountImage->setPixmap(QPixmap(QString::fromAscii("://images/Pro I.ico")));
            ui->lAccountType->setText(tr("PRO I"));
            break;
        case Preferences::ACCOUNT_TYPE_PROII:
            ui->lAccountImage->setPixmap(QPixmap(QString::fromAscii("://images/Pro II.ico")));
            ui->lAccountType->setText(tr("PRO II"));
            break;
        case Preferences::ACCOUNT_TYPE_PROIII:
            ui->lAccountImage->setPixmap(QPixmap(QString::fromAscii("://images/Pro III.ico")));
            ui->lAccountType->setText(tr("PRO III"));
            break;
    }

    Node *node = megaApi->getNodeByHandle(preferences->uploadFolder());
    if(!node) ui->eUploadFolder->setText(tr("/MEGAsync Uploads"));
    else ui->eUploadFolder->setText(QString::fromUtf8(megaApi->getNodePath(node)));

    //Syncs
    loadSyncSettings();

    //Bandwidth
    ui->rAutoLimit->setChecked(preferences->uploadLimitKB()<0);
    ui->rLimit->setChecked(preferences->uploadLimitKB()>0);
    ui->rNoLimit->setChecked(preferences->uploadLimitKB()==0);
    ui->eLimit->setText((preferences->uploadLimitKB()<=0)? QString::fromAscii("0") : QString::number(preferences->uploadLimitKB()));
    ui->eLimit->setEnabled(ui->rLimit->isChecked());

	if(preferences->totalBandwidth() == 0)
	{
		ui->pUsedBandwidth->setValue(0);
		ui->lBandwidth->setText(tr("Data temporarily unavailable"));
	}
	else
	{
		int bandwidthPercentage = 100*((double)preferences->usedBandwidth()/preferences->totalBandwidth());
		ui->pUsedBandwidth->setValue(bandwidthPercentage);
        ui->lBandwidth->setText(Utils::getSizeString(preferences->usedBandwidth()) + QString::fromAscii(" (") +
            QString::number(bandwidthPercentage) + tr("%) of ") +
            Utils::getSizeString(preferences->totalBandwidth()) + tr(" used"));
	}

    //Proxies
    /*ui->rNoProxy->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_NONE);
    ui->rProxyAuto->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_AUTO);
    ui->rProxyManual->setChecked(preferences->proxyType()==Preferences::PROXY_TYPE_CUSTOM);
    ui->cProxyType->setCurrentIndex(preferences->proxyProtocol());
    ui->eProxyServer->setText(preferences->proxyServer());
    ui->eProxyPort->setText(QString::number(preferences->proxyPort()));
    ui->cProxyRequiresPassword->setChecked(preferences->proxyRequiresAuth());
    ui->eProxyUsername->setText(preferences->getProxyUsername());
    ui->eProxyPassword->setText(preferences->getProxyPassword());*/

    //Advanced
    QStringList excludedNames = preferences->getExcludedSyncNames();
    for(int i=0; i<excludedNames.size(); i++)
        ui->lExcludedNames->addItem(excludedNames[i]);


    ui->bApply->setEnabled(false);
    this->update();
}

void SettingsDialog::saveSettings()
{
    //General
    preferences->setShowNotifications(ui->cShowNotifications->isChecked());

    //TODO: Enable autoUpdate again in the next release
    //bool updateAutomatically = ui->cAutoUpdate->isChecked();
    //if(updateAutomatically != preferences->updateAutomatically())
    //{
    //    if(updateAutomatically) app->startUpdateTask();
    //    else app->stopUpdateTask();
    //}
    //preferences->setUpdateAutomatically(updateAutomatically);

    //bool startOnStartup = ui->cStartOnStartup->isChecked();
    //Utils::startOnStartup(startOnStartup);
    //preferences->setStartOnStartup(startOnStartup);

    //Account
    Node *node = megaApi->getNodeByPath(ui->eUploadFolder->text().toUtf8().constData());
    if(node && ui->eUploadFolder->text().compare(tr("/MEGAsync Uploads")))
        preferences->setUploadFolder(node->nodehandle);

    //Syncs
	if(syncsChanged)
	{
        //Check for removed folders
        for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        {
            QString localPath = preferences->getLocalFolder(i);
            long long megaHandle = preferences->getMegaFolderHandle(i);

            int j;
            for(j=0; j<ui->tSyncs->rowCount(); j++)
            {
                QString newLocalPath = ui->tSyncs->item(j, 0)->text().trimmed();
                QString newMegaPath = ui->tSyncs->item(j, 1)->text().trimmed();
                Node *n = megaApi->getNodeByPath(newMegaPath.toUtf8().constData());
                if(!n) continue;

                if((n->nodehandle == megaHandle) || !localPath.compare(newLocalPath))
                    break;
            }

            if(j == ui->tSyncs->rowCount())
            {
                cout << "REMOVING SYNC: " << preferences->getSyncName(i).toStdString() << endl;
                preferences->removeSyncedFolder(i);
                megaApi->removeSync(megaHandle);
                i--;
            }
        }

        //Check for new folders
		for(int i=0; i<ui->tSyncs->rowCount(); i++)
		{
			QString localFolderPath = ui->tSyncs->item(i, 0)->text().trimmed();
			QString megaFolderPath = ui->tSyncs->item(i, 1)->text().trimmed();
            QString syncName = syncNames.at(i);
			Node *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
            if(!node) continue;
            int j;
            for(j=0; j<preferences->getNumSyncedFolders(); j++)
            {
                QString previousLocalPath = preferences->getLocalFolder(j);
                long long previousMegaHandle = preferences->getMegaFolderHandle(j);

                if((node->nodehandle == previousMegaHandle) || !localFolderPath.compare(previousLocalPath))
                    break;
            }

            if(j == preferences->getNumSyncedFolders())
            {
                cout << "ADDING SYNC: " << localFolderPath.toUtf8().constData() << " - " <<
                        megaFolderPath.toUtf8().constData() << endl;
                preferences->addSyncedFolder(localFolderPath,
                                             megaFolderPath,
                                             node->nodehandle,
                                             syncName);
                megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
            }
		}

        updateAddButton();
		syncsChanged = false;
	}

    //Bandwidth
    if(ui->rNoLimit->isChecked() || ui->lLimit->text().trimmed().length()==0) preferences->setUploadLimitKB(0);
    else if(ui->rAutoLimit->isChecked()) preferences->setUploadLimitKB(-1);
    else preferences->setUploadLimitKB(ui->eLimit->text().trimmed().toInt());

    app->setUploadLimit(preferences->uploadLimitKB());

    //Advanced
    if(excludedNamesChanged)
    {
        QStringList excludedNames;
        for(int i=0; i<ui->lExcludedNames->count(); i++)
            excludedNames.append(ui->lExcludedNames->item(i)->text());
        preferences->setExcludedSyncNames(excludedNames);

        QMessageBox::information(this, tr("Warning"), QString::fromUtf8("The new excluded file names will be taken into account\n"
                                                                        "when the application starts again."), QMessageBox::Ok);
        excludedNamesChanged = false;
    }

    //Proxies
/*  if(ui->rNoProxy->isChecked()) preferences->setProxyType(Preferences::PROXY_TYPE_NONE);
    else if(ui->rProxyAuto->isChecked()) preferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
    else preferences->setProxyType(Preferences::PROXY_TYPE_CUSTOM);

    preferences->setProxyProtocol(ui->cProxyType->currentIndex());
    preferences->setProxyServer(ui->eProxyServer->text().trimmed());
    preferences->setProxyPort(ui->eProxyPort->text().toInt());
    preferences->setProxyRequiresAuth(ui->cProxyRequiresPassword->isChecked());
    preferences->setProxyUsername(ui->eProxyUsername->text());
    preferences->setProxyPassword(ui->eProxyPassword->text());
*/

    ui->bApply->setEnabled(false);
}

void SettingsDialog::updateAddButton()
{
    if((ui->tSyncs->rowCount() == 1) && (ui->tSyncs->item(0, 1)->text().trimmed()==QString::fromAscii("/")))
    {
        ui->bAdd->setToolTip(tr("You are already syncing your entire account."));
        ui->bAdd->setEnabled(false);
    }
    else
    {
        ui->bAdd->setToolTip(QString::fromAscii(""));
        ui->bAdd->setEnabled(true);
    }
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
    syncNames.removeAt(index);

	syncsChanged = true;
	stateChanged();
    updateAddButton();
}

void SettingsDialog::loadSyncSettings()
{
    ui->tSyncs->clearContents();
    syncNames.clear();

    ui->tSyncs->horizontalHeader()->setVisible(true);
    int numFolders = preferences->getNumSyncedFolders();
    ui->tSyncs->setRowCount(numFolders);
    ui->tSyncs->setColumnCount(2);
    for(int i=0; i<numFolders; i++)
    {
        QTableWidgetItem *localFolder = new QTableWidgetItem();
        localFolder->setText(QString::fromAscii("  ") + preferences->getLocalFolder(i) + QString::fromAscii("  "));
        QTableWidgetItem *megaFolder = new QTableWidgetItem();
        megaFolder->setText(QString::fromAscii("  ") + preferences->getMegaFolder(i) + QString::fromAscii("  "));
        ui->tSyncs->setItem(i, 0, localFolder);
        ui->tSyncs->setItem(i, 1, megaFolder);
        syncNames.append(preferences->getSyncName(i));
    }
    updateAddButton();
}

void SettingsDialog::on_bAdd_clicked()
{
    QStringList currentLocalFolders;
    QList<long long> currentMegaFolders;
    for(int i=0; i<ui->tSyncs->rowCount(); i++)
    {
        QString localFolder = ui->tSyncs->item(i, 0)->text().trimmed();
        currentLocalFolders.append(localFolder);

        QString newMegaPath = ui->tSyncs->item(i, 1)->text().trimmed();
        Node *n = megaApi->getNodeByPath(newMegaPath.toUtf8().constData());
        if(!n) continue;
        currentMegaFolders.append(n->nodehandle);
    }

    BindFolderDialog *dialog = new BindFolderDialog(app, syncNames, currentLocalFolders, currentMegaFolders, this);
    int result = dialog->exec();
    if(result != QDialog::Accepted)
        return;

    QString localFolderPath = QDir(dialog->getLocalFolder()).canonicalPath();
    MegaApi *megaApi = app->getMegaApi();
    long long handle = dialog->getMegaFolder();
    Node *node = megaApi->getNodeByHandle(handle);
    if(!localFolderPath.length() || !node)
        return;

   QTableWidgetItem *localFolder = new QTableWidgetItem();
   localFolder->setText(QString::fromAscii("  ") + localFolderPath + QString::fromAscii("  "));
   QTableWidgetItem *megaFolder = new QTableWidgetItem();
   megaFolder->setText(QString::fromAscii("  ") +  QString::fromUtf8(megaApi->getNodePath(node)) + QString::fromAscii("  "));
   int pos = ui->tSyncs->rowCount();
   ui->tSyncs->setRowCount(pos+1);
   ui->tSyncs->setItem(pos, 0, localFolder);
   ui->tSyncs->setItem(pos, 1, megaFolder);
   syncNames.append(dialog->getSyncName());

   syncsChanged = true;
   stateChanged();
   updateAddButton();
}

void SettingsDialog::on_bApply_clicked()
{
    saveSettings();
}

void SettingsDialog::on_bUnlink_clicked()
{
    if(QMessageBox::question(this, tr("Logout"),
            tr("Synchronization will stop working.") + QString::fromAscii(" ") + tr("Are you sure?"),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
    {
        this->close();
        app->unlink();
    }
}

void SettingsDialog::on_tSyncs_doubleClicked(const QModelIndex &index)
{
    if(!index.column())
    {
        QString localFolderPath = ui->tSyncs->item(index.row(), 0)->text().trimmed();
        QDesktopServices::openUrl(QUrl::fromLocalFile(localFolderPath));
    }
    else
    {
        QString megaFolderPath = ui->tSyncs->item(index.row(), 1)->text().trimmed();
        Node *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
        if(node)
        {
            const char *handle = MegaApi::getBase64Handle(node);
            QString url = QString::fromAscii("https://mega.co.nz/#fm/") + QString::fromAscii(handle);
            QDesktopServices::openUrl(QUrl(url));
            delete handle;
        }
    }
}

void SettingsDialog::on_bUploadFolder_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, false, this);
    nodeSelector->nodesReady();
    int result = nodeSelector->exec();

    if(result != QDialog::Accepted)
        return;

    handle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    QString newPath = QString::fromUtf8(megaApi->getNodePath(megaApi->getNodeByHandle(selectedMegaFolderHandle)));
    if(newPath.compare(ui->eUploadFolder->text()))
    {
        ui->eUploadFolder->setText(newPath);
        stateChanged();
    }
}

void SettingsDialog::on_bAddName_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Excluded name"),
                                         tr("Please, enter a name to exclude from synchronization.\n(wildcards * and ? are allowed):"), QLineEdit::Normal,
                                         QString::fromAscii(""), &ok);

    text = text.trimmed();
    if (!ok || text.isEmpty()) return;

    QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::Wildcard);
    if(!regExp.isValid())
    {
        QMessageBox::warning(this, tr("Error"), QString::fromUtf8("You have entered an invalid file name or expression."), QMessageBox::Ok);
        return;
    }

    for(int i=0; i<ui->lExcludedNames->count(); i++)
    {
        if(ui->lExcludedNames->item(i)->text() == text)
            return;
        else if(ui->lExcludedNames->item(i)->text().compare(text, Qt::CaseInsensitive)>0)
        {
            ui->lExcludedNames->insertItem(i, text);
            excludedNamesChanged = true;
            stateChanged();
            return;
        }
    }

    ui->lExcludedNames->addItem(text);
    excludedNamesChanged = true;
    stateChanged();
}

void SettingsDialog::on_bDeleteName_clicked()
{
    cout << "bDelete clicked" << endl;
    QList<QListWidgetItem *> selected = ui->lExcludedNames->selectedItems();
    if(selected.size()==0)
    {
        cout << "No items selected" << endl;
        return;
    }

    for(int i=0; i<selected.size(); i++)
       delete selected[i];

    excludedNamesChanged = true;
    stateChanged();
}
