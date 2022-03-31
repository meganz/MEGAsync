#include "BackupsWizard.h"
#include "ui_BackupsWizard.h"
#include "ui_BackupSetupSuccessDialog.h"
#include "MegaApplication.h"
#include "megaapi.h"
#include "QMegaMessageBox.h"
#include "EventHelper.h"

#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <QTimer>

constexpr int HEIGHT_ROW_STEP_1 (40);
constexpr int MARGIN_LV_STEP_1 (50);
constexpr int MAX_ROWS_STEP_1 (3);
constexpr int HEIGHT_MAX_STEP_1 (413);
constexpr int HEIGHT_MIN_STEP_1 (HEIGHT_MAX_STEP_1 - HEIGHT_ROW_STEP_1 * MAX_ROWS_STEP_1);
constexpr int HEIGHT_ROW_STEP_2 (32);
constexpr int MARGIN_LV_STEP_2 (36);
constexpr int MAX_ROWS_STEP_2 (5);
constexpr int HEIGHT_MIN_STEP_2 (260);
constexpr int HEIGHT_MAX_STEP_2 (HEIGHT_MIN_STEP_2 + HEIGHT_ROW_STEP_2 * MAX_ROWS_STEP_2 + 47); //+47 is the height of the wBackupTo widget in step 2.
constexpr QSize FINAL_STEP_MIN_SIZE (QSize(520, 215));
constexpr QSize SIZE_STEP_1 (QSize(600, 370));
constexpr int SHOW_MORE_VISIBILITY (2);





BackupsWizard::BackupsWizard(QWidget* parent) :
    QDialog(parent),
    mUi (new Ui::BackupsWizard),
    mSyncController(),
    mCreateBackupsDir (false),
    mDeviceDirHandle (mega::INVALID_HANDLE),
    mBackupsDirName (),
    mHaveBackupsDir (false),
    mError (false),
    mUserCancelled (false),
    mFoldersModel (new QStandardItemModel(this)),
    mFoldersProxyModel (new ProxyModel(this))
{
    setWindowFlags((windowFlags() | Qt::WindowCloseButtonHint) & ~Qt::WindowContextHelpButtonHint);
    mUi->setupUi(this);

    mHighDpiResize.init(this);

    connect(&mSyncController, &SyncController::deviceName,
            this, &BackupsWizard::onDeviceNameSet);

    connect(&mSyncController, &SyncController::myBackupsHandle,
            this, &BackupsWizard::onBackupsDirSet);

    // React on item check/uncheck
    connect(mFoldersModel, &QStandardItemModel::itemChanged,
            this, &BackupsWizard::onListItemChanged);

    connect(&mSyncController, &SyncController::setMyBackupsStatus,
            this, &BackupsWizard::onSetMyBackupsDirRequestStatus);

    connect(&mSyncController, &SyncController::syncAddStatus,
            this, &BackupsWizard::onSyncAddRequestStatus);

    mFoldersProxyModel->setSourceModel(mFoldersModel);
    mUi->lvFoldersStep1->setModel(mFoldersProxyModel);
    mUi->lvFoldersStep2->setModel(mFoldersProxyModel);

    // Go to Step 1
    setupStep1();

    mLoadingWindow  = new QWidget(this);
    mLoadingWindow->hide();

    //Setting up the spining window.
    mLoadingWindow->setAutoFillBackground(true);
    QColor color(Qt::white);
    color.setAlpha(180);
    QPalette bgPalette = mLoadingWindow->palette();
    bgPalette.setColor(QPalette::Window, color);
    mLoadingWindow->setPalette(bgPalette);
    QHBoxLayout* ly = new QHBoxLayout(mLoadingWindow);
    ly->setAlignment(Qt::AlignCenter);
    QLabel* lbl = new QLabel(mLoadingWindow);
    lbl->setAlignment(Qt::AlignCenter);
    QMovie* mv = new QMovie(QString::fromUtf8(":/animations/loading.gif"), QByteArray(), this);
    mv->setScaledSize(QSize(525, 350));
    mv->start();
    lbl->setMovie(mv);
    ly->addWidget(lbl);
    mLoadingWindow->setLayout(ly);
}

BackupsWizard::~BackupsWizard()
{
    delete mUi;
}

void BackupsWizard::showLess()
{
    QFontMetrics fm (mUi->tTextEdit->font());
    QMargins margins = mUi->tTextEdit->contentsMargins ();
    int nHeight = (fm.lineSpacing () * SHOW_MORE_VISIBILITY) +
        (mUi->tTextEdit->frameWidth () * 2) +
        margins.top () + margins.bottom ();
    mUi->tTextEdit->setMinimumHeight(nHeight);
    mUi->tTextEdit->setMaximumHeight(nHeight);
    mUi->bShowMore->setText(tr("Show more…"));
    EventManager::addEvent(mUi->tTextEdit->viewport(), QEvent::Wheel, EventHelper::BLOCK);
    mUi->tTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mUi->line1->hide();
    mUi->line2->hide();
    setMinimumHeight(FINAL_STEP_MIN_SIZE.height());
    resize(width(), FINAL_STEP_MIN_SIZE.height());

}

void BackupsWizard::showMore()
{
    mUi->line1->show();
    mUi->line2->show();
    mUi->bShowMore->setText(tr("Collapse"));
    mUi->tTextEdit->setMaximumHeight(QWIDGETSIZE_MAX);
    EventManager::addEvent(mUi->tTextEdit->viewport(), QEvent::Wheel, EventHelper::ACCEPT);
    mUi->tTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QFontMetrics fm (mUi->tTextEdit->font());
    QMargins margins = mUi->tTextEdit->contentsMargins ();
    int nHeight = fm.lineSpacing () * ((mErrList.size() > 5? 6 : mErrList.size()) -1) +
        (mUi->tTextEdit->frameWidth ()) * 2 +
        margins.top () + margins.bottom ();

    setMinimumHeight(FINAL_STEP_MIN_SIZE.height() + nHeight);
    resize(width(), FINAL_STEP_MIN_SIZE.height() + nHeight);
}

void BackupsWizard::refreshNextButtonState()
{
    bool enable (false);
    //step1
    if(mUi->sSteps->currentWidget() == mUi->pStep1)
        enable = (isSomethingChecked() && !mUi->lDeviceNameStep1->text().isEmpty());
    else //step2
        enable = mHaveBackupsDir;

    mUi->bNext->setEnabled(enable);
}

void BackupsWizard::setupStep1()
{
    mBackupsStatus.clear();
    mFoldersProxyModel->showOnlyChecked(false);

    qDebug("Backups Wizard: step 1 Init");
    refreshNextButtonState();
    mUi->sSteps->setCurrentWidget(mUi->pStep1);
    mUi->sButtons->setCurrentWidget(mUi->pStepButtons);
    mUi->bCancel->setEnabled(true);
    mUi->bNext->setText(tr("Next"));
    mUi->bBack->hide();
    int dialogHeight (HEIGHT_MAX_STEP_1);

    // Get device name
    mSyncController.getDeviceName();

    if (SyncModel::instance()->isRemoteRootSynced())
    {
        mUi->sMoreFolders->setCurrentWidget(mUi->pAllFoldersSynced);
        mUi->sFolders->setCurrentWidget(mUi->pNoFolders);
        // Remove the unused widget to avoid geometry interference
        mUi->sFolders->removeWidget(mUi->pFolders);
    }
    else
    {
        mUi->sMoreFolders->setCurrentWidget(mUi->pMoreFolders);
        mUi->sFolders->setCurrentWidget(mUi->pFolders);

        // Check if we need to refresh the lists
        if (mFoldersModel->rowCount() == 0)
        {
            QIcon folderIcon (QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")));

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

                    mFoldersModel->appendRow(item);
                }
            }
        }

        int listHeight = (std::max(HEIGHT_ROW_STEP_1,
                                 std::min(HEIGHT_ROW_STEP_1 * MAX_ROWS_STEP_1,
                                          mFoldersModel->rowCount() * HEIGHT_ROW_STEP_1))) + MARGIN_LV_STEP_1;

        dialogHeight = std::min(HEIGHT_MAX_STEP_1, HEIGHT_MIN_STEP_1 + listHeight);
        mUi->fFoldersStep1->setMinimumHeight(listHeight);
        setMinimumHeight(dialogHeight);

        resize(SIZE_STEP_1.width(), dialogHeight);
    }
    qDebug("Backups Wizard: step 1");
}

void BackupsWizard::setupStep2()
{
    mFoldersProxyModel->showOnlyChecked(true);
    qDebug("Backups Wizard: step 2 Init");
    refreshNextButtonState();
    mUi->sSteps->setCurrentWidget(mUi->pStep2);
    mUi->bNext->setText(tr("Setup"));
    mUi->bBack->show();

    // Request MyBackups root folder
    mHaveBackupsDir = false;
    mSyncController.getMyBackupsHandle();

    // Get number of items
    int nbSelectedFolders = mFoldersProxyModel->rowCount();

    // Set folders number
    if (nbSelectedFolders == 1)
    {
        mUi->lFoldersNumber->setText(tr("1 folder"));
    }
    else
    {
        mUi->lFoldersNumber->setText(tr("%1 folders").arg(nbSelectedFolders));
    }

    int listHeight = (std::max(HEIGHT_ROW_STEP_2,
                             std::min(HEIGHT_ROW_STEP_2 * MAX_ROWS_STEP_2,
                                      nbSelectedFolders * HEIGHT_ROW_STEP_2))) + MARGIN_LV_STEP_2;
    int dialogHeight (std::min(HEIGHT_MAX_STEP_2,
                               HEIGHT_MIN_STEP_2 + listHeight));
    mUi->fFoldersStep2->setMinimumHeight(listHeight);
    setMinimumHeight(dialogHeight);
    resize(width(), dialogHeight);

    qDebug("Backups Wizard: step 2");
}

void BackupsWizard::setupError()
{

    mUi->bShowMore->setVisible(mErrList.size() > SHOW_MORE_VISIBILITY);

    bool show_singular = mErrList.size() < 2;

    mUi->lErrorTextSingular->setVisible(show_singular);
    mUi->lErrorTitleSingular->setVisible(show_singular);

    mUi->lErrorTextPlural->setVisible(!show_singular);
    mUi->lErrorTitlePlural->setVisible(!show_singular);

    QTextDocument* doc = mUi->tTextEdit->document();
    doc->setDocumentMargin(0);
    QString txt = mErrList.join(QString::fromUtf8("\n"));
    mUi->tTextEdit->setText(txt);

    mUi->tTextEdit->viewport()->setCursor(Qt::ArrowCursor);
    mUi->tTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    mUi->line1->hide();
    mUi->line2->hide();
    mUi->bTryAgain->setFocus();
    mUi->sSteps->setCurrentWidget(mUi->pError);

    mUi->sButtons->setCurrentWidget(mUi->pErrorButtons);
    showLess();
}

void BackupsWizard::setupFinalize()
{
    qDebug("Backups Wizard: finalize");
    refreshNextButtonState();
    mUi->bCancel->setEnabled(false);
    mUi->bBack->hide();

    mError = false;

    nextStep(SETUP_MYBACKUPS_DIR);
}

void BackupsWizard::setupMyBackupsDir(bool nameCollision)
{
    qDebug("Backups Wizard: setup MyBackups");
    // If the user cancels, exit wizard
    if (mBackupsDirName.isEmpty())
    {
        // If the user cancels, exit wizard (this condition can be met only when executing this
        // code for the second time, as mBackupsDirName is initialized to a non-empty value.)
        mUserCancelled = true;
        nextStep(EXIT);
    }
    else
    {
        // FIXME: Do we still need to check for this with the INBOX switch?
        // Create MyBackups folder if necessary
        if (mCreateBackupsDir || nameCollision)
        {
            mSyncController.setMyBackupsDirName(mBackupsDirName);
        }
        else
        {
            // If not, proceed to setting-up backups
            nextStep(SETUP_BACKUPS);
        }
    }
}

void BackupsWizard::setupBackups()
{
    for(int i=0; i < mFoldersProxyModel->rowCount(); ++i)
    {
        BackupInfo backupInfo;
        backupInfo.folderName = mFoldersProxyModel->index(i, 0).data(Qt::DisplayRole).toString();
        backupInfo.status = QUEUED;
        mBackupsStatus.insert(mFoldersProxyModel->index(i, 0).data(Qt::UserRole).toString(), backupInfo);
    }
    processNextBackupSetup();
}

// Indicates if something is checked
bool BackupsWizard::isSomethingChecked()
{
    for (int i = 0; i < mFoldersModel->rowCount(); ++i)
    {
        if(mFoldersModel->data(mFoldersModel->index(i, 0), Qt::CheckStateRole) == Qt::Checked)
            return true;
    }
    return false;
}

void BackupsWizard::processNextBackupSetup()
{
    mLoadingWindow->resize(this->size());
    mLoadingWindow->show();
    for(auto it = mBackupsStatus.begin(); it != mBackupsStatus.end(); ++it)
    {
        if(it.value().status == QUEUED)
        {
            // Create backup
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                               QString::fromUtf8("Backups Wizard: setup backup \"%1\" to \"%2\"")
                               .arg(it.value().folderName,
                                    mBackupsDirName +  QLatin1Char('/')
                                    + mUi->lDeviceNameStep1->text() + QLatin1Char('/')
                                    + it.key()).toUtf8().constData());

            mSyncController.addSync(it.key(), mega::INVALID_HANDLE,
                                               it.value().folderName, mega::MegaSync::TYPE_BACKUP);
            return;
        }
    }
}

// Checks if the name is already used (remote)
// <displayName> is set to "" if the user cancels
void BackupsWizard::promptAndEnsureUniqueRemoteName(QString& displayName)
{    
    auto api (MegaSyncApp->getMegaApi());
    std::unique_ptr<mega::MegaNode> deviceDirNode (nullptr);

    if (mDeviceDirHandle != mega::INVALID_HANDLE)
    {
        deviceDirNode.reset(api->getNodeByHandle(mDeviceDirHandle));
    }

    std::shared_ptr<mega::MegaNode> dirNode (nullptr);
    // Check for name collision
    if (deviceDirNode)
    {
        dirNode.reset(api->getChildNode(deviceDirNode.get(),
                                        displayName.toUtf8().constData()));
        if (dirNode)
        {
            displayName = remoteFolderExistsDialog(displayName);
        }
    }
}

// Checks if a path belongs is in an existing sync or backup tree; and if the selected
// folder has a sync or backup in its tree.
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
        localFolders += SyncModel::instance()->getLocalFolders(type);
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
        int nbBackups (mFoldersModel->rowCount());
        int row (0);

        while (message.isEmpty() && row < nbBackups)
        {
            QString c (QDir::cleanPath(mFoldersModel->item(row)->data(Qt::UserRole).toString()));
            bool sameSize (cleanInputPath.size() == c.size());

            // Do not consider unchecked items
            if (mFoldersModel->item(row)->checkState() == Qt::Checked)
            {
                // Handle same path another way later: by selecting the row in the view.
                if (cleanInputPath.startsWith(c) && !sameSize
                        && (c.size() == cleanInputPath.size()
                            || cleanInputPath[c.size()] == QDir::separator()))
                {
                    message = tr("The selected local folder is already backed up");
                }
                else if (lf != localFolders.cend() && c.startsWith(cleanInputPath) && !sameSize
                         && (c[cleanInputPath.size()] == QDir::separator()
                             || cleanInputPath == QDir(*lf).rootPath()))
                {
                    message = tr("A backed up folder cannot be inside a backup folder");
                }
            }
            row++;
        }
    }

    if (displayWarning && !message.isEmpty())
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), message, QMessageBox::Ok);
    }

    return (!message.isEmpty());
}

// Returns new name if new name set; empty string if backup canceled.
QString BackupsWizard::remoteFolderExistsDialog(const QString& backupName)
{
    QString newName(QLatin1String(""));
    RenameTargetFolderDialog renameDialog (backupName);

    if (renameDialog.exec() == QDialog::Accepted)
    {
        newName = renameDialog.getNewFolderName();
    }
    return newName;
}

// State machine orchestrator
void BackupsWizard::nextStep(const Steps &step)
{
    qDebug("Backups Wizard: next step");
    refreshNextButtonState();
    mUi->fFoldersStep1->setMinimumHeight(0);
    mUi->fFoldersStep2->setMinimumHeight(0);

    switch (step)
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
        case Steps::SETUP_MYBACKUPS_DIR:
        {
            setupMyBackupsDir();
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
            qDebug("Backups Wizard: exit");
            mUserCancelled ? reject() : accept();
            break;
        }
        case Steps::ERROR_FOUND:
        {
            setupError();
            break;
        }
        default:
            break;
    }
    refreshNextButtonState();
}

void BackupsWizard::on_bNext_clicked()
{
    qDebug("Backups Wizard: next clicked");
    if(mUi->sSteps->currentWidget() == mUi->pStep1)
        nextStep(STEP_2_INIT);
    else
        nextStep(FINALIZE);
}

void BackupsWizard::on_bCancel_clicked()
{
    int userWantsToCancel (QMessageBox::Yes);

    // If the user has made any modification, warn them before exiting.
    if (isSomethingChecked())
    {
        QString title (tr("Warning"));
        QString content (tr("Are you sure you want to cancel? All changes will be lost."));
        userWantsToCancel = QMessageBox::warning(this, title, content,
                                                 QMessageBox::Yes,
                                                 QMessageBox::No);
    }

    if (userWantsToCancel == QMessageBox::Yes)
    {
        qDebug("Backups Wizard: user cancel");
        mUserCancelled = true;
        nextStep(EXIT);
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

        // Check for path and name collision.
        int nbBackups (mFoldersModel->rowCount());
        int row (0);

        while (existingBackup == nullptr && row < nbBackups)
        {
            auto currItem = mFoldersModel->itemData(mFoldersModel->index(row, 0));
            QString name (currItem[Qt::DisplayRole].toString());
            if (path == currItem[Qt::UserRole].toString())
            {
                // If folder is already backed up, set pointer and take existing name
                existingBackup = mFoldersModel->item(row);
                displayName = name;
            }
            row++;
        }

        // Add backup
        QStandardItem* item (nullptr);
        if (existingBackup != nullptr)
        {
            item = existingBackup;
        }
        else
        {
            QIcon icon (QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")));
            item = new QStandardItem(displayName);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setData(path, Qt::ToolTipRole);
            item->setData(path, Qt::UserRole);
            item->setData(icon, Qt::DecorationRole);
            mFoldersModel->insertRow(0, item);
        }
        item->setData(Qt::Checked, Qt::CheckStateRole);

        // Jump to item in list
        auto idx = mFoldersModel->indexFromItem(item);
        mUi->lvFoldersStep1->scrollTo(idx,QAbstractItemView::PositionAtCenter);

        refreshNextButtonState();
        qDebug() << QString::fromUtf8("Backups Wizard: add folder \"%1\"").arg(path);
    }
}

void BackupsWizard::on_bBack_clicked()
{    
    qDebug("Backups Wizard: back");
    // The "Back" button only appears at STEP_2. Go back to STEP_1_INIT.
    nextStep(STEP_1_INIT);
}

void BackupsWizard::on_bViewInBackupCentre_clicked()
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Backups Wizard: show Backup Center");
// FIXME: Revert to live url when feature is merged
//  QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#fm/backups")));
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("https://13755-backup-center.developers.mega.co.nz/dont-deploy/sandbox3.html?apipath=prod&jj=2")));
    nextStep(EXIT);
}

void BackupsWizard::on_bDismiss_clicked()
{
    nextStep(EXIT);
}

void BackupsWizard::on_bTryAgain_clicked()
{
    nextStep(STEP_1_INIT);
}

void BackupsWizard::on_bCancelErr_clicked()
{
    nextStep(EXIT);
}

void BackupsWizard::on_bShowMore_clicked()
{
    if(mUi->tTextEdit->maximumHeight() == QWIDGETSIZE_MAX)
    {
        showLess();
    }
    else
    {
        showMore();
    }
    qDebug() << mUi->tTextEdit->minimumHeight() << mUi->tTextEdit->sizeHint();

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

void BackupsWizard::setupComplete()
{
    if (!mError)
    {
        qDebug("Backups Wizard: setup completed successfully");
        // We are now done, exit
        // No error: show success message!
        bool show_singular = mBackupsStatus.size() < 2;

        mUi->lSuccessTextSingular->setVisible(show_singular);
        mUi->lSuccessTextPlural->setVisible(!show_singular);

        mUi->sSteps->setCurrentWidget(mUi->pSuccessfull);

        mUi->sButtons->setCurrentWidget(mUi->pSuccessButtons);
        mUi->bViewInBackupCentre->setFocus();
        setMinimumHeight(FINAL_STEP_MIN_SIZE.height());
        setMinimumSize(FINAL_STEP_MIN_SIZE);
        resize(FINAL_STEP_MIN_SIZE);
    }
    else
    {
        qDebug("Backups Wizard: setup completed with error, go back to step 1");
        // Error: go back to Step 1 :(
        nextStep(STEP_1_INIT);
    }
}

void BackupsWizard::onDeviceNameSet(QString deviceName)
{
    mUi->lDeviceNameStep1->setText(deviceName);
    mUi->lDeviceNameStep2->setText(deviceName);
    refreshNextButtonState();
}

void BackupsWizard::onBackupsDirSet(mega::MegaHandle backupsDirHandle)
{
    mHaveBackupsDir = true;
    QString backupsDirPath;

    if (backupsDirHandle != mega::INVALID_HANDLE)
    {
        // We still have to check if the folder exists... try to get its path
        auto api (MegaSyncApp->getMegaApi());
        std::unique_ptr<mega::MegaNode> backupsDirNode (api->getNodeByHandle(backupsDirHandle));
        if (backupsDirNode)
        {
            backupsDirPath = QString::fromUtf8(api->getNodePath(backupsDirNode.get()));

            if (!backupsDirPath.isEmpty())
            {
                // If the folder exists, take note
                mBackupsDirName = QDir(backupsDirPath).dirName();
                mCreateBackupsDir = false;
            }

            // FIXME: Do we still need to check for this with the INBOX switch?
            // Check that the folder has not been sent to rubbish
            if (api->isInRubbish(backupsDirNode.get()))
            {
                qDebug("Backups Wizard: MyBackups dir in rubbish bin");
                QString message (tr("The folder to store backups has been moved to the Rubbish bin. "
                                    "Do you want to create a new one?"));
                // Show dialog asking for user action: cancel, or create new?
                auto choice (QMegaMessageBox::critical(nullptr, tr("Error"), message,
                                          QMessageBox::Yes | QMessageBox::No));
                if (choice == QMessageBox::Yes)
                {
                    // Reset path to force creation later on
                    backupsDirPath.clear();
                }
                else
                {
                    // User cancelled. Exit wizard
                    mHaveBackupsDir = false;
                    mCreateBackupsDir = false;
                    mUserCancelled = true;
                    nextStep(EXIT);
                }
            }
        }
    }

    // If backupsDirPath is empty, the remote folder does not exist and we have to create it.
    if (backupsDirPath.isEmpty())
    {
        // If remote dir does not exist, use default name and program its creation
        mBackupsDirName = tr("My Backups");
        backupsDirPath = QLatin1Char('/') + mBackupsDirName;
        mCreateBackupsDir = true;
        qDebug() << QString::fromLatin1("Backups Wizard: MyBackups dir: \"%1\"").arg(backupsDirPath);
    }

    // Build device backup path
    std::shared_ptr<mega::MegaNode> rootNode (MegaSyncApp->getRootNode());
    mUi->leBackupTo->setText(QString::fromUtf8(rootNode->getName())
                             + backupsDirPath + QLatin1Char('/')
                             + mUi->lDeviceNameStep1->text());
    refreshNextButtonState();
}

void BackupsWizard::onSetMyBackupsDirRequestStatus(int errorCode, const QString& errorMsg)
{
    bool nameCollision (false);

    // FIXME: Do we still need to check for this with the INBOX switch?
    if (errorCode == mega::MegaError::API_EEXIST)
    {
        // If dir already exists, prompt for new name
        mHaveBackupsDir = false;
        nameCollision = true;
        mBackupsDirName = remoteFolderExistsDialog(mBackupsDirName);
    }
    else if (errorCode != mega::MegaError::API_OK)
    {
        QMegaMessageBox::critical(nullptr, tr("Error"),
                                  tr("Creating or setting folder \"%1\" as backups root failed.\nReason: %2")
                                  .arg(mBackupsDirName, errorMsg));
    }

    setupMyBackupsDir(nameCollision);
}

void BackupsWizard::onSyncAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name)
{

    // Update tooltip and icon according to result
    if (errorCode == mega::MegaError::API_OK)
    {
        BackupInfo info = mBackupsStatus.value(name);
        info.status = OK;
        mBackupsStatus.insert(name, info);
    }
    else
    {
        mError = true;
        BackupInfo info = mBackupsStatus.value(name);
        info.status = ERR;
        mBackupsStatus.insert(name, info);
    }


    //Check if the process is completed or there are still pending requests
    bool finished(true);
    bool errorsExists(false);
    for(auto it = mBackupsStatus.begin(); it != mBackupsStatus.end(); ++it)
    {
        if(it.value().status == QUEUED)
        {
          finished = false;
          break;
        }
        else if(it.value().status == ERR)
        {
          errorsExists = true;
        }

        if(!finished && errorsExists)
            break;
    }

    if(finished)
    {
        if(!errorsExists)
        {
            nextStep(DONE);
        }
        else
        {
            mErrList.clear();
            for(auto it = mBackupsStatus.begin(); it != mBackupsStatus.end(); ++it)
            {
                QModelIndex index = mFoldersProxyModel->getIndexByPath(it.key());
                QStandardItem* item = mFoldersModel->itemFromIndex(index);
                if(!item)
                {
                    return;
                }
                if(it.value().status == ERR)
                {
                    mErrList.append(it.value().folderName);
                    QIcon   warnIcon (QIcon(QLatin1String("://images/icons/folder/folder-mono-with-warning_24.png")));
                    QString tooltipMsg (item->data(Qt::UserRole).toString()
                                        + QLatin1String("\nError: ") + errorMsg);
                    item->setData(warnIcon, Qt::DecorationRole);
                    item->setData(tooltipMsg, Qt::ToolTipRole);
                    item->setData(Qt::Unchecked, Qt::CheckStateRole);
                }
                else if(it.value().status == OK)
                {
                    QIcon folderIcon (QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")));
                    QString tooltipMsg (item->data(Qt::UserRole).toString());
                    item->setData(folderIcon, Qt::DecorationRole);
                    item->setData(tooltipMsg, Qt::ToolTipRole);
                    mFoldersModel->removeRow(index.row());
                }
            }
            nextStep(ERROR_FOUND);
        }


        mLoadingWindow->hide();
    }
    else
    {
        processNextBackupSetup();
    }
}

ProxyModel::ProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    mShowOnlyChecked(false)
{

}

void ProxyModel::showOnlyChecked(bool val)
{
    if(val != mShowOnlyChecked)
    {
        mShowOnlyChecked = val;
        invalidateFilter();
    }
}

QModelIndex ProxyModel::getIndexByPath(const QString& path)
{
    for(int i = 0; i < sourceModel()->rowCount(); ++i)
    {
        QModelIndex index = sourceModel()->index(i, 0);
        if(index.data(Qt::UserRole).toString() == path)
        {
            return index;
        }
    }
    return QModelIndex();
}

bool ProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    if(!mShowOnlyChecked)
    {
        return true;
    }
    QModelIndex idx = sourceModel()->index(source_row, 0);
    return idx.data(Qt::CheckStateRole).toBool();
}

QVariant ProxyModel::data(const QModelIndex &index, int role) const
{
    if(mShowOnlyChecked && role == Qt::CheckStateRole)
    {
        return QVariant();
    }
    return QSortFilterProxyModel::data(index, role);
}

Qt::ItemFlags ProxyModel::flags(const QModelIndex &index) const
{
    return QSortFilterProxyModel::flags(index)  & ~Qt::ItemIsSelectable;
}

