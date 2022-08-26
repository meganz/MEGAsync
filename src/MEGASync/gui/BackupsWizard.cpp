#include "BackupsWizard.h"
#include "ui_BackupsWizard.h"
#include "MegaApplication.h"
#include "megaapi.h"
#include "QMegaMessageBox.h"
#include "EventHelper.h"
#include "TextDecorator.h"

#include <QStandardPaths>
#include <QStyleOption>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <QTimer>

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

const int ICON_POSITION_STEP_1(50);
const int ICON_POSITION_STEP_2(3);

const int TEXT_MARGIN(12);


BackupsWizard::BackupsWizard(QWidget* parent) :
    QDialog(parent),
    mUi (new Ui::BackupsWizard),
    mSyncController(),
    mCreateBackupsDir (false),
    mDeviceDirHandle (mega::INVALID_HANDLE),
    mHaveBackupsDir (false),
    mError (false),
    mUserCancelled (false),
    mFoldersModel (new QStandardItemModel(this)),
    mFoldersProxyModel (new ProxyModel(this)),
    mCurrentStep (STEP_1_INIT)
{
    setWindowFlags((windowFlags() | Qt::WindowCloseButtonHint) & ~Qt::WindowContextHelpButtonHint);

#ifdef _WIN32
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
#endif

    mUi->setupUi(this);
    mHighDpiResize.init(this);

    connect(&mSyncController, &SyncController::deviceName,
            this, &BackupsWizard::onDeviceNameSet);

    connect(&mSyncController, &SyncController::myBackupsHandle,
            this, &BackupsWizard::onBackupsDirSet);

    connect(&mSyncController, &SyncController::setMyBackupsStatus,
            this, &BackupsWizard::onSetMyBackupsDirRequestStatus);

    connect(&mSyncController, &SyncController::syncAddStatus,
            this, &BackupsWizard::onSyncAddRequestStatus);

    connect(mFoldersModel, &QStandardItemModel::itemChanged,
            this, &BackupsWizard::onItemChanged);

    // Setup tables
    mFoldersProxyModel->setSourceModel(mFoldersModel);
    mUi->lvFoldersStep1->setModel(mFoldersProxyModel);
    mUi->lvFoldersStep1->setItemDelegate(new WizardDelegate(this));
    mUi->lvFoldersStep2->setModel(mFoldersProxyModel);
    mUi->lvFoldersStep2->setItemDelegate(new WizardDelegate(this));
    mUi->lvFoldersStep1->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mUi->lvFoldersStep2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Format header text
    Text::Bold boldText;
    Text::Decorator tc(&boldText);

    QString titleStep1 = tr("1. [B]Select[/B] folders to backup");
    tc.process(titleStep1);
    mUi->lTitleStep1->setText(titleStep1);

    QString titleStep2 = tr("2. [B]Confirm[/B] backup settings");
    tc.process(titleStep2);
    mUi->lTitleStep2->setText(titleStep2);

    //Setting up the spining window.
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

    // Go to Step 1
    setupStep1();
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

        // mUi->retranslateUi sets text to tr("Next"), so we have
        // to change it if we are not in step 1
        if (mCurrentStep > STEP_1)
        {
            mUi->bNext->setText(tr("Setup"));
        }

        // Same with error "Collapse"/"Show more"
        if (mUi->line1->isVisible() && mUi->line2->isVisible())
        {
            mUi->bShowMore->setText(tr("Collapse"));
        }

        // If the backups root dir has not been created yet, localize the name
        if (mHaveBackupsDir && mCreateBackupsDir)
        {
            mUi->leBackupTo->setText(mSyncController.getMyBackupsLocalizedPath()
                                     + QLatin1Char('/') + mUi->lDeviceNameStep1->text());
        }
    }
    QDialog::changeEvent(event);
}

void BackupsWizard::showLess()
{
    // TODO: use updateSize()
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
    // TODO: use updateSize()
    mUi->line1->show();
    mUi->line2->show();
    mUi->bShowMore->setText(tr("Collapse"));
    mUi->tTextEdit->setMaximumHeight(QWIDGETSIZE_MAX);
    EventManager::addEvent(mUi->tTextEdit->viewport(), QEvent::Wheel, EventHelper::ACCEPT);
    mUi->tTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QFontMetrics fm (mUi->tTextEdit->font());
    QMargins margins = mUi->tTextEdit->contentsMargins ();
    int nHeight = fm.lineSpacing () * ((mErrList.size() > 5 ? 6 : mErrList.size()) - 1)
            + (mUi->tTextEdit->frameWidth ()) * 2 + margins.top () + margins.bottom ();
    setMinimumHeight(FINAL_STEP_MIN_SIZE.height() + nHeight);
    resize(width(), FINAL_STEP_MIN_SIZE.height() + nHeight);
}

void BackupsWizard::onItemChanged(QStandardItem *item)
{
    if (item)
    {
        QString path (item->data(Qt::UserRole).toString());
        if(item->checkState() == Qt::Checked
                && !isFolderSyncable(path, true, true))
        {
            item->setData(Qt::Unchecked, Qt::CheckStateRole);
        }
    }
    bool enable (false);

    switch (mCurrentStep)
    {
        case STEP_1_INIT:
        case STEP_1:
        {
            enable = atLeastOneFolderChecked() && !mUi->lDeviceNameStep1->text().isEmpty();
            break;
        }
        case STEP_2_INIT:
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
    mBackupsStatus.clear();
    mFoldersProxyModel->showOnlyChecked(false);

    qDebug("Backups Wizard: step 1 Init");
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
    mSyncController.getDeviceName();

    // Check if we need to refresh the lists
    if (mFoldersModel->rowCount() == 0)
    {
        // Populate with standard locations
        QIcon folderIcon (QIcon(QLatin1String("://images/icons/folder/folder-mono_24.png")));
        for (auto type : {
             QStandardPaths::DocumentsLocation,
             QStandardPaths::MoviesLocation,
             QStandardPaths::PicturesLocation,
             QStandardPaths::MusicLocation,
             QStandardPaths::DownloadLocation,
             QStandardPaths::DesktopLocation})
        {
            const auto standardPaths (QStandardPaths::standardLocations(type));
            QDir dir (QDir::cleanPath(standardPaths.first()));
            QString path (QDir::toNativeSeparators(dir.canonicalPath()));
            if (dir.exists() && dir != QDir::home() && isFolderSyncable(path))
            {
                QStandardItem* item (new QStandardItem(SyncController::getSyncNameFromPath(path)));
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setData(path, Qt::ToolTipRole);
                item->setData(path, Qt::UserRole);
                item->setData(folderIcon, Qt::DecorationRole);
                item->setData(Qt::Unchecked, Qt::CheckStateRole);
                mFoldersModel->appendRow(item);
            }
            if(mFoldersModel->rowCount() >= MAX_ROWS_STEP_1)
            {
                break;
            }
        }
    }

    if(mFoldersModel->rowCount() == 0)
    {
        mUi->bMoreFolders->setText(tr("Add folders"));
        mUi->wDeviceNameStep1->setStyleSheet(QLatin1String("border-bottom-right-radius: 6px;"
                                                           "border-bottom-left-radius: 6px;"));
    }
    else
    {
        mUi->bMoreFolders->setText(tr("More folders"));
    }

    nextStep(STEP_1);
    updateSize();
}

void BackupsWizard::setupStep2()
{
    mFoldersProxyModel->showOnlyChecked(true);
    qDebug("Backups Wizard: step 2 Init");
    onItemChanged();
    setCurrentWidgetsSteps(mUi->pStep2);
    mUi->bNext->setText(tr("Setup"));
    mUi->bBack->show();

    // Request MyBackups root folder
    mHaveBackupsDir = false;
    mSyncController.getMyBackupsHandle();

    // Get number of items
    int nbSelectedFolders = mFoldersProxyModel->rowCount();

    // Set folders number
    mUi->lFoldersNumber->setText(tr("%n folder", "", nbSelectedFolders));

    nextStep(STEP_2);
    updateSize();
}

void BackupsWizard::setupError()
{
    mUi->bShowMore->setVisible(mErrList.size() > SHOW_MORE_VISIBILITY);

    mUi->lErrorTitle->setText(tr("Problem backing up folder", "", mErrList.size()));
    mUi->lErrorText->setText(tr("This folder wasn't backed up. Try again.", "", mErrList.size()));

    QTextDocument* doc = mUi->tTextEdit->document();
    doc->setDocumentMargin(0);
    QString txt = mErrList.join(QString::fromUtf8("\n"));
    mUi->tTextEdit->setText(txt);

    mUi->tTextEdit->viewport()->setCursor(Qt::ArrowCursor);
    mUi->tTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    mUi->line1->hide();
    mUi->line2->hide();
    mUi->bTryAgain->setFocus();
    setCurrentWidgetsSteps(mUi->pError);

    mUi->sButtons->setCurrentWidget(mUi->pErrorButtons);
    showLess();
}

void BackupsWizard::setupFinalize()
{
    qDebug("Backups Wizard: finalize");
    onItemChanged();
    mUi->bCancel->setEnabled(false);
    mUi->bBack->hide();

    mError = false;

    nextStep(SETUP_MYBACKUPS_DIR);
}

void BackupsWizard::setupMyBackupsDir()
{
    qDebug("Backups Wizard: setup Backups root dir");

    // Create MyBackups folder if necessary
    if (mCreateBackupsDir)
    {
        mSyncController.setMyBackupsDirName();
    }
    else
    {
        // If not, proceed to setting-up backups
        nextStep(SETUP_BACKUPS);
    }
}

void BackupsWizard::setupBackups()
{
    for (int i = 0; i < mFoldersProxyModel->rowCount(); ++i)
    {
        QString path (mFoldersProxyModel->index(i, 0).data(Qt::UserRole).toString());
        QString syncName (mFoldersProxyModel->index(i, 0).data(Qt::DisplayRole).toString());
        mBackupsStatus.insert(path, {QUEUED, syncName});
    }
    processNextBackupSetup();
}

// Indicates if something is checked
bool BackupsWizard::atLeastOneFolderChecked()
{
    for (int i = 0; i < mFoldersModel->rowCount(); ++i)
    {
        if (mFoldersModel->data(mFoldersModel->index(i, 0), Qt::CheckStateRole) == Qt::Checked)
            return true;
    }
    return false;
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

// Checks if a path is syncable.
// Path is considered to be canonical.
bool BackupsWizard::isFolderSyncable(const QString& path, bool displayWarning, bool fromCheckAction)
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
                    message = tr("You can't backup this folder as it's already inside a backed up folder.");
                    syncability = SyncController::CANT_SYNC;
                }
                else if (existingPath.startsWith(inputPath)
                         && existingPath[inputPath.size()] == QDir::separator())
                {
                    message = tr("You can't backup this folder as it contains backed up folders.");
                    syncability = SyncController::CANT_SYNC;
                }
            }
            row++;
        }
    }

    if (displayWarning) // Do not display warning when silenced
    {
        if (syncability == SyncController::CANT_SYNC)
        {
            QMegaMessageBox::warning(nullptr, QString(), message, QMessageBox::Ok);
        }
        else if (syncability == SyncController::WARN_SYNC
                 && fromCheckAction // Display warning on check action only (also called when creating)
                 && (QMegaMessageBox::warning(nullptr, QString(), message
                                              + QLatin1Char('/')
                                              + tr("Do you want to continue?"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                     == QMessageBox::Yes))
        {
            syncability = SyncController::CANT_SYNC;
        }
    }

    return (syncability != SyncController::CANT_SYNC);
}

// State machine orchestrator
void BackupsWizard::nextStep(const Step &step)
{
    qDebug(QString::fromLatin1("Backups Wizard: next step: Step %1").arg(static_cast<int>(step)).toLatin1().constData());
    mCurrentStep = step;

    switch (step)
    {
        case Step::STEP_1_INIT:
        {
            setupStep1();
            break;
        }
        case Step::STEP_1:
        {
            // Wait for user interaction
            break;
        }
        case Step::STEP_2_INIT:
        {
            setupStep2();
            break;
        }
        case Step::STEP_2:
        {
            // Wait for user interaction
            break;
        }
        case Step::FINALIZE:
        {
            setupFinalize();
            break;
        }
        case Step::SETUP_MYBACKUPS_DIR:
        {
            setupMyBackupsDir();
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
            qDebug("Backups Wizard: exit");
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

        //TODO: Remove mUi->lNoAvailableFolder when the final decision with this issue is taken.
        //We can show this label when all disks are synced but we do not allow it as root disk is unsyncable.
        //so currently we do not display this lavel never. If this doesn´t change we have to remove this label.
        mUi->lNoAvailableFolder->setVisible(false/*!haveFolders*/);
        mUi->lvFoldersStep1->setVisible(haveFolders);

        int dialogHeight = std::min(HEIGHT_MAX_STEP_1, HEIGHT_MAX_STEP_1
                - (MAX_ROWS_STEP_1 - mFoldersModel->rowCount()) * HEIGHT_ROW_STEP_1
                /*+ !haveFolders * mUi->lNoAvailableFolder->height()*/);
        setFixedHeight(dialogHeight);
    }
    else if (mCurrentStep == STEP_2)
    {
        int nbRows = mFoldersProxyModel->rowCount();
        int listHeight = (std::max(HEIGHT_ROW_STEP_2,
                                 std::min(HEIGHT_ROW_STEP_2 * MAX_ROWS_STEP_2,
                                          nbRows * HEIGHT_ROW_STEP_2)));// + MARGIN_LV_STEP_2;
        mUi->lvFoldersStep2->setFixedHeight(listHeight);

        int dialogHeight = std::min(HEIGHT_MAX_STEP_2, HEIGHT_MAX_STEP_2
                - (MAX_ROWS_STEP_2 - nbRows) * HEIGHT_ROW_STEP_2);
        setFixedHeight(dialogHeight);
    }
}

void BackupsWizard::on_bNext_clicked()
{
    qDebug("Backups Wizard: next clicked");
    if (mCurrentStep == STEP_1)
    {
        nextStep(STEP_2_INIT);
    }
    else
    {
        nextStep(FINALIZE);
    }
}

void BackupsWizard::on_bCancel_clicked()
{
    int userWantsToCancel (QMessageBox::Yes);

    // If the user has made any modification, warn them before exiting.
    if (atLeastOneFolderChecked())
    {
        QString content (tr("Are you sure you want to cancel? All changes will be lost."));
        userWantsToCancel = QMessageBox::warning(this, QString(), content,
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
                                                 tr("Choose directory"),
                                                 homePaths.first(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks));
    QDir dir (QDir::cleanPath(d));
    QString path (QDir::toNativeSeparators(dir.canonicalPath()));

    if (!d.isEmpty() && dir.exists() && isFolderSyncable(path, true))
    {
        QStandardItem* existingBackup (nullptr);

        // Check for path and name collision.
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
            item->setData(path, Qt::ToolTipRole);
            item->setData(path, Qt::UserRole);
            item->setData(icon, Qt::DecorationRole);
            mFoldersModel->insertRow(0, item);
        }
        item->setData(Qt::Checked, Qt::CheckStateRole);

        // Jump to item in list
        auto idx = mFoldersModel->indexFromItem(item);
        mUi->lvFoldersStep1->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        mUi->bMoreFolders->setText(tr("More folders"));
        mUi->wDeviceNameStep1->setStyleSheet(QLatin1String("border-bottom-right-radius: 0px;"
                                                           "border-bottom-left-radius: 0px;"));
        onItemChanged();
        qDebug() << QString::fromUtf8("Backups Wizard: add folder \"%1\"").arg(path);
        updateSize();
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
    if (mUi->tTextEdit->maximumHeight() == QWIDGETSIZE_MAX)
    {
        showLess();
    }
    else
    {
        showMore();
    }
}

void BackupsWizard::setupComplete()
{
    if (!mError)
    {
        qDebug("Backups Wizard: setup completed successfully");
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
    onItemChanged();
}

void BackupsWizard::onBackupsDirSet(mega::MegaHandle backupsDirHandle)
{
    mHaveBackupsDir = true;

    QString backupsDirPath = mSyncController.getMyBackupsLocalizedPath();

    // If the handle is invalid, program the folder to be created
    mCreateBackupsDir = (backupsDirHandle == mega::INVALID_HANDLE);

    // Update path display
    mUi->leBackupTo->setText(backupsDirPath + QLatin1Char('/') + mUi->lDeviceNameStep1->text());
    onItemChanged();
}

void BackupsWizard::onSetMyBackupsDirRequestStatus(int errorCode, const QString& errorMsg)
{
    if (errorCode == mega::MegaError::API_OK)
    {
        mCreateBackupsDir = false;
        setupMyBackupsDir();
    }
    else
    {
        QMegaMessageBox::critical(nullptr, QString(),
                                  tr("Creating or setting folder \"%1\" as backups root failed.\nReason: %2")
                                  .arg(mSyncController.getMyBackupsLocalizedPath(), errorMsg));
    }
}

void BackupsWizard::onSyncAddRequestStatus(int errorCode, const QString& errorMsg, const QString& name)
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
                    QString tooltipMsg (item->data(Qt::UserRole).toString() + QLatin1Char('\n')
                                        + tr("Error: %1").arg(msg));
                    item->setData(warnIcon, Qt::DecorationRole);
                    item->setData(tooltipMsg, Qt::ToolTipRole);
                }
                else if (it.value().status == OK)
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
    return QSortFilterProxyModel::flags(index)  & ~Qt::ItemIsSelectable & ~Qt::ItemIsEditable;
}

WizardDelegate::WizardDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void WizardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    QStyleOptionViewItem optCopy(option);
    bool isS1(option.widget->objectName() == QLatin1String("lvFoldersStep1"));

    // draw the checkbox
    if(isS1 && index.flags().testFlag(Qt::ItemIsUserCheckable))
    {
        optCopy.features |= QStyleOptionViewItem::HasCheckIndicator;
        optCopy.state = optCopy.state & ~QStyle::State_HasFocus;

        // sets the state
        optCopy.state |= index.data(Qt::CheckStateRole).toBool() ? QStyle::State_On : QStyle::State_Off;

        QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_ViewItemCheckIndicator, &optCopy, optCopy.widget);
        optCopy.rect = checkBoxRect;
        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &optCopy, painter, optCopy.widget);
    }

    // draw the icon
    QIcon::Mode mode = QIcon::Normal;
    if (!(optCopy.state & QStyle::State_Enabled))
        mode = QIcon::Disabled;
    else if (optCopy.state & QStyle::State_Selected)
        mode = QIcon::Selected;
    QIcon::State state = optCopy.state & QStyle::State_Open ? QIcon::On : QIcon::Off;
    QIcon icon =  qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    if(!icon.isNull())
    {
        optCopy.features |= QStyleOptionViewItem::HasDecoration;
    }
    optCopy.icon = icon;
    QRect iconRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &optCopy, optCopy.widget);
    int offset = isS1 ? ICON_POSITION_STEP_1 : ICON_POSITION_STEP_2;
    iconRect.moveTo(QPoint(offset, iconRect.y()));
    optCopy.rect = iconRect;
    optCopy.icon.paint(painter, optCopy.rect, optCopy.decorationAlignment, mode, state);

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
        QRect textRect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &optCopy, optCopy.widget);
        textRect.moveTo(iconRect.right() + TEXT_MARGIN, textRect.y());
        textRect.setRight(option.rect.right());
        QString elidedText = painter->fontMetrics().elidedText(optCopy.text, Qt::ElideMiddle, textRect.width() - TEXT_MARGIN);
        painter->drawText(textRect, elidedText, QTextOption(Qt::AlignVCenter | Qt::AlignLeft));

    }

  painter->restore();
}
