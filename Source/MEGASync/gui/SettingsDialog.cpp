#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>
#include <QTranslator>

#include "MegaApplication.h"
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "control/Utilities.h"
#include "platform/Platform.h"

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

long long calculateCacheSize()
{
    Preferences *preferences = Preferences::instance();
    long long cacheSize = 0;
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
    {
        QString syncPath = preferences->getLocalFolder(i);
        if(!syncPath.isEmpty())
            Utilities::getFolderSize(syncPath + QDir::separator() + QString::fromAscii("Rubbish"), &cacheSize);
    }
    return cacheSize;
}

void deleteCache()
{
    Preferences *preferences = Preferences::instance();
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
    {
        QString syncPath = preferences->getLocalFolder(i);
        if(!syncPath.isEmpty())
            Utilities::removeRecursively(QDir(syncPath + QDir::separator() + QString::fromAscii("Rubbish")));
    }
}

SettingsDialog::SettingsDialog(MegaApplication *app, bool proxyOnly, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->app = app;
    this->megaApi = app->getMegaApi();
    this->preferences = Preferences::instance();
	syncsChanged = false;
    excludedNamesChanged = false;
    this->proxyOnly = proxyOnly;
    this->proxyTestProgressDialog = NULL;
    proxyTestTimer.setSingleShot(true);
    connect(&proxyTestTimer, SIGNAL(timeout()), this, SLOT(onProxyTestTimeout()));
    networkAccess = NULL;
    shouldClose = false;

    ui->eProxyPort->setValidator(new QIntValidator(this));
    ui->eLimit->setValidator(new QDoubleValidator(this));
    ui->bAccount->setChecked(true);
    ui->wStack->setCurrentWidget(ui->pAccount);
#ifndef WIN32
    ui->cAutoUpdate->hide();
    ui->bUpdate->hide();
    ui->rProxyAuto->hide();
#endif

    if(!proxyOnly && preferences->logged())
    {
        connect(&cacheSizeWatcher, SIGNAL(finished()), this, SLOT(onCacheSizeAvailable()));
        QFuture<long long> futureCacheSize = QtConcurrent::run(calculateCacheSize);
        cacheSizeWatcher.setFuture(futureCacheSize);
    }

    ui->gCache->setVisible(false);
    setProxyOnly(proxyOnly);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setProxyOnly(bool proxyOnly)
{
    this->proxyOnly = proxyOnly;
    loadSettings();
    if(proxyOnly)
    {
        ui->bAccount->setEnabled(false);
        ui->bAccount->setChecked(false);
        ui->bAdvanced->setEnabled(false);
        ui->bAdvanced->setChecked(false);
        ui->bSyncs->setEnabled(false);
        ui->bSyncs->setChecked(false);
        ui->bBandwidth->setEnabled(false);
        ui->bBandwidth->setChecked(false);
        ui->bProxies->setChecked(true);
        ui->wStack->setCurrentWidget(ui->pProxies);

    }
    else
    {
        ui->bAccount->setEnabled(true);
        ui->bAdvanced->setEnabled(true);
        ui->bSyncs->setEnabled(true);
        ui->bBandwidth->setEnabled(true);
    }
}

void SettingsDialog::stateChanged()
{
    ui->bApply->setEnabled(true);
}

void SettingsDialog::onCacheSizeAvailable()
{
    long long cacheSize = cacheSizeWatcher.result();
    if(!cacheSize)
        return;

    ui->lCacheSize->setText(ui->lCacheSize->text().arg(Utilities::getSizeString(cacheSize)));
    ui->gCache->setVisible(true);
}

void SettingsDialog::on_bAccount_clicked()
{
    ui->bAccount->setChecked(true);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pAccount);
}

void SettingsDialog::on_bSyncs_clicked()
{
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(true);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pSyncs);
    ui->tSyncs->horizontalHeader()->setVisible( true );
}

void SettingsDialog::on_bBandwidth_clicked()
{
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(true);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pBandwidth);
}

void SettingsDialog::on_bAdvanced_clicked()
{
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(true);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pAdvanced);
}


void SettingsDialog::on_bProxies_clicked()
{
    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(true);
    ui->wStack->setCurrentWidget(ui->pProxies);
}


void SettingsDialog::on_bCancel_clicked()
{
    this->close();
}

void SettingsDialog::on_bOk_clicked()
{
    bool saved = true;
    if(ui->bApply->isEnabled())
        saved = saveSettings();

    if(saved)
        this->close();
    else
        shouldClose = true;
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
    if(!proxyOnly)
    {
        //General
        ui->cShowNotifications->setChecked(preferences->showNotifications());
        ui->cAutoUpdate->setChecked(preferences->updateAutomatically());

    #ifdef WIN32
        qt_ntfs_permission_lookup++; // turn checking on
    #endif
        if(!QFileInfo(MegaApplication::applicationFilePath()).isWritable())
            ui->cAutoUpdate->setEnabled(false);
    #ifdef WIN32
        qt_ntfs_permission_lookup--; // turn it off again
    #endif
        ui->cStartOnStartup->setChecked(preferences->startOnStartup());

        //Language
        ui->cLanguage->clear();
        languageCodes.clear();
        QString fullPrefix = MegaApplication::TRANSLATION_FOLDER+MegaApplication::TRANSLATION_PREFIX;
        QDirIterator it(MegaApplication::TRANSLATION_FOLDER);
        QStringList languages;
        languages.append(QString::fromAscii("English"));
        languageCodes.append(QString::fromAscii("en"));
        int currentIndex = -1;
        QString currentLanguage = preferences->language();
        while (it.hasNext())
        {
            QString file = it.next();
            if(file.startsWith(fullPrefix))
            {
                int extensionIndex = file.lastIndexOf(QString::fromAscii("."));
                if((extensionIndex-fullPrefix.size()) <= 0) continue;
                QString languageCode = file.mid(fullPrefix.size(), extensionIndex-fullPrefix.size());
                QString languageString = Utilities::languageCodeToString(languageCode);
                if(!languageString.isEmpty())
                {
                    int i=0;
                    while(i<languages.size() && (languageString > languages[i])) i++;
                    languages.insert(i, languageString);
                    languageCodes.insert(i, languageCode);
                }
            }
        }
        for(int i=languageCodes.size()-1; i>=0; i--)
        {
            if(currentLanguage.startsWith(languageCodes[i]))
            {
                currentIndex = i;
                break;
            }
        }
        if(currentIndex == -1)
            currentIndex = languageCodes.indexOf(QString::fromAscii("en"));

        ui->cLanguage->addItems(languages);
        ui->cLanguage->setCurrentIndex(currentIndex);

        int width = ui->bBandwidth->width();
        QFont f = ui->bBandwidth->font();
        QFontMetrics fm = QFontMetrics(f);
        int neededWidth = fm.width(tr("Bandwidth"));
        if(width < neededWidth)
            ui->bBandwidth->setText(tr("Transfers"));

        if(ui->lAutoLimit->text().trimmed().at(0)!=QChar::fromAscii('('))
            ui->lAutoLimit->setText(QString::fromAscii("(%1)").arg(ui->lAutoLimit->text().trimmed()));

        if(!preferences->canUpdate())
            ui->bUpdate->setEnabled(false);

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
            ui->lStorage->setText(tr("%1 (%2%) of %3 used")
                  .arg(Utilities::getSizeString(preferences->usedStorage()))
                  .arg(QString::number(percentage))
                  .arg(Utilities::getSizeString(preferences->totalStorage())));
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

        MegaNode *node = megaApi->getNodeByHandle(preferences->uploadFolder());
        if(!node) ui->eUploadFolder->setText(tr("/MEGAsync Uploads"));
        else
        {
            const char *nPath = megaApi->getNodePath(node);
            if(!nPath)
            {
                ui->eUploadFolder->setText(tr("/MEGAsync Uploads"));
            }
            else
            {
                ui->eUploadFolder->setText(QString::fromUtf8(nPath));
                delete nPath;
            }
        }
        delete node;

        //Syncs
        loadSyncSettings();

        //Bandwidth
        ui->rAutoLimit->setChecked(preferences->uploadLimitKB()<0);
        ui->rLimit->setChecked(preferences->uploadLimitKB()>0);
        ui->rNoLimit->setChecked(preferences->uploadLimitKB()==0);
        ui->eLimit->setText((preferences->uploadLimitKB()<=0)? QString::fromAscii("0") : QString::number(preferences->uploadLimitKB()));
        ui->eLimit->setEnabled(ui->rLimit->isChecked());

        double totalBandwidth = preferences->totalBandwidth();
        if(totalBandwidth == 0)
        {
            ui->pUsedBandwidth->setValue(0);
            ui->lBandwidth->setText(tr("Data temporarily unavailable"));
        }
        else
        {
            int bandwidthPercentage = 100*((double)preferences->usedBandwidth()/totalBandwidth);
            ui->pUsedBandwidth->setValue(bandwidthPercentage);
            ui->lBandwidth->setText(tr("%1 (%2%) of %3 used")
                    .arg(Utilities::getSizeString(preferences->usedBandwidth()))
                    .arg(QString::number(bandwidthPercentage))
                    .arg(Utilities::getSizeString(preferences->totalBandwidth())));
        }

        //Advanced
        ui->lExcludedNames->clear();
        QStringList excludedNames = preferences->getExcludedSyncNames();
        for(int i=0; i<excludedNames.size(); i++)
            ui->lExcludedNames->addItem(excludedNames[i]);
    }

    if(!proxyTestProgressDialog)
    {
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

        if(ui->rProxyManual->isChecked())
        {
            ui->cProxyType->setEnabled(true);
            ui->eProxyServer->setEnabled(true);
            ui->eProxyPort->setEnabled(true);
            ui->cProxyRequiresPassword->setEnabled(true);
        }
        else
        {
            ui->cProxyType->setEnabled(false);
            ui->eProxyServer->setEnabled(false);
            ui->eProxyPort->setEnabled(false);
            ui->cProxyRequiresPassword->setEnabled(false);
        }

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

    ui->bApply->setEnabled(false);
    this->update();
}

bool SettingsDialog::saveSettings()
{
    if(!proxyOnly)
    {
        //General
        preferences->setShowNotifications(ui->cShowNotifications->isChecked());

        bool updateAutomatically = ui->cAutoUpdate->isChecked();
        if(updateAutomatically != preferences->updateAutomatically())
        {
            preferences->setUpdateAutomatically(updateAutomatically);
            if(updateAutomatically)
                on_bUpdate_clicked();
        }

        bool startOnStartup = ui->cStartOnStartup->isChecked();
        Platform::startOnStartup(startOnStartup);
        preferences->setStartOnStartup(startOnStartup);

        //Language
        int currentIndex = ui->cLanguage->currentIndex();
        QString selectedLanguageCode = languageCodes[currentIndex];
        if(preferences->language() != selectedLanguageCode)
        {
            preferences->setLanguage(selectedLanguageCode);
            app->changeLanguage(selectedLanguageCode);
        }

        //Account
        MegaNode *node = megaApi->getNodeByPath(ui->eUploadFolder->text().toUtf8().constData());
        if(node && ui->eUploadFolder->text().compare(tr("/MEGAsync Uploads")))
            preferences->setUploadFolder(node->getHandle());
        delete node;

        //Syncs
        if(syncsChanged)
        {
            //Check for removed folders
            for(int i=0; i<preferences->getNumSyncedFolders(); i++)
            {
                QString localPath = preferences->getLocalFolder(i);
                mega::handle megaHandle = preferences->getMegaFolderHandle(i);

                int j;
                for(j=0; j<ui->tSyncs->rowCount(); j++)
                {
                    QString newLocalPath = ui->tSyncs->item(j, 0)->text().trimmed();
                    QString newMegaPath = ui->tSyncs->item(j, 1)->text().trimmed();
                    MegaNode *n = megaApi->getNodeByPath(newMegaPath.toUtf8().constData());
                    if(!n) continue;

                    if((n->getHandle() == megaHandle) && !localPath.compare(newLocalPath))
                    {
                        delete n;
                        break;
                    }
                    delete n;
                }

                if(j == ui->tSyncs->rowCount())
                {
                    LOG(QString::fromAscii("REMOVING SYNC: %1").arg(preferences->getSyncName(i)));
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i), preferences->getSyncName(i));
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
                MegaNode *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
                if(!node) continue;
                int j;
                for(j=0; j<preferences->getNumSyncedFolders(); j++)
                {
                    QString previousLocalPath = preferences->getLocalFolder(j);
                    mega::handle previousMegaHandle = preferences->getMegaFolderHandle(j);

                    if((node->getHandle() == previousMegaHandle) && !localFolderPath.compare(previousLocalPath))
                        break;
                }

                if(j == preferences->getNumSyncedFolders())
                {
                    LOG(QString::fromAscii("ADDING SYNC: %1 - %2").arg(localFolderPath).arg(megaFolderPath));
                    preferences->addSyncedFolder(localFolderPath,
                                                 megaFolderPath,
                                                 node->getHandle(),
                                                 syncName);
                    megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
                }
                delete node;
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

            vector<string> vExclusions;
            for(int i=0; i<excludedNames.size(); i++)
                vExclusions.push_back(excludedNames[i].toUtf8().constData());
            megaApi->setExcludedNames(&vExclusions);

            QMessageBox::information(this, tr("Warning"), tr("The new excluded file names will be taken into account\n"
                                                                            "when the application starts again."), QMessageBox::Ok);
            excludedNamesChanged = false;
        }
    }

    bool proxyChanged = false;
    //Proxies
    if( (ui->rNoProxy->isChecked() && (preferences->proxyType() != Preferences::PROXY_TYPE_NONE))       ||
        (ui->rProxyAuto->isChecked() &&  (preferences->proxyType() != Preferences::PROXY_TYPE_AUTO))    ||
        (ui->rProxyManual->isChecked() &&  (preferences->proxyType() != Preferences::PROXY_TYPE_CUSTOM))||
        (preferences->proxyProtocol() != ui->cProxyType->currentIndex())                                ||
        (preferences->proxyServer() != ui->eProxyServer->text().trimmed())                              ||
        (preferences->proxyPort() != ui->eProxyPort->text().toInt())                                    ||
        (preferences->proxyRequiresAuth() != ui->cProxyRequiresPassword->isChecked())                   ||
        (preferences->getProxyUsername() != ui->eProxyUsername->text())                                 ||
        (preferences->getProxyPassword() != ui->eProxyPassword->text()))
    {
        proxyChanged = true;
        LOG("New proxy settings");
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::NoProxy);
        if(ui->rProxyManual->isChecked())
        {
            LOG("Manual proxy");
            LOG(ui->eProxyServer->text().trimmed());
            LOG(ui->eProxyPort->text().trimmed());
            proxy.setType(QNetworkProxy::HttpProxy);
            proxy.setHostName(ui->eProxyServer->text().trimmed());
            proxy.setPort(ui->eProxyPort->text().trimmed().toInt());
            if(ui->cProxyRequiresPassword->isChecked())
            {
                LOG("Auth proxy");
                LOG(ui->eProxyUsername->text());
                LOG(ui->eProxyPassword->text());
                proxy.setUser(ui->eProxyUsername->text());
                proxy.setPassword(ui->eProxyPassword->text());
            }
        }
        else if(ui->rProxyAuto->isChecked())
        {
            LOG("Auto proxy");
            MegaProxySettings *proxySettings = megaApi->getAutoProxySettings();
            if(proxySettings->getProxyType()==MegaProxySettings::CUSTOM)
            {
                LOG("Custom proxy");
                string sProxyURL = proxySettings->getProxyURL();
                QString proxyURL = QString::fromUtf8(sProxyURL.data());
                LOG(proxyURL);
                QStringList arguments = proxyURL.split(QString::fromAscii(":"));
                if(arguments.size() == 2)
                {
                    LOG(arguments[0]);
                    LOG(arguments[1]);
                    proxy.setType(QNetworkProxy::HttpProxy);
                    proxy.setHostName(arguments[0]);
                    proxy.setPort(arguments[1].toInt());
                }
            }
            else LOG("No proxy");

            delete proxySettings;
        }
        else LOG("No proxy");

        QNetworkRequest proxyTestRequest(Preferences::PROXY_TEST_URL);
        proxyTestRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                             QVariant( int(QNetworkRequest::AlwaysNetwork)));
        networkAccess = new QNetworkAccessManager();
        connect(networkAccess, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(onProxyTestFinished(QNetworkReply*)));
        connect(networkAccess, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
                this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

        networkAccess->setProxy(proxy);
        networkAccess->get(proxyTestRequest);
        proxyTestProgressDialog = new MegaProgressDialog(tr("Please wait..."), QString(), 0, 0, this, Qt::CustomizeWindowHint|Qt::WindowTitleHint);
        proxyTestProgressDialog->setWindowModality(Qt::WindowModal);
        proxyTestProgressDialog->show();
        LOG("Testing proxy settings...");
        proxyTestTimer.start(5000);
    }

    ui->bApply->setEnabled(false);
    return !proxyChanged;
}

void SettingsDialog::updateAddButton()
{
    if((ui->tSyncs->rowCount() == 1) && (ui->tSyncs->item(0, 1)->text().trimmed()==QString::fromAscii("/")))
    {
        ui->tSyncs->hide();
        ui->wSyncsButtons->hide();
        ui->lSyncType->setText(tr("Full account sync active"));
        ui->lSyncText->setText(tr("Disabling full account sync will allow you to set up selective folder syncing"));
        ui->bSyncChange->setText(tr("Disable full account sync"));
    }
    else
    {
        ui->tSyncs->show();
        ui->wSyncsButtons->show();
        ui->lSyncType->setText(tr("Selective sync active"));
        ui->lSyncText->setText(tr("Enabling full account sync will disable all your current syncs"));
        ui->bSyncChange->setText(tr("Enable full account sync"));
    }
}

void SettingsDialog::on_bSyncChange_clicked()
{
    if((ui->tSyncs->rowCount() == 1) && (ui->tSyncs->item(0, 1)->text().trimmed()==QString::fromAscii("/")))
    {
        ui->tSyncs->clearContents();
        ui->tSyncs->setRowCount(0);
        syncNames.clear();
        syncsChanged = true;
        stateChanged();
    }
    else
    {
        ui->tSyncs->clearContents();
        ui->tSyncs->setRowCount(0);
        syncNames.clear();
        syncsChanged = true;
        stateChanged();

    #if QT_VERSION < 0x050000
        QString localFolderPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    #else
        QString localFolderPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
    #endif

        localFolderPath.append(QString::fromAscii("/MEGA"));
        localFolderPath = QDir::toNativeSeparators(localFolderPath);
        QDir defaultFolder(localFolderPath);
        defaultFolder.mkpath(QString::fromAscii("."));

        QTableWidgetItem *localFolder = new QTableWidgetItem();
        localFolder->setText(QString::fromAscii("  ") + localFolderPath + QString::fromAscii("  "));

        QTableWidgetItem *megaFolder = new QTableWidgetItem();
        megaFolder->setText(QString::fromAscii("  ") +  QString::fromUtf8("/") + QString::fromAscii("  "));

        int pos = ui->tSyncs->rowCount();
        ui->tSyncs->setRowCount(pos+1);
        ui->tSyncs->setItem(pos, 0, localFolder);
        ui->tSyncs->setItem(pos, 1, megaFolder);
        syncNames.append(QString::fromAscii("MEGA"));
    }

    updateAddButton();
}


void SettingsDialog::on_bDelete_clicked()
{
    QList<QTableWidgetSelectionRange> selected = ui->tSyncs->selectedRanges();
    if(selected.size()==0) return;

    int index = selected.first().topRow();
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
        MegaNode *n = megaApi->getNodeByPath(newMegaPath.toUtf8().constData());
        if(!n) continue;
        currentMegaFolders.append(n->getHandle());
        delete n;
    }

    BindFolderDialog *dialog = new BindFolderDialog(app, syncNames, currentLocalFolders, currentMegaFolders, this);
    int result = dialog->exec();
    if(result != QDialog::Accepted)
        return;

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    long long handle = dialog->getMegaFolder();
    MegaNode *node = megaApi->getNodeByHandle(handle);
    if(!localFolderPath.length() || !node)
    {
        delete node;
        return;
    }
   QTableWidgetItem *localFolder = new QTableWidgetItem();
   localFolder->setText(QString::fromAscii("  ") + localFolderPath + QString::fromAscii("  "));
   QTableWidgetItem *megaFolder = new QTableWidgetItem();
   const char *nPath = megaApi->getNodePath(node);
   if(!nPath)
   {
       delete node;
       return;
   }

   megaFolder->setText(QString::fromAscii("  ") +  QString::fromUtf8(nPath) + QString::fromAscii("  "));
   int pos = ui->tSyncs->rowCount();
   ui->tSyncs->setRowCount(pos+1);
   ui->tSyncs->setItem(pos, 0, localFolder);
   ui->tSyncs->setItem(pos, 1, megaFolder);
   syncNames.append(dialog->getSyncName());
   delete node;
   delete nPath;

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
        MegaNode *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
        if(node)
        {
            const char *handle = node->getBase64Handle();
            QString url = QString::fromAscii("https://mega.co.nz/#fm/") + QString::fromAscii(handle);
            QDesktopServices::openUrl(QUrl(url));
            delete handle;
            delete node;
        }
    }
}

void SettingsDialog::on_bUploadFolder_clicked()
{
    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, false, this);
    nodeSelector->nodesReady();
    int result = nodeSelector->exec();
    if(result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    mega::handle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
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

    QString newPath = QString::fromUtf8(nPath);
    delete nodeSelector;
    delete nPath;
    delete node;
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
                                         tr("Enter a name to exclude from synchronization.\n(wildcards * and ? are allowed):"), QLineEdit::Normal,
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
    QList<QListWidgetItem *> selected = ui->lExcludedNames->selectedItems();
    if(selected.size()==0) return;

    for(int i=0; i<selected.size(); i++)
       delete selected[i];

    excludedNamesChanged = true;
    stateChanged();
}

void SettingsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        loadSettings();
    }
    QDialog::changeEvent(event);
}

void SettingsDialog::on_bClearCache_clicked()
{
    QtConcurrent::run(deleteCache);
    ui->gCache->setVisible(false);
}

void SettingsDialog::onProxyTestTimeout()
{
    if(networkAccess)
    {
        networkAccess->deleteLater();
        networkAccess = NULL;
    }

    if(proxyTestProgressDialog)
    {
        proxyTestProgressDialog->hide();
        delete proxyTestProgressDialog;
        proxyTestProgressDialog = NULL;
        ui->bApply->setEnabled(true);
        QMessageBox::critical(this, tr("Error"), tr("Your proxy settings are invalid or the proxy doesn't respond"));
    }

    shouldClose = false;
}

void SettingsDialog::onProxyTestFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        onProxyTestTimeout();
        return;
    }

    if(ui->rNoProxy->isChecked())
        preferences->setProxyType(Preferences::PROXY_TYPE_NONE);
    else if(ui->rProxyAuto->isChecked())
        preferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
    else if(ui->rProxyManual->isChecked())
        preferences->setProxyType(Preferences::PROXY_TYPE_CUSTOM);

    preferences->setProxyProtocol(ui->cProxyType->currentIndex());
    preferences->setProxyServer(ui->eProxyServer->text().trimmed());
    preferences->setProxyPort(ui->eProxyPort->text().toInt());
    preferences->setProxyRequiresAuth(ui->cProxyRequiresPassword->isChecked());
    preferences->setProxyUsername(ui->eProxyUsername->text());
    preferences->setProxyPassword(ui->eProxyPassword->text());

    app->applyProxySettings();
    megaApi->retryPendingConnections();

    if(networkAccess)
    {
        networkAccess->deleteLater();
        networkAccess = NULL;
    }

    if(proxyTestProgressDialog)
    {
        proxyTestProgressDialog->hide();
        delete proxyTestProgressDialog;
        proxyTestProgressDialog = NULL;
    }

    if(shouldClose)
    {
        shouldClose = false;
        this->close();
    }
    else loadSettings();
}

void SettingsDialog::onProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth)
{
    if(ui->rProxyManual->isChecked() && ui->cProxyRequiresPassword->isChecked())
    {
        auth->setUser(ui->eProxyUsername->text());
        auth->setPassword(ui->eProxyPassword->text());
    }
}

void SettingsDialog::on_bUpdate_clicked()
{
    app->checkForUpdates();
}

MegaProgressDialog::MegaProgressDialog(const QString &labelText, const QString &cancelButtonText, int minimum, int maximum, QWidget *parent, Qt::WindowFlags f) :
    QProgressDialog(labelText, cancelButtonText, minimum, maximum, parent, f) {}

void MegaProgressDialog::reject() {}
void MegaProgressDialog::closeEvent(QCloseEvent * event)
{
    event->ignore();
}
