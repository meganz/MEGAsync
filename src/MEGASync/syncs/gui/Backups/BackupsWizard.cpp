#include "BackupsWizard.h"
#include "ui_BackupsWizard.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "QMegaMessageBox.h"
#include "EventHelper.h"
#include "TextDecorator.h"
#include "UserAttributesRequests/DeviceName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "syncs/gui/Backups/BackupNameConflictDialog.h"
#include "syncs/gui/SyncTooltipCreator.h"
#include "Platform.h"
#include <DialogOpener.h>

#include "megaapi.h"

#include <QtConcurrent/QtConcurrent>

// Consts used to implement the resizing of the window
// in function of the number of lines in the tables
const int HEIGHT_ROW_STEP_1 (40);
const int MAX_ROWS_STEP_1 (3);
const int HEIGHT_MAX_STEP_1 (413);
const int HEIGHT_ROW_STEP_2 (32);
const int MAX_ROWS_STEP_2 (5);
const int HEIGHT_MAX_STEP_2 (455);
const QSize FINAL_STEP_MIN_SIZE (QSize(520, 230));
const int SHOW_MORE_VISIBILITY (2);
const int SHOW_MORE_MAX_ELEM (5);

const int ICON_POSITION_STEP_1 (50);
const int ICON_POSITION_STEP_2 (3);

const int TEXT_MARGIN (12);

const char* BackupsWizard::EMPTY_PROPERTY = "EMPTY";

// BackupsWizard class ------------------------------------------------------

BackupsWizard::BackupsWizard(QWidget* parent) :
    QDialog(parent),
    mUi (new Ui::BackupsWizard),
    mDeviceNameRequest (UserAttributes::DeviceName::requestDeviceName()),
    mMyBackupsHandleRequest (UserAttributes::MyBackupsHandle::requestMyBackupsHandle()),
    mSyncController(),
    mError (false),
    mUserCancelled (false),
    mFoldersModel (new QStandardItemModel(this)),
    mFoldersProxyModel (new ProxyModel(this)),
    mCurrentStep (STEP_1)
{
    // Setup UI
    setWindowFlags((windowFlags() | Qt::WindowCloseButtonHint));

#ifdef _WIN32
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
#endif
    mUi->setupUi(this);
    setupHeaders();
    setupLoadingWindow();

    // Connect signals
    connect(mDeviceNameRequest.get(), &UserAttributes::DeviceName::attributeReady,
            this, &BackupsWizard::onDeviceNameSet);
    connect(mMyBackupsHandleRequest.get(), &UserAttributes::MyBackupsHandle::attributeReady,
            this, &BackupsWizard::onMyBackupsFolderHandleSet);
    connect(&mSyncController, &SyncController::syncAddStatus,
            this, &BackupsWizard::onSyncAddRequestStatus);
    connect(mFoldersModel, &QStandardItemModel::itemChanged,
            this, &BackupsWizard::onItemChanged);

    // Setup backups lists
    setupLists();

    // Go to Step 1
    nextStep(STEP_1);
}

BackupsWizard::~BackupsWizard()
{
    delete mUi;
}

void BackupsWizard::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        //SETUP_BACKUPS only shows a GIF
        if(mCurrentStep != Step::SETUP_BACKUPS)
        {
            nextStep(mCurrentStep);
        }
        setupHeaders();
    }
    QDialog::changeEvent(event);
}

// State machine orchestrator
void BackupsWizard::nextStep(const Step &step)
{
    mCurrentStep = step;

    switch (step)
    {
        case Step::STEP_1:
        {
            setupStep1();
            break;
        }
        case Step::STEP_2:
        {
            setupStep2();
            break;
        }
        case Step::HANDLE_NAME_CONFLICTS:
        {
            handleNameConflicts();
            break;
        }
        case Step::FINALIZE:
        {
            setupFinalize();
            break;
        }
        case Step::SETUP_BACKUPS:
        {
            setupBackups();
            break;
        }
        case Step::DONE:
        {
            setupComplete();
            break;
        }
        case Step::EXIT:
        {
            mUserCancelled ? reject() : accept();
            break;
        }
        case Step::ERROR_FOUND:
        {
            setupError();
            break;
        }
        default:
            break;
    }
}

void BackupsWizard::setupStep1()
{
    mBackupsStatus.clear();
    mFoldersProxyModel->showOnlyChecked(false);

    onItemChanged();
    mUi->lAllFoldersSynced->hide();
    setCurrentWidgetsSteps(mUi->pStep1);
    mUi->sButtons->setCurrentWidget(mUi->pStepButtons);

#ifdef _WIN32
    QFontMetrics fm(mUi->lSubtitleStep1->font());
    QRect boundingRect = fm.boundingRect(QRect(0,0, mUi->fFoldersStep1->width()
                                               - mUi->lSubtitleStep1->contentsMargins().left()
                                               - mUi->lSubtitleStep1->contentsMargins().right()
                                               ,0), Qt::TextWordWrap, mUi->lSubtitleStep1->toPlainText());
    int bottomMargin = mUi->lSubtitleStep1->contentsMargins().bottom();
    mUi->lSubtitleStep1->setFixedHeight(boundingRect.height() + bottomMargin + mUi->lSubtitleStep1->document()->documentMargin());
    mUi->lSubtitleStep1->viewport()->setCursor(Qt::ArrowCursor);

#endif

    mUi->bCancel->setEnabled(true);
    mUi->bNext->setText(tr("Next"));
    mUi->bBack->hide();

    // Get device name
    onDeviceNameSet(mDeviceNameRequest->getDeviceName());

    // Check if we need to refresh the lists
    if (mFoldersModel->rowCount() == 0)
    {
        // Populate with standard locations

        QList<QStandardPaths::StandardLocation> locations{
            QStandardPaths::DocumentsLocation,
                    QStandardPaths::MoviesLocation,
                    QStandardPaths::PicturesLocation,
                    QStandardPaths::MusicLocation,
                    QStandardPaths::DownloadLocation,
                    QStandardPaths::DesktopLocation};
        checkStandardLocations(locations);
    }
    else
    {
        mUi->bMoreFolders->setText(tr("More folders"));
        updateSize();
    }
}

void BackupsWizard::checkStandardLocations(QList<QStandardPaths::StandardLocation> locations)
{
    if(!locations.isEmpty() && mFoldersModel->rowCount() < MAX_ROWS_STEP_1)
    {
        QIcon folderIcon (QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")));

        auto location = locations.takeFirst();

        const auto standardPaths (QStandardPaths::standardLocations(location));
        QDir dir (QDir::cleanPath(standardPaths.first()));
        QString path (QDir::toNativeSeparators(dir.canonicalPath()));
        if (dir.exists() && dir != QDir::home())
        {
            auto isSyncable = [this, folderIcon, path, locations](bool canSync)
            {
                if(canSync)
                {
                    QStandardItem* item (new QStandardItem(SyncController::getSyncNameFromPath(path)));
                    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                    item->setData(SyncTooltipCreator::createForLocal(path), Qt::ToolTipRole);
                    item->setData(path, Qt::UserRole);
                    item->setData(folderIcon, Qt::DecorationRole);
                    item->setData(Qt::Unchecked, Qt::CheckStateRole);
                    mFoldersModel->appendRow(item);
                }

                checkStandardLocations(locations);
            };
            isFolderSyncable(path, isSyncable);
        }
        else
        {
            checkStandardLocations(locations);
        }
    }
    else
    {
        if(mFoldersModel->rowCount() == 0)
        {
            mUi->bMoreFolders->setText(tr("Add folders"));
            mUi->wDeviceNameStep1->setProperty(EMPTY_PROPERTY, true);
            mUi->wDeviceNameStep1->setStyleSheet(mUi->wDeviceNameStep1->styleSheet());
        }
        else
        {
            mUi->bMoreFolders->setText(tr("More folders"));
        }

        updateSize();
    }
}

void BackupsWizard::setupStep2()
{
    mFoldersProxyModel->showOnlyChecked(true);
    onItemChanged();
    setCurrentWidgetsSteps(mUi->pStep2);
    mUi->bNext->setText(tr("Setup"));
    mUi->bBack->show();

    // Set MyBackups root folder
    onMyBackupsFolderHandleSet(mMyBackupsHandleRequest->getMyBackupsHandle());

    // Get number of items
    int nbSelectedFolders = mFoldersProxyModel->rowCount();

    // Set folders number
    mUi->lFoldersNumber->setText(tr("%n folder", "", nbSelectedFolders));

    updateSize();
}

void BackupsWizard::handleNameConflicts()
{
    // Get candidates list
    QStringList candidatePaths;
    for (int i = 0; i < mFoldersProxyModel->rowCount(); ++i)
    {
        QString path (mFoldersProxyModel->index(i, 0).data(Qt::UserRole).toString());
        candidatePaths.append(path);
    }

    if(!BackupNameConflictDialog::backupNamesValid(candidatePaths))
    {
        BackupNameConflictDialog* conflictDialog = new BackupNameConflictDialog(candidatePaths, this);
        DialogOpener::showDialog<BackupNameConflictDialog>(conflictDialog, [this, conflictDialog](){
            if(conflictDialog->result() == QDialog::Accepted)
            {
                const auto changes (conflictDialog->getChanges());
                auto changeIt (changes.cbegin());
                while (changeIt != changes.cend())
                {
                    int nbBackups (mFoldersModel->rowCount());
                    int row (0);
                    bool found (false);
                    while (!found && row < nbBackups)
                    {
                        auto item (mFoldersModel->item(row));
                        if (item && item->data(Qt::UserRole).toString() == changeIt.key())
                        {
                            item->setData(changeIt.value(), Qt::DisplayRole);
                            found = true;
                        }
                        else
                        {
                            row++;
                        }
                    }
                    changeIt++;
                }
                nextStep(FINALIZE);
            }
            else
            {
                nextStep(STEP_2);
            }
        });
    }
    else
    {
        nextStep(FINALIZE);
    }
}

void BackupsWizard::setupFinalize()
{
    onItemChanged();
    mUi->bCancel->setEnabled(false);
    mUi->bBack->hide();

    mError = false;

    nextStep(SETUP_BACKUPS);
}

void BackupsWizard::setupBackups()
{
    for (int i = 0; i < mFoldersProxyModel->rowCount(); ++i)
    {
        QString path (mFoldersProxyModel->index(i, 0).data(Qt::UserRole).toString());
        if(!mBackupsStatus.contains(path))
        {
            QString syncName (mFoldersProxyModel->index(i, 0).data(Qt::DisplayRole).toString());
            mBackupsStatus.insert(path, {QUEUED, syncName});
        }
    }
    processNextBackupSetup();
}

void BackupsWizard::processNextBackupSetup()
{
    mLoadingWindow->resize(this->size());
    mLoadingWindow->show();
    for (auto it = mBackupsStatus.begin(); it != mBackupsStatus.end(); ++it)
    {
        if (it.value().status == QUEUED)
        {
            // Create backup
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO,
                               QString::fromUtf8("Backups Wizard: setup backup \"%1\" from \"%2\"")
                               .arg(it.value().syncName, it.key()).toUtf8().constData());

            mSyncController.addBackup(it.key(), it.value().syncName);
            return;
        }
    }
}

void BackupsWizard::setupComplete()
{
    if (!mError)
    {
        // We are now done, exit
        // No error: show success message!
        mUi->lTitle->setText(tr("Backup created","", mBackupsStatus.size()));
        mUi->lSuccessText->setText(tr("We're backing up your folder. The time this takes depends on the files in this folder.","", mBackupsStatus.size()));
        setCurrentWidgetsSteps(mUi->pSuccessfull);

        mUi->sButtons->setCurrentWidget(mUi->pSuccessButtons);
        mUi->bViewInBackupCentre->setFocus();
        setMinimumHeight(FINAL_STEP_MIN_SIZE.height());
        setMinimumSize(FINAL_STEP_MIN_SIZE);
        resize(FINAL_STEP_MIN_SIZE);

        MegaSyncApp->createAppMenus();
    }
    else
    {
        // Error: go back to Step 1 :(
        nextStep(STEP_1);
    }
}

void BackupsWizard::setupError()
{
    mUi->bShowMore->setVisible(mErrList.size() > SHOW_MORE_VISIBILITY);

    mUi->lErrorTitle->setText(tr("Problem backing up folder", "", mErrList.size()));
    mUi->lErrorText->setText(tr("This folder wasn't backed up. Try again.", "", mErrList.size()));

    QString txt = mErrList.join(QString::fromUtf8("\n"));
    mUi->tTextEdit->setText(txt);
    mUi->tTextEdit->document()->setDocumentMargin(0);
    mUi->tTextEdit->viewport()->setCursor(Qt::ArrowCursor);
    mUi->tTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);

    mUi->bTryAgain->setFocus();

    setCurrentWidgetsSteps(mUi->pError);
    mUi->sButtons->setCurrentWidget(mUi->pErrorButtons);

    showLess();
}

// Indicates if something is checked
bool BackupsWizard::atLeastOneFolderChecked() const
{
    for (int i = 0; i < mFoldersModel->rowCount(); ++i)
    {
        if (mFoldersModel->data(mFoldersModel->index(i, 0), Qt::CheckStateRole) == Qt::Checked)
            return true;
    }
    return false;
}

// Checks if a path is syncable.
// Path is considered to be canonical.
void BackupsWizard::isFolderSyncable(const QString& path, std::function<void(bool)> func, bool displayWarning, bool fromCheckAction)
{
    QString inputPath (QDir::toNativeSeparators(QDir(path).absolutePath()));

    // Check syncability
    QString message;
    auto syncability (SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message));

    // Check current list
    if (syncability != SyncController::CANT_SYNC)
    {
        // Check for path collision
        int nbBackups (mFoldersModel->rowCount());
        int row (0);

        while (syncability != SyncController::CANT_SYNC && row < nbBackups)
        {
            QString existingPath (mFoldersModel->item(row)->data(Qt::UserRole).toString());

            // Only take checked items into account
            if (mFoldersModel->item(row)->checkState() == Qt::Checked)
            {
                // Handle same path another way later: by selecting the row in the view.
                if (!fromCheckAction && inputPath == existingPath)
                {
                    message = tr("Folder is already selected. Select a different folder.");
                    syncability = SyncController::CANT_SYNC;
                }
                else if (inputPath.startsWith(existingPath)
                         && inputPath[existingPath.size()] == QDir::separator())
                {
                    message = SyncController::getErrStrCurrentBackupInsideExistingBackup();
                    syncability = SyncController::CANT_SYNC;
                }
                else if (existingPath.startsWith(inputPath)
                         && existingPath[inputPath.size()] == QDir::separator())
                {
                    message = SyncController::getErrStrCurrentBackupOverExistingBackup();
                    syncability = SyncController::CANT_SYNC;
                }
            }
            row++;
        }
    }

    if (displayWarning) // Do not display warning when silenced
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.text = message;

        if (syncability == SyncController::CANT_SYNC)
        {
            QMegaMessageBox::warning(msgInfo);
        }
        // Display warning on check action only (also called when creating)
        else if (syncability == SyncController::WARN_SYNC
                 && fromCheckAction)
        {
            msgInfo.text += QLatin1Char('/')
                    + tr("Do you want to continue?");
            msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
            msgInfo.defaultButton = QMessageBox::No;
            msgInfo.finishFunc = [this, func](QPointer<QMessageBox> msg)
            {
                func(msg->result() == QMessageBox::Yes);
            };
            QMegaMessageBox::warning(msgInfo);
            return;
        }
    }

    func(syncability != SyncController::CANT_SYNC);
}

void BackupsWizard::setupLists()
{
    mFoldersProxyModel->setSourceModel(mFoldersModel);
    mUi->lvFoldersStep1->setModel(mFoldersProxyModel);
    mUi->lvFoldersStep1->setItemDelegate(new WizardDelegate(this));
    mUi->lvFoldersStep2->setModel(mFoldersProxyModel);
    mUi->lvFoldersStep2->setItemDelegate(new WizardDelegate(this));
    mUi->lvFoldersStep1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mUi->lvFoldersStep2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void BackupsWizard::setupHeaders()
{
    Text::Bold boldText;
    Text::Decorator tc(&boldText);

    QString titleStep1 = tr("1. [B]Select[/B] folders to backup");
    tc.process(titleStep1);
    mUi->lTitleStep1->setText(titleStep1);

    QString titleStep2 = tr("2. [B]Confirm[/B] backup settings");
    tc.process(titleStep2);
    mUi->lTitleStep2->setText(titleStep2);
}

void BackupsWizard::setupLoadingWindow()
{
    //TODO: move this to separate UI to suit better with darkmode(future develop)
    mLoadingWindow  = new QWidget(this);
    mLoadingWindow->hide();
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

void BackupsWizard::showLess()
{
    mUi->line1->hide();
    mUi->line2->hide();
    mUi->bShowMore->setText(tr("Show more…"));

    EventManager::addEvent(mUi->tTextEdit->viewport(), QEvent::Wheel, EventHelper::BLOCK);
    mUi->tTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    updateSize();
}

void BackupsWizard::showMore()
{
    mUi->line1->show();
    mUi->line2->show();
    mUi->bShowMore->setText(tr("Collapse"));

    EventManager::addEvent(mUi->tTextEdit->viewport(), QEvent::Wheel, EventHelper::ACCEPT);
    mUi->tTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    updateSize();
}

void BackupsWizard::setCurrentWidgetsSteps(QWidget *widget)
{
    foreach (auto& widget_child, mUi->sSteps->findChildren<QWidget*>())
    {
        mUi->sSteps->removeWidget(widget_child);
    }
    mUi->sSteps->addWidget(widget);
    mUi->sSteps->setCurrentWidget(widget);
}

void BackupsWizard::updateSize()
{
    if (mCurrentStep == STEP_1)
    {
        int nbRows = mFoldersModel->rowCount();
        int listHeight = (std::max(HEIGHT_ROW_STEP_1,
                                   std::min(HEIGHT_ROW_STEP_1 * MAX_ROWS_STEP_1,
                                            nbRows * HEIGHT_ROW_STEP_1)));
        mUi->lvFoldersStep1->setFixedHeight(listHeight);

        bool haveFolders (nbRows);

        mUi->lNoAvailableFolder->setVisible(false);
        mUi->lvFoldersStep1->setVisible(haveFolders);

        int dialogHeight = std::min(HEIGHT_MAX_STEP_1, HEIGHT_MAX_STEP_1
                - (MAX_ROWS_STEP_1 - mFoldersModel->rowCount()) * HEIGHT_ROW_STEP_1);
        setFixedHeight(dialogHeight);
    }
    else if (mCurrentStep == STEP_2)
    {
        int nbRows = mFoldersProxyModel->rowCount();
        int listHeight = (std::max(HEIGHT_ROW_STEP_2,
                                 std::min(HEIGHT_ROW_STEP_2 * MAX_ROWS_STEP_2,
                                          nbRows * HEIGHT_ROW_STEP_2)));
        mUi->lvFoldersStep2->setFixedHeight(listHeight);

        int dialogHeight = std::min(HEIGHT_MAX_STEP_2, HEIGHT_MAX_STEP_2
                - (MAX_ROWS_STEP_2 - nbRows) * HEIGHT_ROW_STEP_2);
        setFixedHeight(dialogHeight);
    }
    else if (mCurrentStep == ERROR_FOUND)
    {
        QFontMetrics fm (mUi->tTextEdit->font());
        QMargins margins (mUi->tTextEdit->contentsMargins());
        int nHeight = margins.top() + margins.bottom();
        int addedHeight = 0;

        if (mUi->line1->isHidden()) // showLess
        {
            nHeight += fm.lineSpacing() * SHOW_MORE_VISIBILITY
                    + mUi->tTextEdit->frameWidth() * 2;
            mUi->tTextEdit->setMinimumHeight(nHeight);
            mUi->tTextEdit->setMaximumHeight(nHeight);
        }
        else // showMore
        {
            mUi->tTextEdit->setMaximumHeight(QWIDGETSIZE_MAX);
            nHeight += fm.lineSpacing () * std::min(SHOW_MORE_MAX_ELEM, mErrList.size() - 1)
                    + mUi->tTextEdit->frameWidth() * 2;
            addedHeight = nHeight;
        }

        setMinimumHeight(FINAL_STEP_MIN_SIZE.height() + addedHeight);
        resize(width(), FINAL_STEP_MIN_SIZE.height() + addedHeight);
    }
}

void BackupsWizard::on_bNext_clicked()
{
    if (mCurrentStep == STEP_1)
    {
        nextStep(STEP_2);
    }
    else
    {
        nextStep(HANDLE_NAME_CONFLICTS);
    }
}

void BackupsWizard::on_bCancel_clicked()
{
    // If the user has made any modification, warn them before exiting.
    if (atLeastOneFolderChecked())
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.text = tr("Are you sure you want to cancel? All changes will be lost.");
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        msgInfo.defaultButton = QMessageBox::No;
        msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
        {
            if(msg->result() == QMessageBox::Yes)
            {
                mUserCancelled = true;
                nextStep(EXIT);
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
    else
    {
        mUserCancelled = true;
        nextStep(EXIT);
    }
}

void BackupsWizard::on_bMoreFolders_clicked()
{
    auto processResult = [this](QStringList paths)
    {
        if(!paths.isEmpty())
        {
            QDir dir (QDir::cleanPath(paths.first()));
            QString path (QDir::toNativeSeparators(dir.canonicalPath()));

            if (!path.isEmpty() && dir.exists())
            {
                auto isSyncable = [this, path](bool canSync)
                {
                    if(canSync)
                    {
                        QStandardItem* existingBackup (nullptr);

                        // Check for path collision.
                        int nbBackups (mFoldersModel->rowCount());
                        int row (0);

                        while (existingBackup == nullptr && row < nbBackups)
                        {
                            auto currItem = mFoldersModel->itemData(mFoldersModel->index(row, 0));
                            if (path == currItem[Qt::UserRole].toString())
                            {
                                // If folder is already backed up, set pointer
                                existingBackup = mFoldersModel->item(row);
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
                            item = new QStandardItem(SyncController::getSyncNameFromPath(path));
                            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                            item->setData(SyncTooltipCreator::createForLocal(path), Qt::ToolTipRole);
                            item->setData(path, Qt::UserRole);
                            item->setData(icon, Qt::DecorationRole);
                            mFoldersModel->insertRow(0, item);
                        }
                        item->setData(Qt::Checked, Qt::CheckStateRole);

                        // Jump to item in list
                        auto idx = mFoldersModel->indexFromItem(item);
                        mUi->lvFoldersStep1->scrollTo(idx, QAbstractItemView::PositionAtCenter);
                        mUi->bMoreFolders->setText(tr("More folders"));
                        mUi->wDeviceNameStep1->setProperty(EMPTY_PROPERTY, false);
                        mUi->wDeviceNameStep1->setStyleSheet(mUi->wDeviceNameStep1->styleSheet());

                        onItemChanged();
                        updateSize();
                    }
                };

                isFolderSyncable(path, isSyncable, true);
            }
        }
    };
    SelectorInfo info;
    info.title = tr("Choose directory");
    info.defaultDir = Utilities::getDefaultBasePath();
    info.parent = this;
    info.func = processResult;
    info.canCreateDirectories = true;
    Platform::getInstance()->folderSelector(info);
}

void BackupsWizard::on_bBack_clicked()
{    
    // The "Back" button only appears at STEP_2. Go back to STEP_1_INIT.
    nextStep(STEP_1);
}

void BackupsWizard::on_bViewInBackupCentre_clicked()
{
    Utilities::openBackupCenter();
    nextStep(EXIT);
}

void BackupsWizard::on_bDismiss_clicked()
{
    nextStep(EXIT);
}

void BackupsWizard::on_bTryAgain_clicked()
{
    nextStep(STEP_1);
}

void BackupsWizard::on_bCancelErr_clicked()
{
    nextStep(EXIT);
}

void BackupsWizard::on_bShowMore_clicked()
{
    if (mUi->tTextEdit->maximumHeight() == QWIDGETSIZE_MAX)
    {
        showLess();
    }
    else
    {
        showMore();
    }
}

void BackupsWizard::onItemChanged(QStandardItem *item)
{
    if (item)
    {
        QString path (item->data(Qt::UserRole).toString());
        auto isSyncable = [this, item, path](bool canSync)
        {
            if(!canSync)
            {
                item->setData(Qt::Unchecked, Qt::CheckStateRole);
            }
        };
        if(item->checkState() == Qt::Checked)
        {
            isFolderSyncable(path, isSyncable, true, true);
        }
    }
    bool enable (false);

    switch (mCurrentStep)
    {
        case STEP_1:
        {
            enable = atLeastOneFolderChecked() && mDeviceNameRequest->isAttributeReady();
            break;
        }
        case STEP_2:
        {
            enable = true;
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

void BackupsWizard::onDeviceNameSet(QString deviceName)
{

    QString elidedTextStep1 = mUi->lDeviceNameStep1->fontMetrics().elidedText(deviceName, Qt::ElideMiddle, 400);
    QString elidedTextStep2 = mUi->lDeviceNameStep1->fontMetrics().elidedText(deviceName, Qt::ElideMiddle, 375);

    if(elidedTextStep1 != deviceName)
    {
        mUi->lDeviceNameStep1->setToolTip(deviceName);
    }

    if(elidedTextStep2 != deviceName)
    {
        mUi->lDeviceNameStep2->setToolTip(deviceName);
    }

    mUi->lDeviceNameStep1->setText(elidedTextStep1);
    mUi->lDeviceNameStep2->setText(elidedTextStep2);

    onMyBackupsFolderHandleSet();
    onItemChanged();
}

void BackupsWizard::onMyBackupsFolderHandleSet(mega::MegaHandle)
{
    // Update path display
    QString text = UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath()
                   + QLatin1Char('/')
                   + mDeviceNameRequest->getDeviceName();
    mUi->leBackupTo->setText(text);
    if(mUi->leBackupTo->fontMetrics().horizontalAdvance(text) > mUi->leBackupTo->width())
    {
        mUi->leBackupTo->setToolTip(text);
    }
    else
    {
        mUi->leBackupTo->setToolTip(QString());
    }
}

void BackupsWizard::onSyncAddRequestStatus(int errorCode, int, const QString& errorMsg, const QString& name)
{
    // Update BackupInfo
    BackupInfo info = mBackupsStatus.value(name);
    if (errorCode == mega::MegaError::API_OK)
    {
        info.status = OK;
    }
    else
    {
        mError = true;
        info.status = ERR;
    }
    mBackupsStatus.insert(name, info);

    //Check if the process is completed or there are still pending requests
    bool finished(true);
    bool errorsExists(false);
    for (auto it = mBackupsStatus.begin(); it != mBackupsStatus.end(); ++it)
    {
        if (it.value().status == QUEUED)
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

    if (finished)
    {
        if (!errorsExists)
        {
            nextStep(DONE);
        }
        else
        {
            // Update tooltip and icon according to result
            mErrList.clear();
            for (auto it = mBackupsStatus.begin(); it != mBackupsStatus.end(); ++it)
            {
                QModelIndex index = mFoldersProxyModel->getIndexByPath(it.key());
                QStandardItem* item = mFoldersModel->itemFromIndex(index);
                if (!item)
                {
                    return;
                }
                // Uncheck the item whatever the status is
                item->setData(Qt::Unchecked, Qt::CheckStateRole);
                if (it.value().status == ERR)
                {
                    QString msg = errorMsg;
                    Text::ClearLink clink;
                    Text::Decorator tc(&clink);
                    tc.process(msg);

                    mErrList.append(it.value().syncName);
                    QIcon   warnIcon (QIcon(QLatin1String("://images/icons/folder/folder-mono-with-warning_24.png")));
                    QString tooltipMsg (SyncTooltipCreator::createForLocal(item->data(Qt::UserRole).toString()) + QChar::LineSeparator
                                        + tr("Error: %1").arg(msg));
                    item->setData(warnIcon, Qt::DecorationRole);
                    item->setData(tooltipMsg, Qt::ToolTipRole);
                }
                else if (it.value().status == OK)
                {
                    QIcon folderIcon (QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")));
                    QString tooltipMsg (SyncTooltipCreator::createForLocal(item->data(Qt::UserRole).toString()));
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

// ProxyModel class ------------------------------------------------------

bool ProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    return !mShowOnlyChecked
            || sourceModel()->index(source_row, 0).data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked;
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
    return QSortFilterProxyModel::flags(index) & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable;
}

void ProxyModel::showOnlyChecked(bool val)
{
    if(val != mShowOnlyChecked)
    {
        mShowOnlyChecked = val;
        invalidateFilter();
    }
}

QModelIndex ProxyModel::getIndexByPath(const QString& path) const
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

// WizardDelegate class ------------------------------------------------------

void WizardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyleOptionViewItem optCopy(option);
    optCopy.index = index;
    bool isS1(isStep1(optCopy.widget));

    // draw the checkbox
    if(isS1 && index.flags().testFlag(Qt::ItemIsUserCheckable))
    {
        optCopy.features |= QStyleOptionViewItem::HasCheckIndicator;
        optCopy.state = optCopy.state & ~QStyle::State_HasFocus;

        // sets the state
        optCopy.state |= index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked ? QStyle::State_On : QStyle::State_Off;

        QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &optCopy, optCopy.widget);
        optCopy.rect = checkBoxRect;

        auto oldAlignment (optCopy.displayAlignment);
        optCopy.displayAlignment |= Qt::AlignVCenter;
        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &optCopy, painter, optCopy.widget);
        optCopy.displayAlignment = oldAlignment;
    }

    // draw the icon
    QIcon::Mode mode = QIcon::Normal;
    if (!(optCopy.state & QStyle::State_Enabled))
        mode = QIcon::Disabled;
    else if (optCopy.state & QStyle::State_Selected)
        mode = QIcon::Selected;

    QRect iconRect = calculateIconRect(optCopy);
    optCopy.rect = iconRect;
    optCopy.icon.paint(painter, optCopy.rect, optCopy.decorationAlignment, mode, optCopy.state & QStyle::State_Open ? QIcon::On : QIcon::Off);

    // draw the text
    QString text = qvariant_cast<QString>(index.data(Qt::DisplayRole));
    if (!text.isEmpty())
    {
        optCopy.text = text;
        QPalette::ColorGroup cg = optCopy.state & QStyle::State_Enabled
                ? QPalette::Normal : QPalette::Disabled;

        if (cg == QPalette::Normal && !(optCopy.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        optCopy.rect = option.rect;
        painter->setPen(optCopy.palette.color(cg, QPalette::Text));
        QRect textRect = calculateTextRect(optCopy, iconRect);
        QString elidedText = painter->fontMetrics().elidedText(optCopy.text, Qt::ElideMiddle, textRect.width() - TEXT_MARGIN);
        painter->drawText(textRect, elidedText, QTextOption(Qt::AlignVCenter | Qt::AlignLeft));
    }

    painter->restore();
}

bool WizardDelegate::editorEvent(QEvent *event,
                            QAbstractItemModel *model,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index)
{
    //Hack to toggle the checkbox when pressing in the row
    if (event->type() == QEvent::MouseButtonRelease)
    {
        Qt::CheckState newState = index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked ? Qt::Unchecked : Qt::Checked;
        model->setData(index, newState, Qt::CheckStateRole);

        //do not call the base implementation, as it will toggle again the checkbox if the user presses just on the checkbox
        return true;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool WizardDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItem optCopy(option);
    optCopy.index = index;

    QRect iconRect = calculateIconRect(optCopy);
    QRect textRect = calculateTextRect(optCopy, iconRect);
    QString text = qvariant_cast<QString>(index.data(Qt::DisplayRole));
    auto textBoundingRect = option.fontMetrics.boundingRect(text);
    textRect.setWidth(textBoundingRect.width());

    if(!textRect.contains(event->pos()))
    {
        event->accept();
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QRect WizardDelegate::calculateIconRect(QStyleOptionViewItem &option) const
{
    QIcon icon =  qvariant_cast<QIcon>(option.index.data(Qt::DecorationRole));
    if(!icon.isNull())
    {
        option.features |= QStyleOptionViewItem::HasDecoration;
    }
    option.icon = icon;

    bool isS1(isStep1(option.widget));
    QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget);
    int offset = isS1 ? ICON_POSITION_STEP_1 : ICON_POSITION_STEP_2;
    iconRect.moveTo(QPoint(offset, iconRect.y()));
    return iconRect;
}

QRect WizardDelegate::calculateTextRect(QStyleOptionViewItem &option, QRect iconRect) const
{
    QRect textRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget);
    if(iconRect.isEmpty())
    {
        iconRect = calculateIconRect(option);
    }
    textRect.moveTo(iconRect.right() + TEXT_MARGIN, textRect.y());
    textRect.setRight(option.rect.right());

    return textRect;
}

bool WizardDelegate::isStep1(const QWidget *view) const
{
    return view->objectName() == QLatin1String("lvFoldersStep1");
}
