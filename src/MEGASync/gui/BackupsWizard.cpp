#include "BackupsWizard.h"
#include "ui_BackupsWizard.h"
#include "ui_BackupSetupSuccessDialog.h"
#include "MegaApplication.h"
#include "megaapi.h"
#include "QMegaMessageBox.h"

#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>
//constexpr int HEIGHT_S1 (412);
//constexpr int HEIGHT_MIN_S2 (296);
//constexpr int HEIGHT_MAX_S2 (465);

BackupsWizard::BackupsWizard(QWidget* parent) :
    QDialog(parent),
    mUi (new Ui::BackupsWizard),
    mCurrentStep (Steps::STEP_1_INIT),
    mSyncsModel (SyncModel::instance()),
    mSyncController(),
    mCreateBackupsDir (false),
    mDeviceDirHandle (mega::INVALID_HANDLE),
    mBackupsDirName (),
    mHaveBackupsDir (false),
    mDeviceName (),
    mHaveDeviceName (false),
    mError (false),
    mUserCancelled (false),
    mStep1FoldersModel (new QStandardItemModel()),
    mCurrentSyncIdx (0),
    mSuccessDialog (nullptr),
    mSuccessDialogUi (nullptr)
{
    mUi->setupUi(this);
    mHighDpiResize.init(this);

    mUi->lvFoldersS1->setModel(mStep1FoldersModel);

    // Empty state
    mOriginalState = getCurrentState();

    connect(this, &BackupsWizard::nextStep,
            this, &BackupsWizard::onNextStep, Qt::QueuedConnection);

    connect(&mSyncController, &SyncController::deviceName,
            this, &BackupsWizard::onDeviceNameSet);

    connect(&mSyncController, &SyncController::backupsRootDirHandle,
            this, &BackupsWizard::onBackupsDirSet);

    // React on item check/uncheck
    connect(mStep1FoldersModel, &QStandardItemModel::itemChanged,
            this, &BackupsWizard::onListItemChanged);

    connect(&mSyncController, &SyncController::setMyBackupsDirStatus,
            this, &BackupsWizard::onSetMyBackupsDirRequestStatus);

    connect(&mSyncController, &SyncController::setDeviceDirStatus,
            this, &BackupsWizard::onSetDeviceNameRequestStatus);

    connect(&mSyncController, &SyncController::syncAddStatus,
            this, &BackupsWizard::onSyncAddRequestStatus);

    // Go to Step 1
    setupStep1();
}

BackupsWizard::~BackupsWizard()
{
    delete mUi;
}

void BackupsWizard::refreshNextButtonState()
{
    bool enable (false);
    switch (mCurrentStep)
    {
        case STEP_1:
        {
            enable = (getCurrentState() != mOriginalState) && mHaveDeviceName;
            break;
        }
        case STEP_2:
        {
            enable = mHaveBackupsDir;
            break;
        }
        default:
        {
            enable = false;
            break;
        }
    }
    mUi->bNext->setEnabled(enable);
}

void BackupsWizard::setupStep1()
{
//    setFixedHeight(HEIGHT_S1);

    refreshNextButtonState();
    mUi->sSteps->setCurrentWidget(mUi->pStep1);
    mUi->bCancel->setEnabled(true);
    mUi->bNext->setText(tr("Next"));
    mUi->bBack->hide();

    // Get device name
    mHaveDeviceName = false;
    mSyncController.ensureDeviceNameIsSetOnRemote();

    bool isRemoteRootSynced(mSyncsModel->isRemoteRootSynced());
    if (isRemoteRootSynced)
    {
        mUi->sMoreFolders->setCurrentWidget(mUi->pAllFoldersSynced);
        mUi->sFolders->setCurrentWidget(mUi->pNoFolders);
        // Remove this, otherwise the height doens't go below 8x px??
        mUi->sFolders->removeWidget(mUi->pFolders);
        mUi->sMoreFolders->removeWidget(mUi->pMoreFolders);
        mUi->pFolders->setSizePolicy(mUi->pFolders->sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);
        mUi->fFoldersS1->setSizePolicy(mUi->fFoldersS1->sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);
        mUi->sFolders->setSizePolicy(mUi->sFolders->sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);
        mUi->pStep1->setSizePolicy(mUi->pStep1->sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);
        mUi->sSteps->setSizePolicy(mUi->sSteps->sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);
        setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Maximum);
    }
    else
    {
        mUi->sMoreFolders->setCurrentWidget(mUi->pMoreFolders);
        mUi->sFolders->setCurrentWidget(mUi->pFolders);

        // Check if we need to refresh the lists
        if (mStep1FoldersModel->rowCount() == 0)
        {
            QIcon folderIcon (QIcon(QLatin1String("://images/folder_icon.png")));

            for (auto type : {
                 QStandardPaths::DocumentsLocation,
                 //QStandardPaths::MusicLocation,
                 QStandardPaths::MoviesLocation,
                 QStandardPaths::PicturesLocation,
                 //QStandardPaths::DownloadLocation,
        })
            {
                const auto standardPaths (QStandardPaths::standardLocations(type));
                QDir dir (standardPaths.first());
                if (dir.exists() && dir != QDir::home() && !isFolderAlreadySynced(dir.canonicalPath()))
                {
                    QStandardItem* item (new QStandardItem(dir.dirName()));
                    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                    item->setData(QDir::toNativeSeparators(dir.canonicalPath()), Qt::ToolTipRole);
                    item->setData(QDir::toNativeSeparators(dir.canonicalPath()), Qt::UserRole);
                    item->setData(folderIcon, Qt::DecorationRole);
                    item->setData(Qt::Unchecked, Qt::CheckStateRole);
//                    item->setData(QSize(-1, 40), Qt::SizeHintRole);

                    mStep1FoldersModel->appendRow(item);
                }
            }
            // Snapshot original state
            mOriginalState = getCurrentState();
        }
        mUi->lvFoldersS1->style()->polish(mUi->lvFoldersS1);
        mUi->lvFoldersS1->adjustSize();
    }

    adjustSize();

    mCurrentStep = Steps::STEP_1;
}

void BackupsWizard::setupStep2()
{
    refreshNextButtonState();
    mUi->sSteps->setCurrentWidget(mUi->pStep2);
    mUi->bNext->setText(tr("Setup"));
    mUi->bBack->show();

    // Request MyBackups root folder
    mHaveBackupsDir = false;
    mSyncController.getBackupsRootDirHandle();

    // Get number of items
    int nbFolders (mStep1FoldersModel->rowCount());
    int nbSelectedFolders (0);
    mUi->lvFoldersS2->clear();
    // Populate lvFoldersS2 from lvFoldersS1
    for (int i = 0; i < nbFolders; ++i)
    {
        auto item (mStep1FoldersModel->itemData(mStep1FoldersModel->index(i, 0)));
        QListWidgetItem* itemS2 (new QListWidgetItem());

        itemS2->setData(Qt::DisplayRole, item[Qt::DisplayRole]);
        itemS2->setData(Qt::ToolTipRole, item[Qt::ToolTipRole]);
        itemS2->setData(Qt::UserRole, item[Qt::UserRole]);
        itemS2->setData(Qt::DecorationRole, item[Qt::DecorationRole]);
//        itemS2->setData(Qt::SizeHintRole, QSize(-1, 32));
        mUi->lvFoldersS2->addItem(itemS2);

        // Hide unchecked items and count selected ones
        if (item[Qt::CheckStateRole] == Qt::Checked)
        {
            nbSelectedFolders++;
        }
        else
        {
            itemS2->setHidden(true);
        }
    }

    // Set folders number
    if (nbSelectedFolders == 1)
    {
        mUi->lFoldersNumber->setText(tr("1 folder"));
        setProperty("S2OneItem", true);
    }
    else
    {
        mUi->lFoldersNumber->setText(tr("%1 folders").arg(nbSelectedFolders));
        setProperty("S2OneItem", false);
    }

//    setFixedHeight(std::min(HEIGHT_MAX_S2, HEIGHT_MIN_S2 + nbSelectedFolders * 32));
    mUi->lvFoldersS2->style()->polish(mUi->lvFoldersS2);
    mUi->lvFoldersS2->adjustSize();
    mUi->fFoldersS2->adjustSize();
    mUi->pStep2->adjustSize();
    adjustSize();

    mCurrentStep = Steps::STEP_2;
}

void BackupsWizard::setupFinalize()
{
    refreshNextButtonState();
    mUi->bCancel->setEnabled(false);
    mUi->bBack->hide();

    mCurrentSyncIdx = 0;
    mError = false;

    mCurrentStep = Steps::SETUP_MYBACKUPS;
    emit nextStep();
}

void BackupsWizard::setupMyBackupsDir(bool nameCollision)
{
    // If the user cancels, exit wizard
    if (mBackupsDirName.isEmpty())
    {
        mUserCancelled = true;
        mCurrentStep = Steps::EXIT;
        emit nextStep();
    }
    else
    {
        // Create MyBackups folder if necessary
        if (mCreateBackupsDir || nameCollision)
        {
            mSyncController.createMyBackupsDir(mBackupsDirName);
        }
        else
        {
            // If not, proceed to setting-up backups
            mCurrentStep = Steps::SETUP_DEVICE_NAME;
            emit nextStep();
        }
    }
}

void BackupsWizard::setupDeviceName()
{
    mSyncController.setDeviceName(mDeviceName);
}

void BackupsWizard::setupBackups()
{
    if (mCurrentSyncIdx < mUi->lvFoldersS2->count())
    {
        auto item (mUi->lvFoldersS2->item(mCurrentSyncIdx));

        // Only process non-hidden items (those who were checked in STEP_1)
        if (!item->isHidden())
        {
            // Get local folder path
            QString localFolderPath (item->data(Qt::UserRole).toString());

            // Get Sync Name
            QString currentBackupName (item->data(Qt::DisplayRole).toString());

            // Create backup
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                               QString::fromUtf8("Adding backup %1 to %2 from BackupsWizard:")
                               .arg(localFolderPath,
                                    mBackupsDirName +  QLatin1Char('/')
                                    + mDeviceName + QLatin1Char('/')
                                    + currentBackupName).toUtf8().constData());

            mSyncController.addSync(localFolderPath, mega::INVALID_HANDLE,
                                               currentBackupName, mega::MegaSync::TYPE_BACKUP);
        }
        else
        {
            mCurrentSyncIdx++;
            emit nextStep();
        }
    }
    else
    {
        // If we reached the end o fthe list, we're done.
        mCurrentStep = Steps::DONE;
        emit nextStep();
    }
}

// Create folder check map: "-X XX   X". 'X' == checked, ' ' == unchecked.
// Pre-requities:
// 1. The order of existing items in the list do not change
// 2. New items are inserted at the beginning of the list
QString BackupsWizard::getCurrentState()
{
    QString currState (QLatin1Char('-'));
    int nbFolders (mStep1FoldersModel->rowCount());
    for (int i = nbFolders - 1; i >= 0; --i)
    {
        currState += mStep1FoldersModel->data(mStep1FoldersModel->index(i, 0), Qt::CheckStateRole) == Qt::Unchecked ?
                         QLatin1Char(' ')
                       : QLatin1Char('X');
    }
    return currState.trimmed();
}

// Toggle the backup's state in the original state map.
// To be used after an action succeed.
void BackupsWizard::updateOriginalState(int index)
{
    int nbBackups (mStep1FoldersModel->rowCount());
    int nbMeaningfulRows (mOriginalState.size() - 1);
    int pos (nbBackups - index);

    if (nbMeaningfulRows < pos)
    {
        mOriginalState.resize(pos + 1, QLatin1Char(' '));
        mOriginalState[pos] = 'X';
    }
    else
    {
        mOriginalState[pos] = mOriginalState[pos] == QLatin1Char(' ') ? 'X' : ' ';
        mOriginalState = mOriginalState.trimmed();
    }
}

// Checks if the name is already used (remote)
// returns true if unique, false if collision
// <displayName> is set to "" if the user cancels
bool BackupsWizard::promptAndEnsureUniqueRemoteName(QString& displayName, bool forcePrompt)
{    
    bool nameCollision (false);
    auto api (MegaSyncApp->getMegaApi());
    std::unique_ptr<mega::MegaNode> deviceDirNode;
    bool showPromptAtLeastOnce(forcePrompt);

    if (mDeviceDirHandle != mega::INVALID_HANDLE)
    {
        deviceDirNode.reset(api->getNodeByHandle(mDeviceDirHandle));
    }

    do
    {
        std::shared_ptr<mega::MegaNode> dirNode;
        // Check for name collision
        if (deviceDirNode)
        {
            dirNode.reset(api->getChildNode(deviceDirNode.get(),
                                            displayName.toUtf8()));
            nameCollision = dirNode.get();
        }

        if (nameCollision || showPromptAtLeastOnce)
        {
            displayName = remoteFolderExistsDialog(displayName,
                                                   RenameTargetFolderDialog::TYPE_BACKUP_FOLDER,
                                                   dirNode);
            nameCollision = !displayName.isEmpty();
            showPromptAtLeastOnce = false;
        }
    }
    while (nameCollision && !displayName.isEmpty());

    return !nameCollision;
}

// Checks if a path belongs is in an existing sync or backup tree; and if the selected
// folder has a sync or backup ii its tree.
// Special case for backups: if the exact path is already backed up, handle it later.
// Returns true if folder is already synced or backed up (backup: except exact path),
// false if not.
bool BackupsWizard::isFolderAlreadySynced(const QString& path, bool displayWarning)
{
    auto cleanInputPath (QDir::cleanPath(QDir(path).canonicalPath()));
    QString message;

    // Gather all synced or backed-up dirs
    QStringList localFolders;
    for (auto type : {mega::MegaSync::SyncType::TYPE_TWOWAY,
         mega::MegaSync::SyncType::TYPE_BACKUP})
    {
        localFolders += mSyncsModel->getLocalFolders(type);
    }

    // First check existing syncs
    auto lf (localFolders.cbegin());
    while (message.isEmpty() && lf != localFolders.cend())
    {        
        QString c = QDir::cleanPath(*lf);

        if (cleanInputPath.startsWith(c)
                && (c.size() == cleanInputPath.size()
                    || cleanInputPath[c.size()] == QDir::separator()))
        {
            message = tr("The selected local folder is already synced");
        }
        else if (c.startsWith(cleanInputPath)
                 && (c[cleanInputPath.size()] == QDir::separator()
                 || cleanInputPath == QDir(*lf).rootPath()))
        {
            message = tr("A synced folder cannot be inside a backup folder");
        }
        lf++;
    }

    // Then check current list
    if (message.isEmpty())
    {
        // Check for path and name collision.
        int nbBackups (mStep1FoldersModel->rowCount());
        int idx (0);

        while (message.isEmpty() && idx < nbBackups)
        {
            QString c (QDir::cleanPath(mStep1FoldersModel->item(idx)->data(Qt::UserRole).toString()));
            bool sameSize (cleanInputPath.size() == c.size());

            // Do not consider unchecked items
            if (mStep1FoldersModel->item(idx)->checkState() == Qt::Checked)
            {
                // Handle same path another way later: by selecting the row in the view.
                if (cleanInputPath.startsWith(c) && !sameSize
                        && (c.size() == cleanInputPath.size()
                            || cleanInputPath[c.size()] == QDir::separator()))
                {
                    message = tr("The selected local folder is already backed up");
                }
                else if (c.startsWith(cleanInputPath) && !sameSize
                         && (c[cleanInputPath.size()] == QDir::separator()
                             || cleanInputPath == QDir(*lf).rootPath()))
                {
                    message = tr("A backed up folder cannot be inside a backup folder");
                }
            }
            idx++;
        }
    }

    if (displayWarning && !message.isEmpty())
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), message, QMessageBox::Ok);
    }

    return (!message.isEmpty());
}

// Returns new backup name if new name set; empty string if backup canceled.
QString BackupsWizard::remoteFolderExistsDialog(const QString& backupName,
                                                RenameTargetFolderDialog::FolderType type,
                                                std::shared_ptr<mega::MegaNode> node)
{
    QString newName(QLatin1String(""));
    RenameTargetFolderDialog dialog (backupName, type, node);

    auto res (dialog.exec());

    if (res == QDialog::Accepted)
    {
        newName = dialog.getNewFolderName();
    }

    return newName;
}

// State machine orchestrator
void BackupsWizard::onNextStep()
{
    refreshNextButtonState();
    switch (mCurrentStep)
    {
        case Steps::STEP_1_INIT:
        {
            setupStep1();
            break;
        }
        case Steps::STEP_2_INIT:
        {
            setupStep2();
            break;
        }
        case Steps::FINALIZE:
        {
            setupFinalize();
            break;
        }
        case Steps::SETUP_MYBACKUPS:
        {
            setupMyBackupsDir();
            break;
        }
        case Steps::SETUP_DEVICE_NAME:
        {
            setupDeviceName();
            break;
        }
        case Steps::SETUP_BACKUPS:
        {
            setupBackups();
            break;
        }
        case Steps::DONE:
        {
            setupComplete();
            break;
        }
        case Steps::EXIT:
        {
            mUserCancelled ? reject() : accept();
            break;
        }
        default:
            break;
    }
    refreshNextButtonState();
}

void BackupsWizard::on_bNext_clicked()
{
    switch (mCurrentStep)
    {
        case Steps::STEP_1:
        {
            mCurrentStep = Steps::STEP_2_INIT;
            break;
        }
        case Steps::STEP_2:
        {
            mCurrentStep = Steps::FINALIZE;
            break;
        }
        default:
            break;
    }
    emit nextStep();
}

void BackupsWizard::on_bCancel_clicked()
{
    int userWantsToCancel (QMessageBox::Ok);

    // If the user has made any modification, warn him before exiting.
    if (getCurrentState() != mOriginalState)
    {
        QString title (tr("Warning"));
        QString content (tr("If you cancel, all changes will be lost."));
        userWantsToCancel = QMessageBox::warning(this, title, content,
                                                 QMessageBox::Ok|QMessageBox::Abort,
                                                 QMessageBox::Abort);
    }

    if (userWantsToCancel == QMessageBox::Ok)
    {
        mUserCancelled = true;
        mCurrentStep = Steps::EXIT;
        emit nextStep();
    }
}

void BackupsWizard::on_bMoreFolders_clicked()
{
    const auto homePaths (QStandardPaths::standardLocations(QStandardPaths::HomeLocation));
    QString d (QFileDialog::getExistingDirectory(this,
                                                 tr("Choose Directory"),
                                                 homePaths.first(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks));
    QDir dir (d);
    if (!d.isEmpty() && dir.exists() && !isFolderAlreadySynced(dir.canonicalPath(), true))
    {
        QString path (QDir::toNativeSeparators(dir.canonicalPath()));
        QStandardItem* existingBackup (nullptr);
        QString displayName (dir.dirName());
        bool nameCollision (false);
        do
        {
            // Check for path and name collision.
            int nbBackups (mStep1FoldersModel->rowCount());
            int idx (0);
            nameCollision = false;

            while ((!nameCollision || existingBackup == nullptr) && idx < nbBackups)
            {
                auto currItem = mStep1FoldersModel->itemData(mStep1FoldersModel->index(idx, 0));
                QString name (currItem[Qt::DisplayRole].toString());
                if (path != currItem[Qt::UserRole].toString())
                {
                    // Folder is not backed up, set pointer and check name collision
                    nameCollision |= (displayName == name);
                }
                else
                {
                    // If folder is already backed up, set pointer and take existing name
                    existingBackup = mStep1FoldersModel->item(idx);
                    displayName = name;
                }
                idx++;
            }

            // Prompt for new name if collision
            // + ensure no remote collision
            if (existingBackup == nullptr
                    && (mDeviceDirHandle != mega::INVALID_HANDLE || nameCollision))
            {
                QString originalName (displayName);
                promptAndEnsureUniqueRemoteName(displayName, nameCollision);
                // Re-check local collision if name changed (because of remote collision)
                if (displayName != originalName)
                {
                    nameCollision = true;
                }
            }
        }
        while (existingBackup == nullptr && nameCollision && !displayName.isEmpty());

        // Add backup if the user didn't cancel
        if (!displayName.isEmpty())
        {
            QStandardItem* item (nullptr);

            if (existingBackup != nullptr)
            {
                item = existingBackup;
            }
            else
            {
                QIcon icon (QIcon(QLatin1String("://images/folder_icon.png")));
                item = new QStandardItem(displayName);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setData(path, Qt::ToolTipRole);
                item->setData(path, Qt::UserRole);
                item->setData(icon, Qt::DecorationRole);
                mStep1FoldersModel->insertRow(0, item);
            }
            item->setData(Qt::Checked, Qt::CheckStateRole);

            // Jump to item in list
            mUi->lvFoldersS1->scrollTo(mStep1FoldersModel->indexFromItem(item));
        }
        refreshNextButtonState();
    }
}

void BackupsWizard::on_bBack_clicked()
{    
    // The "Back" button only appears at STEP_2. Go back to STEP_1_INIT.
    mCurrentStep = Steps::STEP_1_INIT;
    emit nextStep();
}

void BackupsWizard::onListItemChanged(QStandardItem* item)
{
    QString displayName (item->data(Qt::DisplayRole).toString());

    if (item->checkState() == Qt::Checked)
    {
        promptAndEnsureUniqueRemoteName(displayName);

        // Add backup if the user didn't cancel
        if (!displayName.isEmpty())
        {
            item->setData(displayName, Qt::DisplayRole);
        }
        else
        {
            item->setData(Qt::Unchecked, Qt::CheckStateRole);
        }
    }
    refreshNextButtonState();
}

void BackupsWizard::displayError(const QString& message)
{
    QMegaMessageBox::critical(nullptr, tr("Error"), message);
}

void BackupsWizard::setupComplete()
{
    if (!mError)
    {
        // We are now done, exit
        mCurrentStep = Steps::EXIT;

        // No error: show success message!
        mSuccessDialog.reset(new QDialog(this));
        mSuccessDialogUi.reset(new Ui::BackupSetupSuccessDialog);

        mSuccessDialogUi->setupUi(mSuccessDialog.get());

        connect(mSuccessDialog.get(), &QDialog::rejected,
                this, &BackupsWizard::onNextStep);
        connect(mSuccessDialog.get(), &QDialog::accepted,
                this, &BackupsWizard::onSuccessDialogAccepted);

        mSuccessDialog->exec();
    }
    else
    {
        // Error: go back to Step 1 :(
        mCurrentStep = Steps::STEP_1_INIT;
        emit nextStep();
    }
}

void BackupsWizard::onDeviceNameSet(QString deviceName)
{
    mDeviceName = deviceName;
    mUi->lDeviceNameS1->setText(mDeviceName);
    mUi->lDeviceNameS2->setText(mDeviceName);
    mHaveDeviceName = true;
    refreshNextButtonState();
}

void BackupsWizard::onBackupsDirSet(mega::MegaHandle backupsDirHandle)
{
    mHaveBackupsDir = true;
    QString backupsDirPath;

    if (backupsDirHandle != mega::INVALID_HANDLE)
    {
        // We still have to check if the folder exists... try to get its pah
        auto api (MegaSyncApp->getMegaApi());
        backupsDirPath = QString::fromUtf8(api->getNodePathByNodeHandle(backupsDirHandle));

        if (!backupsDirPath.isEmpty())
        {
            // If the folder exists, take note
            mBackupsDirName = QDir(backupsDirPath).dirName();
            mCreateBackupsDir = false;
        }
    }

    // If backupsDirPath is empty, the remote folder does not exist and we have to create it.
    if (backupsDirPath.isEmpty())
    {
        // If remote dir does not exist, use default name and program its creation
        mBackupsDirName = tr("My Backups");
        backupsDirPath = QLatin1Char('/') + mBackupsDirName;
        mCreateBackupsDir = true;
    }

    // Build device backup path
    std::shared_ptr<mega::MegaNode> rootNode (MegaSyncApp->getRootNode());
    mUi->leBackupTo->setText(QString::fromUtf8(rootNode->getName())
                             + backupsDirPath + QLatin1Char('/')
                             + mDeviceName);
    refreshNextButtonState();
}

void BackupsWizard::onSetMyBackupsDirRequestStatus(int errorCode, QString errorMsg)
{
    bool nameCollision (false);

    if (errorCode == mega::MegaError::API_EEXIST)
    {
        // If dir already exists, rompt for new name
        mHaveBackupsDir = false;
        nameCollision = true;
        mBackupsDirName = remoteFolderExistsDialog(mBackupsDirName,
                                                   RenameTargetFolderDialog::TYPE_MYBACKUPS);
    }
    else if (errorCode != mega::MegaError::API_OK)
    {
        displayError(tr("Creating or setting folder \"%1\" as backups root failed.\nReason: %2")
                     .arg(mBackupsDirName, errorMsg));
    }

    setupMyBackupsDir(nameCollision);
}

void BackupsWizard::onSetDeviceNameRequestStatus(int errorCode, QString errorMsg)
{
    if (errorCode != mega::MegaError::API_OK)
    {
        displayError(tr("Setting \"%1\" as device name failed.\nReason: %2")
                     .arg(mDeviceName, errorMsg));
        mCurrentStep = Steps::STEP_2;
    }
    else
    {
        mCurrentStep = Steps::SETUP_BACKUPS;
    }
    emit nextStep();
}

void BackupsWizard::onSyncAddRequestStatus(int errorCode, QString errorMsg)
{
    if (mCurrentSyncIdx < mUi->lvFoldersS2->count())
    {
        auto itemL1 (mStep1FoldersModel->item(mCurrentSyncIdx));
        auto itemL2 (mUi->lvFoldersS2->item(mCurrentSyncIdx));

        // Update tooltip and icon according to result
        if (errorCode == mega::MegaError::API_OK)
        {
            QIcon folderIcon (QIcon(QLatin1String("://images/folder_icon.png")));
            QString tooltipMsg (itemL2->data(Qt::UserRole).toString());
            itemL2->setData(Qt::DecorationRole, folderIcon);
            itemL2->setData(Qt::ToolTipRole, tooltipMsg);
            itemL1->setData(folderIcon, Qt::DecorationRole);
            itemL1->setData(tooltipMsg, Qt::ToolTipRole);
        }
        else
        {
            mError = true;
            QString name (itemL1->data(Qt::DisplayRole).toString());
            QIcon   warnIcon (QIcon(QLatin1String("://images/mimes/folder with warning/folder with warning.png")));
            QString tooltipMsg (itemL2->data(Qt::UserRole).toString()
                                + QLatin1String("\nError: ") + errorMsg);
            itemL2->setData(Qt::DecorationRole, warnIcon);
            itemL2->setData(Qt::ToolTipRole, tooltipMsg);
            itemL1->setData(warnIcon, Qt::DecorationRole);
            itemL1->setData(tooltipMsg, Qt::ToolTipRole);

            displayError(errorMsg);
        }
        itemL1->setData(Qt::Unchecked, Qt::CheckStateRole);

        // Process next item
        mCurrentSyncIdx++;
        setupBackups();
    }
}

void BackupsWizard::onSuccessDialogAccepted()
{
// FIXME: Revert to live url when feature is merged
//  QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#fm/backups")));
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("https://13755-backup-center.developers.mega.co.nz/fm/backups")));

    emit nextStep();
}
