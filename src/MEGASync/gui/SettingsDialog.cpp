#include "MegaApplication.h"
#include "SettingsDialog.h"
#include "QMegaMessageBox.h"
#include "ui_SettingsDialog.h"
#include "control/Utilities.h"
#include "platform/Platform.h"
#include "AddExclusionDialog.h"
#include "BandwidthSettings.h"
#include "BugReportDialog.h"
#include "ProxySettings.h"
#include "UserAttributesRequests/FullName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "PowerOptions.h"
#include "syncs/gui/Backups/BackupsWizard.h"
#include "syncs/gui/Backups/AddBackupDialog.h"
#include "syncs/gui/Backups/RemoveBackupDialog.h"
#include "TextDecorator.h"
#include "DialogOpener.h"
#include "syncs/gui/Twoways/BindFolderDialog.h"
#include "GuiUtilities.h"

#include "mega/types.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>
#include <QTranslator>
#include <QMessageBox>
#include <QButtonGroup>
#include <QtConcurrent/QtConcurrent>
#include <QShortcut>
#include <QMenu>

#include <assert.h>
#include <memory>

#ifdef Q_OS_MACOS
    #include "gui/CocoaHelpButton.h"
#endif

#ifdef Q_OS_WINDOWS
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#else
#include "gui/PermissionsDialog.h"
#endif

using namespace mega;

// Common ------------------------------------------------------------------------------------------
#ifdef Q_OS_MACOS
//Const values used for macOS Settings dialog resize animation
constexpr auto SETTING_ANIMATION_PAGE_TIMEOUT{150};//ms
constexpr auto SETTING_ANIMATION_GENERAL_TAB_HEIGHT{646};
constexpr auto SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT{295};//px height
constexpr auto SETTING_ANIMATION_SYNCS_TAB_HEIGHT{539};
constexpr auto SETTING_ANIMATION_BACKUP_TAB_HEIGHT{534};
constexpr auto SETTING_ANIMATION_SECURITY_TAB_HEIGHT{372};
constexpr auto SETTING_ANIMATION_FOLDERS_TAB_HEIGHT{513};
constexpr auto SETTING_ANIMATION_NETWORK_TAB_HEIGHT{205};
constexpr auto SETTING_ANIMATION_NOTIFICATIONS_TAB_HEIGHT{422};
#endif

const QString SYNCS_TAB_MENU_LABEL_QSS = QString::fromUtf8("QLabel{ border-image: url(%1); }");
static constexpr int NUMBER_OF_CLICKS_TO_DEBUG {5};
static constexpr int NETWORK_LIMITS_MAX {9999};

long long calculateCacheSize()
{
    long long cacheSize = 0;
    auto model (SyncInfo::instance());
    for (auto syncType : SyncInfo::AllHandledSyncTypes)
    {
        for (int i = 0; i < model->getNumSyncedFolders(syncType); i++)
        {
            auto syncSetting = model->getSyncSetting(i, syncType);
            QString syncPath = syncSetting->getLocalFolder();
            if (!syncPath.isEmpty())
            {
                Utilities::getFolderSize(syncPath + QDir::separator()
                                         + QString::fromUtf8(MEGA_DEBRIS_FOLDER), &cacheSize);
            }
        }
    }
    return cacheSize;
}

long long calculateRemoteCacheSize(MegaApi* mMegaApi)
{
    MegaNode* n = mMegaApi->getNodeByPath("//bin/SyncDebris");
    long long size = mMegaApi->getSize(n);
    delete n;
    return size;
}

SettingsDialog::SettingsDialog(MegaApplication* app, bool proxyOnly, QWidget* parent) :
    QDialog (parent),
    mUi (new Ui::SettingsDialog),
    mApp (app),
    mPreferences (Preferences::instance()),
    mSyncController (),
    mBackupController (),
    mModel (SyncInfo::instance()),
    mMegaApi (app->getMegaApi()),
    mLoadingSettings (0),
    mThreadPool (ThreadPoolSingleton::getInstance()),
    mCacheSize (-1),
    mRemoteCacheSize (-1),
    mDebugCounter (0)
{
    mSyncTableEventFilter = std::unique_ptr<SyncTableViewTooltips>(new SyncTableViewTooltips());
    mBackupTableEventFilter = std::unique_ptr<BackupTableViewTooltips>(new BackupTableViewTooltips());
    mUi->setupUi(this);

    mUi->bOpenBackupFolder->setEnabled(false);

    // override whatever indexes might be set in .ui files (frequently checked in by mistake)
    mUi->wStack->setCurrentWidget(mUi->pGeneral);
    mUi->wStackFooter->setCurrentWidget(mUi->wGeneralFooter);
    // Add Ctrl+index keyboard shortcut for Settings tabs
    setShortCutsForToolBarItems();

    connect(mUi->wStack, &QStackedWidget::currentChanged, [=](const int &newValue){
          mUi->wStackFooter->setCurrentIndex(newValue);
          //Setting new index in the stack widget cause the focus to be set to footer button
          //avoid it, setting to main wStack to ease tab navigation among different controls.
          mUi->wStack->setFocus();
    });

#ifdef Q_OS_MACOS
    mUi->wStack->setFocus();
#else
    mUi->bGeneral->setChecked(true); // override whatever might be set in .ui
    mUi->gCache->setTitle(mUi->gCache->title().arg(QString::fromUtf8(MEGA_DEBRIS_FOLDER)));
#endif

#ifdef Q_OS_LINUX
    mUi->wUpdateSection->hide();
#endif

    mUi->gExcludedFilesInfo->hide();

#ifdef Q_OS_WINDOWS
    mUi->cFinderIcons->hide();

    typedef LONG MEGANTSTATUS;
    typedef struct _MEGAOSVERSIONINFOW {
        DWORD dwOSVersionInfoSize;
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;
        DWORD dwBuildNumber;
        DWORD dwPlatformId;
        WCHAR  szCSDVersion[128];     // Maintenance string for PSS usage
    } MEGARTL_OSVERSIONINFOW, *PMEGARTL_OSVERSIONINFOW;

    typedef MEGANTSTATUS (WINAPI* RtlGetVersionPtr)(PMEGARTL_OSVERSIONINFOW);
    MEGARTL_OSVERSIONINFOW version = {0};
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning( disable : 4191 ) // 'type cast': unsafe conversion from 'FARPROC' to 'RtlGetVersionPtr'
#endif
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
        if (RtlGetVersion)
        {
            RtlGetVersion(&version);
            if (version.dwMajorVersion >= 10)
            {
                mUi->cFinderIcons->show();
            }
        }
    }
#endif

#ifdef Q_OS_MACOS
    this->setWindowTitle(tr("Settings"));
    mUi->cStartOnStartup->setText(tr("Launch at login"));
    mUi->lLocalDebris->setText(mUi->lLocalDebris->text().arg(QString::fromUtf8(MEGA_DEBRIS_FOLDER)));


    auto current = QOperatingSystemVersion::current();
    if (current <= QOperatingSystemVersion::OSXMavericks) //FinderSync API support from 10.10+
    {
        mUi->cOverlayIcons->hide();
    }

    initializeNativeUIComponents();
#endif

    setProxyOnly(proxyOnly);

#ifdef Q_OS_MACOS
    mMinHeightAnimation = new QPropertyAnimation();
    mMaxHeightAnimation = new QPropertyAnimation();
    mAnimationGroup = new QParallelAnimationGroup();
    mAnimationGroup->addAnimation(mMinHeightAnimation);
    mAnimationGroup->addAnimation(mMaxHeightAnimation);
    connect(mAnimationGroup, &QParallelAnimationGroup::finished,
            this, &SettingsDialog::onAnimationFinished);

    mUi->pSyncs->hide();

    if (!proxyOnly)
    {
        setMinimumHeight(SETTING_ANIMATION_GENERAL_TAB_HEIGHT);
        setMaximumHeight(SETTING_ANIMATION_GENERAL_TAB_HEIGHT);
        mUi->pNetwork->hide();
    }

    macOSretainSizeWhenHidden();
#endif

    mUi->bRestart->hide();

    mApp->attachStorageObserver(*this);
    mApp->attachBandwidthObserver(*this);
    mApp->attachAccountObserver(*this);

    connect(mApp, &MegaApplication::storageStateChanged, this, &SettingsDialog::storageStateChanged);
    storageStateChanged(app->getAppliedStorageState());

    syncsStateInformation(SyncStateInformation::SAVING_SYNCS_FINISHED);
    syncsStateInformation(SyncStateInformation::SAVING_BACKUPS_FINISHED);

    connectSyncHandlers();
    connectBackupHandlers();

    connect(mApp, &MegaApplication::shellNotificationsProcessed,
            this, &SettingsDialog::onShellNotificationsProcessed);
    mUi->cOverlayIcons->setEnabled(!mApp->isShellNotificationProcessingOngoing());

    mUi->syncTableView->installEventFilter(mSyncTableEventFilter.get());
    mUi->backupTableView->installEventFilter(mBackupTableEventFilter.get());
}

SettingsDialog::~SettingsDialog()
{
    mApp->dettachStorageObserver(*this);
    mApp->dettachBandwidthObserver(*this);
    mApp->dettachAccountObserver(*this);

#ifdef Q_OS_MACOS
    mToolBar->deleteLater();
#endif

    delete mUi;
}

void SettingsDialog::openSettingsTab(int tab)
{
    if(mProxyOnly) // do not switch tabs when in guest mode
        return;

    switch (tab)
    {
    case GENERAL_TAB:
#ifndef Q_OS_MACOS
        mUi->bGeneral->click();
#else
        mToolBar->setSelectedItem(bGeneral);
        emit bGeneral->activated();
#endif
        break;

    case ACCOUNT_TAB:
#ifndef Q_OS_MACOS
        mUi->bAccount->click();
#else
        mToolBar->setSelectedItem(bAccount);
        emit bAccount->activated();
#endif
        break;

    case SYNCS_TAB:
#ifndef Q_OS_MACOS
        mUi->bSyncs->click();
#else
        mToolBar->setSelectedItem(bSyncs);
        emit bSyncs->activated();
#endif
        break;

    case BACKUP_TAB:
#ifndef Q_OS_MACOS
        mUi->bBackup->click();
#else
        mToolBar->setSelectedItem(bBackup);
        emit bBackup->activated();
#endif
        break;

    case SECURITY_TAB:
#ifndef Q_OS_MACOS
        mUi->bSecurity->click();
#else
        mToolBar->setSelectedItem(bSecurity);
        emit bSecurity->activated();
#endif
        break;

    case FOLDERS_TAB:
#ifndef Q_OS_MACOS
        mUi->bFolders->click();
#else
        mToolBar->setSelectedItem(bFolders);
        emit bFolders->activated();
#endif
        break;

    case NETWORK_TAB:
#ifndef Q_OS_MACOS
        mUi->bNetwork->click();
#else
        mToolBar->setSelectedItem(bNetwork);
        emit bNetwork->activated();
#endif
        break;

    case NOTIFICATIONS_TAB:
#ifndef Q_OS_MACOS
        mUi->bNotifications->click();
#else
        mToolBar->setSelectedItem(bNotifications);
        emit bNotifications->activated();
#endif
        break;

    default:
        break;
    }
}

void SettingsDialog::setProxyOnly(bool proxyOnly)
{
    mProxyOnly = proxyOnly;

#ifndef Q_OS_MACOS
    mUi->bGeneral->setEnabled(!proxyOnly);
    mUi->bAccount->setEnabled(!proxyOnly);
    mUi->bSyncs->setEnabled(!proxyOnly);
    mUi->bBackup->setEnabled(!proxyOnly);
    mUi->bSecurity->setEnabled(!proxyOnly);
    mUi->bFolders->setEnabled(!proxyOnly);
    mUi->bNotifications->setEnabled(!proxyOnly);
#endif

    if (proxyOnly)
    {
#ifdef Q_OS_MACOS
        // TODO: Re-evaluate sizes for Network tab
        setMinimumHeight(435);
        setMaximumHeight(435);
        mToolBar->setSelectedItem(bNetwork);
#else
        mUi->bNetwork->setEnabled(true);
        mUi->bNetwork->setChecked(true);
#endif
    }
    else
    {
        loadSettings();
    }
}

void SettingsDialog::showGuestMode()
{
    mUi->wStack->setCurrentWidget(mUi->pNetwork);
    mUi->pNetwork->show();
    QPointer<ProxySettings> proxySettingsDialog = new ProxySettings(mApp, this);
    proxySettingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    DialogOpener::showDialog(proxySettingsDialog,
    [proxySettingsDialog, this]()
    {
        if (proxySettingsDialog->result() == QDialog::Accepted)
        {
            mApp->applyProxySettings();
            if (mProxyOnly) accept(); // close Settings in guest mode
        }
        else
        {
            if (mProxyOnly) reject(); // close Settings in guest mode
        }
    });
}

#ifdef Q_OS_MACOS
void SettingsDialog::initializeNativeUIComponents()
{
    CocoaHelpButton* helpButton = new CocoaHelpButton();
    mUi->layoutBottom->insertWidget(0, helpButton);
    connect(helpButton, &CocoaHelpButton::clicked,
            this, &SettingsDialog::on_bHelp_clicked);

    // Set native NSToolBar for settings.
    mToolBar = new QCustomMacToolbar();

    QString general(QString::fromUtf8("settings-general"));
    QString account(QString::fromUtf8("settings-account"));
    QString syncs(QString::fromUtf8("settings-syncs"));
    QString backup(QString::fromUtf8("settings-backup"));
    QString security(QString::fromUtf8("settings-security"));
    QString folders(QString::fromUtf8("settings-folders"));
    QString network(QString::fromUtf8("settings-network"));
    QString notifications(QString::fromUtf8("settings-notifications"));

    // add Items
    bGeneral = mToolBar->addItem(QIcon(), tr("General"));
    mToolBar->customizeIconToolBarItem(bGeneral, general);
    connect(bGeneral, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bGeneral_clicked);

    bAccount=mToolBar->addItem(QIcon(), tr("Account"));
    mToolBar->customizeIconToolBarItem(bAccount, account);
    connect(bAccount, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bAccount_clicked);

    bSyncs=mToolBar->addItem(QIcon(), tr("Sync"));
    mToolBar->customizeIconToolBarItem(bSyncs, syncs);
    connect(bSyncs, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bSyncs_clicked);

    bBackup=mToolBar->addItem(QIcon(), tr("Backup"));
    mToolBar->customizeIconToolBarItem(bBackup, backup);
    connect(bBackup, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bBackup_clicked);

    bSecurity=mToolBar->addItem(QIcon(), tr("Security"));
    mToolBar->customizeIconToolBarItem(bSecurity, security);
    connect(bSecurity, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bSecurity_clicked);

    bFolders=mToolBar->addItem(QIcon(), tr("Folders"));
    mToolBar->customizeIconToolBarItem(bFolders, folders);
    connect(bFolders, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bFolders_clicked);

    bNetwork=mToolBar->addItem(QIcon(), tr("Network"));
    mToolBar->customizeIconToolBarItem(bNetwork, network);
    connect(bNetwork, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bNetwork_clicked);

    bNotifications = mToolBar->addItem(QIcon(), tr("Notifications"));
    mToolBar->customizeIconToolBarItem(bNotifications, notifications);
    connect(bNotifications, &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bNotifications_clicked);

    mToolBar->setSelectableItems(true);
    mToolBar->setAllowsUserCustomization(false);
    mToolBar->setSelectedItem(bGeneral);

    // Attach to the window according Qt docs
    this->window()->winId(); // create window->windowhandle()
    mToolBar->attachToWindowWithStyle(window()->windowHandle(), QCustomMacToolbar::StylePreference);

    // Configure segmented control for +/- syncs
    mUi->wSyncsSegmentedControl->configureTableSegment();
    connect(mUi->wSyncsSegmentedControl, &QSegmentedControl::addButtonClicked,
            this, &SettingsDialog::on_bAddSync_clicked);
    connect(mUi->wSyncsSegmentedControl, &QSegmentedControl::removeButtonClicked,
            this, &SettingsDialog::on_bDeleteSync_clicked);

    mUi->wBackupSegmentedControl->configureTableSegment();
    connect(mUi->wBackupSegmentedControl, &QSegmentedControl::addButtonClicked,
            this, &SettingsDialog::on_bAddBackup_clicked);
    connect(mUi->wBackupSegmentedControl, &QSegmentedControl::removeButtonClicked,
            this, &SettingsDialog::on_bDeleteBackup_clicked);

    mUi->wExclusionsSegmentedControl->configureTableSegment();
    connect(mUi->wExclusionsSegmentedControl, &QSegmentedControl::addButtonClicked,
            this, &SettingsDialog::on_bAddName_clicked);
    connect(mUi->wExclusionsSegmentedControl, &QSegmentedControl::removeButtonClicked,
            this, &SettingsDialog::on_bDeleteName_clicked);
}
#endif

void SettingsDialog::loadSettings()
{
    mLoadingSettings++;

    if (mPreferences->logged())
    {
        connect(&mCacheSizeWatcher, &QFutureWatcher<long long>::finished,
                this, &SettingsDialog::onLocalCacheSizeAvailable);
        QFuture<long long> futureCacheSize = QtConcurrent::run(calculateCacheSize);
        mCacheSizeWatcher.setFuture(futureCacheSize);

        connect(&mRemoteCacheSizeWatcher, &QFutureWatcher<long long>::finished,
                this, &SettingsDialog::onRemoteCacheSizeAvailable);
        QFuture<long long> futureRemoteCacheSize = QtConcurrent::run(calculateRemoteCacheSize,
                                                                     mMegaApi);
        mRemoteCacheSizeWatcher.setFuture(futureRemoteCacheSize);
    }

    //General
    mUi->cFileVersioning->setChecked(!mPreferences->fileVersioningDisabled());
    mUi->cbSleepMode->setChecked(mPreferences->awakeIfActiveEnabled());
    mUi->cOverlayIcons->setChecked(!mPreferences->overlayIconsDisabled());
    mUi->cCacheSchedulerEnabled->setChecked(mPreferences->cleanerDaysLimit());
    mUi->sCacheSchedulerDays->setEnabled(mPreferences->cleanerDaysLimit());
    mUi->sCacheSchedulerDays->setValue(mPreferences->cleanerDaysLimitValue());
    updateCacheSchedulerDaysLabel();

    if (!mPreferences->canUpdate(MegaApplication::applicationFilePath()))
    {
        mUi->bUpdate->setEnabled(false);
        mUi->cAutoUpdate->setEnabled(false);
        mUi->cAutoUpdate->setChecked(false);
    }
    else
    {
        mUi->bUpdate->setEnabled(true);
        mUi->cAutoUpdate->setEnabled(true);
        mUi->cAutoUpdate->setChecked(mPreferences->updateAutomatically());
    }

    // if checked: make sure both sources are true
    mUi->cStartOnStartup->setChecked(mPreferences->startOnStartup()
                                     && Platform::getInstance()->isStartOnStartupActive());

    //Language
    mUi->cLanguage->clear();
    mLanguageCodes.clear();
    QString fullPrefix = Preferences::TRANSLATION_FOLDER + Preferences::TRANSLATION_PREFIX;
    QDirIterator it(Preferences::TRANSLATION_FOLDER);
    QStringList languages;
    int currentIndex = -1;
    QString currentLanguage = mPreferences->language();
    while (it.hasNext())
    {
        QString file = it.next();
        if (file.startsWith(fullPrefix))
        {
            int extensionIndex = file.lastIndexOf(QString::fromUtf8("."));
            if ((extensionIndex - fullPrefix.size()) <= 0)
            {
                continue;
            }

            QString languageCode = file.mid(fullPrefix.size(), extensionIndex-fullPrefix.size());
            QString languageString = Utilities::languageCodeToString(languageCode);
            if (!languageString.isEmpty())
            {
                int i = 0;
                while (i < languages.size() && (languageString > languages[i]))
                {
                    i++;
                }
                languages.insert(i, languageString);
                mLanguageCodes.insert(i, languageCode);
            }
        }
    }

    for (int i = mLanguageCodes.size() - 1; i >= 0; i--)
    {
        if (currentLanguage.startsWith(mLanguageCodes[i]))
        {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex == -1)
    {
        currentIndex = mLanguageCodes.indexOf(QString::fromUtf8("en"));
    }

    mUi->cLanguage->addItems(languages);
    mUi->cLanguage->setCurrentIndex(currentIndex);

    //Account
    mUi->lEmail->setText(mPreferences->email());
    auto fullName ((mPreferences->firstName() + QStringLiteral(" ")
                + mPreferences->lastName()).trimmed());
    mUi->lName->setText(fullName);

    //Update name in case it changes
    auto FullNameRequest = UserAttributes::FullName::requestFullName();
    connect(FullNameRequest.get(), &UserAttributes::FullName::fullNameReady, this, [this](const QString& fullName){
        mUi->lName->setText(fullName);
    });

    // Avatar
    mUi->wAvatar->setUserEmail();

    // account type and details
    updateAccountElements();
    updateStorageElements();
    updateBandwidthElements();

    updateUploadFolder();
    updateDownloadFolder();

    //Syncs
    loadSyncSettings();

    loadBackupSettings();

#ifdef Q_OS_WINDOWS
    mUi->cFinderIcons->setChecked(!mPreferences->leftPaneIconsDisabled());
#endif

    updateNetworkTab();

    // Folders tab
    mUi->lExcludedNames->clear();
    QStringList excludedNames = mPreferences->getExcludedSyncNames();
    for (int i = 0; i < excludedNames.size(); i++)
    {
        mUi->lExcludedNames->addItem(excludedNames[i]);
    }

    QStringList excludedPaths = mPreferences->getExcludedSyncPaths();
    for (int i = 0; i < excludedPaths.size(); i++)
    {
        mUi->lExcludedNames->addItem(excludedPaths[i]);
    }

    for (auto cb : {mUi->cbExcludeUpperUnit, mUi->cbExcludeLowerUnit})
    {
        cb->clear();
        cb->addItem(tr("B"));
        cb->addItem(tr("KB"));
        cb->addItem(tr("MB"));
        cb->addItem(tr("GB"));
    }

    bool upperSizeLimit (mPreferences->upperSizeLimit());
    mUi->eLowerThan->setMaximum(NETWORK_LIMITS_MAX);
    mUi->cExcludeUpperThan->setChecked(upperSizeLimit);
    mUi->eUpperThan->setEnabled(upperSizeLimit);
    mUi->cbExcludeUpperUnit->setEnabled(upperSizeLimit);
    mUi->eUpperThan->setValue(static_cast<int>(mPreferences->upperSizeLimitValue()));
    mUi->cbExcludeUpperUnit->setCurrentIndex(mPreferences->upperSizeLimitUnit());

    bool lowerSizeLimit (mPreferences->lowerSizeLimit());
    mUi->eUpperThan->setMaximum(NETWORK_LIMITS_MAX);
    mUi->cExcludeLowerThan->setChecked(lowerSizeLimit);
    mUi->eLowerThan->setEnabled(lowerSizeLimit);
    mUi->cbExcludeLowerUnit->setEnabled(lowerSizeLimit);
    mUi->eLowerThan->setValue(static_cast<int>(mPreferences->lowerSizeLimitValue()));
    mUi->cbExcludeLowerUnit->setCurrentIndex(mPreferences->lowerSizeLimitUnit());

    mLoadingSettings--;
}

// General -----------------------------------------------------------------------------------------

void deleteCache()
{
    MegaSyncApp->cleanLocalCaches(true);
}

void deleteRemoteCache(MegaApi* mMegaApi)
{
    MegaNode* n = mMegaApi->getNodeByPath("//bin/SyncDebris");
    mMegaApi->remove(n);
    delete n;
}

void SettingsDialog::setOverQuotaMode(bool mode)
{
    if (mode)
    {
        mUi->wOQError->show();
    }
    else
    {
        mUi->wOQError->hide();
    }

    return;
}

void SettingsDialog::setUpdateAvailable(bool updateAvailable)
{
    if (updateAvailable)
    {
        mUi->bUpdate->setText(tr("Install Update"));
    }
    else
    {
        mUi->bUpdate->setText(tr("Check for Updates"));
    }
}

void SettingsDialog::storageChanged()
{
    onCacheSizeAvailable();
}

void SettingsDialog::onLocalCacheSizeAvailable()
{
    mCacheSize = mCacheSizeWatcher.result();
    onCacheSizeAvailable();
}

void SettingsDialog::onRemoteCacheSizeAvailable()
{
    mRemoteCacheSize = mRemoteCacheSizeWatcher.result();
    onCacheSizeAvailable();
}

void SettingsDialog::onSavingSyncsCompleted(SyncStateInformation value)
{
    qint64 startTime(0);
    if(value == SyncStateInformation::SAVING_SYNCS_FINISHED)
    {
       startTime =  mUi->wSpinningIndicatorSyncs->getStartTime();
    }
    else if(value == SyncStateInformation::SAVING_BACKUPS_FINISHED)
    {
        startTime =  mUi->wSpinningIndicatorBackups->getStartTime();
    }
    auto closeDelay = std::max(0ll, 350ll - (QDateTime::currentMSecsSinceEpoch()
                                                 - startTime));
    QTimer::singleShot(closeDelay, this, [this, value] () {
        syncsStateInformation(value);
    });
}

void SettingsDialog::on_bHelp_clicked()
{
    QString helpUrl = Preferences::BASE_URL + QString::fromUtf8("/help/client/megasync");
    Utilities::openUrl(QUrl(helpUrl));
}

#ifdef Q_OS_MACOS
void SettingsDialog::onAnimationFinished()
{
    if (mUi->wStack->currentWidget() == mUi->pGeneral)
    {
        mUi->pGeneral->show();
    }
    else if (mUi->wStack->currentWidget() == mUi->pAccount)
    {
        mUi->pAccount->show();
    }
    else if (mUi->wStack->currentWidget() == mUi->pSyncs)
    {
        mUi->pSyncs->show();
    }
    else if (mUi->wStack->currentWidget() == mUi->pBackup)
    {
        mUi->pBackup->show();
    }
    else if (mUi->wStack->currentWidget() == mUi->pFolders)
    {
        mUi->pFolders->show();
    }
    else if (mUi->wStack->currentWidget() == mUi->pNetwork)
    {
        mUi->pNetwork->show();
    }
    else if (mUi->wStack->currentWidget() == mUi->pSecurity)
    {
        mUi->pSecurity->show();
    }
    else if (mUi->wStack->currentWidget() == mUi->pNotifications)
    {
        mUi->pNotifications->show();
    }
}

void SettingsDialog::animateSettingPage(int endValue, int duration)
{
    mMinHeightAnimation->setTargetObject(this);
    mMaxHeightAnimation->setTargetObject(this);
    mMinHeightAnimation->setPropertyName("minimumHeight");
    mMaxHeightAnimation->setPropertyName("maximumHeight");
    mMinHeightAnimation->setStartValue(minimumHeight());
    mMaxHeightAnimation->setStartValue(maximumHeight());
    mMinHeightAnimation->setEndValue(endValue);
    mMaxHeightAnimation->setEndValue(endValue);
    mMinHeightAnimation->setDuration(duration);
    mMaxHeightAnimation->setDuration(duration);
    mAnimationGroup->start();
}

void SettingsDialog::closeMenus()
{
    auto menus = findChildren<QMenu*>();
    foreach(auto& menu, menus)
    {
        if(dynamic_cast<QAbstractItemView*>(menu->parentWidget()))
        {
            menu->close();
        }
    }
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
    closeMenus();
    QDialog::closeEvent(event);
}

void SettingsDialog::macOSretainSizeWhenHidden()
{
    QSizePolicy spExcludedFiles = mUi->gExcludedFilesInfo->sizePolicy();
    spExcludedFiles.setRetainSizeWhenHidden(true);
    mUi->gExcludedFilesInfo->setSizePolicy(spExcludedFiles);

    QSizePolicy spStorageQuota = mUi->pStorageQuota->sizePolicy();
    spStorageQuota.setRetainSizeWhenHidden(true);
    mUi->pStorageQuota->setSizePolicy(spStorageQuota);

    QSizePolicy spTransferQuota = mUi->pTransferQuota->sizePolicy();
    spTransferQuota.setRetainSizeWhenHidden(true);
    mUi->pTransferQuota->setSizePolicy(spTransferQuota);
}

void SettingsDialog::reloadToolBarItemNames()
{
    bGeneral->setText(tr("General"));
    bAccount->setText(tr("Account"));
    bSyncs->setText(tr("Sync"));
    bBackup->setText(tr("Backup"));
    bSecurity->setText(tr("Security"));
    bFolders->setText(tr("Folders"));
    bNetwork->setText(tr("Network"));
    bNotifications->setText(tr("Notifications"));
}
#endif

void SettingsDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        QString backupsDirPath = UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath();
        mUi->lBackupFolder->setText(backupsDirPath);

#ifdef Q_OS_MACOS
        reloadToolBarItemNames();
        //review and check
        mUi->cStartOnStartup->setText(tr("Launch at login"));

        mUi->lLocalDebris->setText(mUi->lLocalDebris->text().arg(QString::fromUtf8(MEGA_DEBRIS_FOLDER)));
#else
        mUi->gCache->setTitle(mUi->gCache->title().arg(QString::fromUtf8(MEGA_DEBRIS_FOLDER)));
#endif

        onCacheSizeAvailable();
        updateNetworkTab();
        updateStorageElements();
        updateBandwidthElements();
        updateAccountElements();
    }

    QDialog::changeEvent(event);
}

void SettingsDialog::on_bGeneral_clicked()
{
    emit userActivity();

    if ((mUi->wStack->currentWidget() == mUi->pGeneral))
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pGeneral);

#ifdef Q_OS_MACOS
    closeMenus();
    onCacheSizeAvailable();

    mUi->pGeneral->hide();
    animateSettingPage(SETTING_ANIMATION_GENERAL_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bClearCache_clicked()
{
    QString syncs;
    for (auto syncSetting : mModel->getAllSyncSettings())
    {
        QFileInfo fi(syncSetting->getLocalFolder() + QDir::separator()
                     + QString::fromUtf8(MEGA_DEBRIS_FOLDER));
        if (fi.exists() && fi.isDir())
        {
            syncs += QString::fromUtf8("<br/><a href=\"local://#%1\">%2</a>")
                     .arg(fi.absoluteFilePath() + QDir::separator()).arg(syncSetting->name());
        }
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = tr("Clear local backup");
    msgInfo.text = tr("Backups of the previous versions of your synced files in your computer"
                      " will be permanently deleted. Please, check your backup folders to see"
                      " if you need to rescue something before continuing:")
                   + QString::fromUtf8("<br/>") + syncs
                   + QString::fromUtf8("<br/><br/>")
                   + tr("Do you want to delete your local backup now?");
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::No;
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            QtConcurrent::run(deleteCache);
            mCacheSize = 0;
            onCacheSizeAvailable();
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

void SettingsDialog::on_bClearRemoteCache_clicked()
{
    std::shared_ptr<MegaNode> syncDebris(mMegaApi->getNodeByPath("//bin/SyncDebris"));
    if (!syncDebris)
    {
        mRemoteCacheSize = 0;
        return;
    }

    std::unique_ptr<const char[]> base64Handle(syncDebris->getBase64Handle());

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = tr("Clear remote backup");
    msgInfo.text = tr("Backups of the previous versions of your synced files in MEGA will be"
                      " permanently deleted. Please, check your [A] folder in the Rubbish Bin"
                      " of your MEGA account to see if you need to rescue something"
                      " before continuing.")
            .replace(QString::fromUtf8("[A]"),
                     QString::fromUtf8("<a href=\"mega://#fm/%1\">SyncDebris</a>")
                     .arg(QString::fromUtf8(base64Handle.get())))
            + QString::fromUtf8("<br/><br/>")
            + tr("Do you want to delete your remote backup now?");
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::No;
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            QtConcurrent::run(deleteRemoteCache, mMegaApi);
            mRemoteCacheSize = 0;
            onCacheSizeAvailable();
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

void SettingsDialog::on_bClearFileVersions_clicked()
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.text = tr("You are about to permanently remove all file versions."
                      " Would you like to proceed?");
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.textFormat = Qt::RichText;
    msgInfo.defaultButton = QMessageBox::No;
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            mMegaApi->removeVersions(new MegaListenerFuncExecuter(true, [](MegaApi* api,
                                                                 MegaRequest* request, MegaError* e)
            {
                Q_UNUSED(api)
                Q_UNUSED(request)
                if (e->getErrorCode() == MegaError::API_OK)
                {
                    MegaSyncApp->updateUserStats(true, false, false, true, USERSTATS_REMOVEVERSIONS);
                }
            }));
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

void SettingsDialog::on_cCacheSchedulerEnabled_toggled()
{
    if (mLoadingSettings) return;
    bool isEnabled = mUi->cCacheSchedulerEnabled->isChecked();
    mUi->sCacheSchedulerDays->setEnabled(isEnabled);
    mPreferences->setCleanerDaysLimit(isEnabled);
    if(isEnabled)
    {
        mApp->cleanLocalCaches();
    }
}

void SettingsDialog::on_sCacheSchedulerDays_valueChanged(int i)
{
    if (mLoadingSettings) return;
    if(mUi->cCacheSchedulerEnabled->isChecked())
    {
        mPreferences->setCleanerDaysLimitValue(i);
        updateCacheSchedulerDaysLabel();
        mApp->cleanLocalCaches();
    }
}

void SettingsDialog::on_cAutoUpdate_toggled(bool checked)
{
    if (mLoadingSettings) return;
    if (mUi->cAutoUpdate->isEnabled() && (checked != mPreferences->updateAutomatically()))
    {
        mPreferences->setUpdateAutomatically(checked);
        if (checked)
        {
            on_bUpdate_clicked();
        }
    }
}

void SettingsDialog::on_cStartOnStartup_toggled(bool checked)
{
    if (mLoadingSettings) return;
    if (!Platform::getInstance()->startOnStartup(checked))
    {
        // in case of failure - make sure configuration keeps the right value
        //LOG_debug << "Failed to " << (checked ? "enable" : "disable") << " MEGASync on startup.";
        mPreferences->setStartOnStartup(!checked);
    }
    else
    {
        mPreferences->setStartOnStartup(checked);
    }
}

void SettingsDialog::on_cLanguage_currentIndexChanged(int index)
{
    if (mLoadingSettings) return;
    if (index < 0) return; // QComboBox can emit with index -1; do nothing in that case
    QString selectedLanguage = mLanguageCodes[index];
    if (mPreferences->language() != selectedLanguage)
    {
        mPreferences->setLanguage(selectedLanguage);
        mApp->changeLanguage(selectedLanguage);
        updateCacheSchedulerDaysLabel();
        QString currentLanguage = mApp->getCurrentLanguageCode();
        mThreadPool->push([=]()
        {
            mMegaApi->setLanguage(currentLanguage.toUtf8().constData());
            mMegaApi->setLanguagePreference(currentLanguage.toUtf8().constData());
        });
    }
}

void SettingsDialog::on_cFileVersioning_toggled(bool checked)
{
    if (mLoadingSettings) return;
    if (!checked)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.text = tr("Disabling file versioning will prevent"
                          " the creation and storage of new file versions."
                          " Do you want to continue?");
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        msgInfo.defaultButton = QMessageBox::No;
        msgInfo.parent = this;
        msgInfo.finishFunc = [this, checked](QPointer<QMessageBox> msg)
        {
            if(msg->result() == QMessageBox::No)
            {
                mUi->cFileVersioning->blockSignals(true);
                mUi->cFileVersioning->setChecked(true);
                mUi->cFileVersioning->blockSignals(false);
            }
            else
            {
                mMegaApi->setFileVersionsOption(!checked);
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
    else
    {
        // This is actually saved to Preferences after the MegaApi call succeeds;
        mMegaApi->setFileVersionsOption(!checked);
    }
}

void SettingsDialog::on_cbSleepMode_toggled(bool checked)
{
    if (mLoadingSettings) return;

    // This is actually saved to Preferences before calling the keepAwake, as this method uses the setting state;
    mPreferences->setAwakeIfActive(checked);

    PowerOptions options;
    auto result = options.keepAwake(MegaSyncApp->getTransfersModel()->hasActiveTransfers() > 0);

    if (checked && !result)
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = tr("Sleep mode can't be setup");
        msgInfo.text = tr("Your operating system doesn't allow its sleep setting to be overwritten.");
        msgInfo.buttons = QMessageBox::Ok;
        msgInfo.defaultButton = QMessageBox::Ok;
        msgInfo.parent = this;
        msgInfo.finishFunc = [this, checked](QPointer<QMessageBox> msg)
        {
            mUi->cbSleepMode->blockSignals(true);
            mUi->cbSleepMode->setChecked(!checked);
            mPreferences->setAwakeIfActive(!checked);
            mUi->cbSleepMode->blockSignals(false);
        };

        QMegaMessageBox::critical(msgInfo);
    }
}

void SettingsDialog::on_cOverlayIcons_toggled(bool checked)
{
    if (mLoadingSettings) return;
    mUi->cOverlayIcons->setEnabled(false);
    mPreferences->disableOverlayIcons(!checked);
#ifdef Q_OS_MACOS
    Platform::getInstance()->notifyRestartSyncFolders();
#endif
    mApp->notifyChangeToAllFolders();
}

#ifdef Q_OS_WINDOWS
void SettingsDialog::on_cFinderIcons_toggled(bool checked)
{
    if (mLoadingSettings) return;
    if (checked)
    {
        for (auto syncSetting : mModel->getAllSyncSettings())
        {
            Platform::getInstance()->addSyncToLeftPane(syncSetting->getLocalFolder(),
                                        syncSetting->name(),
                                        syncSetting->getSyncID());
        }
    }
    else
    {
        Platform::getInstance()->removeAllSyncsFromLeftPane();
    }
    mPreferences->disableLeftPaneIcons(!checked);
}
#endif

void SettingsDialog::on_bUpdate_clicked()
{
    if (mUi->bUpdate->text() == tr("Check for Updates"))
    {
        mApp->checkForUpdates();
    }
    else
    {
        mApp->triggerInstallUpdate();
    }
}

void SettingsDialog::on_bFullCheck_clicked()
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = tr("Full scan");
    msgInfo.text = tr("MEGAsync will perform a full scan of your synced folders"
                      " when it starts.\n\nDo you want to restart MEGAsync now?");
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::No;
    msgInfo.parent = this;
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
    {
        if(msg->result() == QMessageBox::Yes)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: full re-scan requested");
            mPreferences->setDeleteSdkCacheAtStartup(true);
            MegaSyncApp->rebootApplication(false);
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

void SettingsDialog::on_bSendBug_clicked()
{
    QPointer<BugReportDialog> dialog = new BugReportDialog(this, mApp->getLogger());
    DialogOpener::showDialog(dialog);
}

void SettingsDialog::onCacheSizeAvailable()
{
    if(!mPreferences->logged()) return;

    auto versionsStorage (mPreferences->versionsStorage());

    mUi->lFileVersionsSize->setText(Utilities::getSizeString(versionsStorage));

    if (mCacheSize != -1)
    {
        mUi->lCacheSize->setText(Utilities::getSizeString(mCacheSize));
    }

    if (mRemoteCacheSize != -1)
    {
        mUi->lRemoteCacheSize->setText(Utilities::getSizeString(mRemoteCacheSize));
    }

    mUi->bClearCache->setEnabled(mCacheSize > 0);
    mUi->bClearRemoteCache->setEnabled(mRemoteCacheSize > 0);
    mUi->bClearFileVersions->setEnabled(versionsStorage > 0);
}

// Account -----------------------------------------------------------------------------------------
void SettingsDialog::updateStorageElements()
{
    auto totalStorage = mPreferences->totalStorage();
    auto usedStorage = mPreferences->usedStorage();
    if (totalStorage == 0)
    {
        mUi->pStorageQuota->setValue(0);
        mUi->lStorage->setText(tr("Data temporarily unavailable"));
        mUi->bStorageDetails->setEnabled(false);
    }
    else
    {
        mUi->bStorageDetails->setEnabled(true);

        if (Utilities::isBusinessAccount())
        {
            mUi->lStorage->setText(Utilities::createSimpleUsedString(usedStorage));
        }
        else
        {
            int percentage = Utilities::partPer(usedStorage, totalStorage);

            mUi->pStorageQuota->setValue(std::min(percentage, mUi->pStorageQuota->maximum()));
            mUi->lStorage->setText(Utilities::createCompleteUsedString(usedStorage, totalStorage, percentage));
        }
    }
}

void SettingsDialog::updateBandwidthElements()
{
    int accountType = mPreferences->accountType();
    auto totalBandwidth = mPreferences->totalBandwidth();
    auto usedBandwidth = mPreferences->usedBandwidth();
    mUi->lBandwidthFree->hide();

    if (accountType == Preferences::ACCOUNT_TYPE_FREE)
    {
        mUi->lBandwidth->setText(tr("Used quota for the last %n hour:", "", mPreferences->bandwidthInterval()));
        mUi->lBandwidthFree->show();
        mUi->lBandwidthFree->setText(Utilities::getSizeString(usedBandwidth));
    }
    else if (Utilities::isBusinessAccount())
    {
        mUi->lBandwidth->setText(Utilities::createSimpleUsedString(usedBandwidth));
    }
    else
    {
        if (totalBandwidth == 0)
        {
            mUi->pTransferQuota->setValue(0);
            mUi->lBandwidth->setText(tr("Data temporarily unavailable"));
        }
        else
        {
            int percentage = Utilities::partPer(usedBandwidth, totalBandwidth);
            mUi->pTransferQuota->setValue(std::min(percentage, 100));
            mUi->lBandwidth->setText(Utilities::createCompleteUsedString(usedBandwidth, totalBandwidth, std::min(percentage, 100)));
        }
    }
}

void SettingsDialog::updateAccountElements()
{
    QIcon icon;
    switch(mPreferences->accountType())
    {
        case Preferences::ACCOUNT_TYPE_FREE:
            icon = Utilities::getCachedPixmap(QString::fromLatin1(":/images/Small_Free.png"));
            mUi->lAccountType->setText(tr("Free"));
            mUi->bUpgrade->show();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->hide();
            break;
        case Preferences::ACCOUNT_TYPE_PROI:
            icon = Utilities::getCachedPixmap(QString::fromLatin1(":/images/Small_Pro_I.png"));
            mUi->lAccountType->setText(tr("Pro I"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_PROII:
            icon = Utilities::getCachedPixmap(QString::fromLatin1(":/images/Small_Pro_II.png"));
            mUi->lAccountType->setText(tr("Pro II"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_PROIII:
            icon = Utilities::getCachedPixmap(QString::fromLatin1(":/images/Small_Pro_III.png"));
            mUi->lAccountType->setText(tr("Pro III"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_LITE:
            icon = Utilities::getCachedPixmap(QString::fromLatin1(":/images/Small_Lite.png"));
            mUi->lAccountType->setText(tr("Pro Lite"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_BUSINESS:
            icon = Utilities::getCachedPixmap(QString::fromLatin1(":/images/Small_Business.png"));
            mUi->lAccountType->setText(tr("Business"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->hide();
            mUi->pTransferQuota->hide();
            break;
        case Preferences::ACCOUNT_TYPE_PRO_FLEXI:
            icon = Utilities::getCachedPixmap(QString::fromLatin1(":/images/Small_Pro_Flexi.png"));
            mUi->lAccountType->setText(tr("Pro Flexi"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->hide();
            mUi->pTransferQuota->hide();
            break;
        default:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_I.png"));
            mUi->lAccountType->setText(QString());
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
    }

    mUi->lAccountType->setIcon(icon);
}

void SettingsDialog::storageStateChanged(int newStorageState)
{
     setOverQuotaMode(newStorageState == MegaApi::STORAGE_STATE_RED
                      || newStorageState == MegaApi::STORAGE_STATE_PAYWALL);
}

void SettingsDialog::on_bAccount_clicked()
{
    emit userActivity();

    if ((mUi->wStack->currentWidget() == mUi->pAccount))
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pAccount);

#ifdef Q_OS_MACOS
    closeMenus();
    mUi->pAccount->hide();
    animateSettingPage(SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_lAccountType_clicked()
{
    mDebugCounter++;
    if (mDebugCounter == NUMBER_OF_CLICKS_TO_DEBUG)
    {
        mApp->toggleLogging();
        mDebugCounter = 0;
    }
}

void SettingsDialog::on_bUpgrade_clicked()
{
    QString url = QString::fromUtf8("mega://#pro");
    Utilities::getPROurlWithParameters(url);
    Utilities::openUrl(QUrl(url));
}

void SettingsDialog::on_bBuyMoreSpace_clicked()
{
    on_bUpgrade_clicked();
}

void SettingsDialog::on_bMyAccount_clicked()
{
    Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/account")));
}

void SettingsDialog::on_bStorageDetails_clicked()
{
#ifdef Q_OS_MACOS
    auto accountDetailsDialog = new AccountDetailsDialog(this);
#else
    auto accountDetailsDialog = new AccountDetailsDialog();
#endif

    mApp->updateUserStats(true, true, true, true, USERSTATS_STORAGECLICKED);
    DialogOpener::showNonModalDialog<AccountDetailsDialog>(accountDetailsDialog);
}

void SettingsDialog::on_bLogout_clicked()
{
    QString text;
    bool haveSyncs (false);
    bool haveBackups (false);

    // Check if we have syncs and backups
    if (mModel)
    {
        haveSyncs = !mModel->getSyncSettingsByType(mega::MegaSync::TYPE_TWOWAY).isEmpty();
        haveBackups = !mModel->getSyncSettingsByType(mega::MegaSync::TYPE_BACKUP).isEmpty();
    }

    // Set text according to situation
    if (haveSyncs && haveBackups)
    {
        text = tr("Synchronizations and backups will stop working.");
    }
    else if (haveBackups)
    {
        text = tr("Backups will stop working.");
    }
    else if (haveSyncs)
    {
        text = tr("Synchronizations will stop working.");
    }

    auto unlink = [this](){
        close();
        mApp->unlink();
    };

    if(text.isEmpty())
    {
        unlink();
    }
    else
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = tr("Log out");
        msgInfo.text = text + QLatin1Char(' ') + tr("Are you sure?");
        msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
        msgInfo.defaultButton = QMessageBox::Yes;
        msgInfo.parent = this;
        msgInfo.finishFunc = [this,unlink](QPointer<QMessageBox> msg)
        {
            if(msg->result() == QMessageBox::Yes)
            {
                unlink();
            }
        };

        QMegaMessageBox::question(msgInfo);
    }
}

// Syncs -------------------------------------------------------------------------------------------
void SettingsDialog::connectSyncHandlers()
{
    connect(mUi->syncTableView, &BackupTableView::removeSync, this, &SettingsDialog::removeSync);

    connect(&mSyncController, &SyncController::syncRemoveError, this, [this](std::shared_ptr<mega::MegaError> err)
    {
        onSavingSyncsCompleted(SAVING_SYNCS_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = tr("Error removing sync");
        msgInfo.text = tr("Your sync can't be removed. Reason: %1")
                .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
        QMegaMessageBox::warning(msgInfo);
    });

    connect(&mSyncController, &SyncController::syncEnableError, this, [this](std::shared_ptr<SyncSettings> sync, mega::MegaSync::Error errorCode)
    {
        onSavingSyncsCompleted(SAVING_SYNCS_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = tr("Error enabling sync");
        msgInfo.text = tr("Your sync \"%1\" can't be enabled. Reason: %2")
                .arg(sync->name())
                .arg(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode)));
        QMegaMessageBox::warning(msgInfo);
    });

    connect(&mSyncController, &SyncController::syncDisableError, this, [this](std::shared_ptr<SyncSettings> sync, mega::MegaSync::Error errorCode)
    {
        onSavingSyncsCompleted(SAVING_SYNCS_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = tr("Error disabling sync");
        msgInfo.text = tr("Your sync \"%1\" can't be disabled. Reason: %2")
                .arg(sync->name())
                .arg(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode)));
        QMegaMessageBox::warning(msgInfo);
    });
}

void SettingsDialog::loadSyncSettings()
{
    SyncItemModel *model(new SyncItemModel(mUi->syncTableView));
    model->fillData();
    connect(model, &SyncItemModel::enableSync, this, [this](std::shared_ptr<SyncSettings> sync)
    {
        syncsStateInformation(SyncStateInformation::SAVING_SYNCS);
        mSyncController.enableSync(sync);
    });
    connect(model, &SyncItemModel::disableSync, this, [this](std::shared_ptr<SyncSettings> sync)
    {
        syncsStateInformation(SyncStateInformation::SAVING_SYNCS);
        mSyncController.disableSync(sync);
    });
    connect(model, &SyncItemModel::syncUpdateFinished, this, [this](std::shared_ptr<SyncSettings> syncSetting)
    {
        if(syncSetting->getType() == mega::MegaSync::SyncType::TYPE_TWOWAY)
        {
            onSavingSyncsCompleted(SAVING_SYNCS_FINISHED);
        }
    });
    connectAddSyncHandler();

    SyncItemSortModel *sortModel = new SyncItemSortModel(mUi->syncTableView);
    sortModel->setSourceModel(model);
    mUi->syncTableView->setModel(sortModel);
    mSyncTableEventFilter->setSourceModel(model);
}

void SettingsDialog::addSyncFolder(MegaHandle megaFolderHandle)
{
    auto overQuotaDialog = mApp->showSyncOverquotaDialog();
    if(overQuotaDialog)
    {
        DialogOpener::showDialog(overQuotaDialog, [megaFolderHandle, overQuotaDialog, this]()
        {
            if(overQuotaDialog->result() == QDialog::Rejected)
            {
                addSyncFolderAfterOverQuotaCheck(megaFolderHandle);
            }
        });
    }
    else
    {
        addSyncFolderAfterOverQuotaCheck(megaFolderHandle);
    }
}

void SettingsDialog::addSyncFolderAfterOverQuotaCheck(MegaHandle megaFolderHandle)
{
    QPointer<BindFolderDialog> dialog = new BindFolderDialog(mApp, this);

    if (megaFolderHandle != mega::INVALID_HANDLE)
    {
        dialog->setMegaFolder(megaFolderHandle);
    }

    DialogOpener::showDialog<BindFolderDialog>(dialog, [dialog, this]()
    {
        QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder())
                                                           .canonicalPath());
        

    if (localFolderPath.isEmpty() || dialog->getMegaPath().isEmpty()
        || dialog->getSyncName().isEmpty() || !dialog->getMegaFolder())
    {
        return;
    }

    syncsStateInformation(SyncStateInformation::SAVING_SYNCS);
    mSyncController.addSync(localFolderPath, dialog->getMegaFolder(), dialog->getSyncName(), MegaSync::TYPE_TWOWAY);
    });

}

void SettingsDialog::connectAddSyncHandler()
{
    GuiUtilities::connectAddSyncDefaultHandler(&mSyncController, mPreferences->accountType());
    connect(&mSyncController, &SyncController::syncAddStatus, this, [this](int errorCode, int, const QString, const QString)
    {
        if (errorCode != MegaError::API_OK)
        {
            onSavingSyncsCompleted(SyncStateInformation::SAVING_SYNCS_FINISHED);
        }
    });
}

void SettingsDialog::setEnabledAllControls(const bool enabled)
{
    setGeneralTabEnabled(enabled);
    mUi->pAccount->setEnabled(enabled);
    mUi->pSyncs->setEnabled(enabled);
    mUi->pBackup->setEnabled(enabled);
    mUi->pSecurity->setEnabled(enabled);
    mUi->pFolders->setEnabled(enabled);
    mUi->pNetwork->setEnabled(enabled);
    mUi->pNotifications->setEnabled(enabled);

    mUi->wStackFooter->setEnabled(enabled);
}

void SettingsDialog::setGeneralTabEnabled(const bool enabled)
{
    // We want to keep only the "Send bug report" button enabled.
    // If we call setEnable() on the whole SettingsDialog, it will be
    // disabled and can't be enabled without enabling everything.
    // Another approach is to loop through all child widgets of SettingsDialog,
    // but we need to take care to skip all parents of BugReport button.
    // Experimentally it didn't work, so the last solution is to
    // call setEnable() manually for all controls and leave
    // Bug report controls as they are.

#ifdef Q_OS_MACOS
    mUi->gGeneralDefaultOptions->setEnabled(enabled);
    mUi->gLanguage->setEnabled(enabled);
    mUi->gDebris->setEnabled(enabled);
    mUi->gSyncDebris->setEnabled(enabled);
    mUi->gVersions->setEnabled(enabled);
    mUi->gSleepMode->setEnabled(enabled);
#else
    mUi->gGeneral->setEnabled(enabled);
    mUi->gLanguage->setEnabled(enabled);
    mUi->gCache->setEnabled(enabled);
    mUi->gRemoteCache->setEnabled(enabled);
    mUi->gFileVersions->setEnabled(enabled);
#ifdef Q_OS_LINUX
    mUi->gSleepSettings->setEnabled(enabled);
#else
    mUi->gSleepMode->setEnabled(enabled);
#endif
#endif
}

void SettingsDialog::on_bSyncs_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pSyncs)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pSyncs);

#ifdef Q_OS_MACOS
    closeMenus();
    mUi->pSyncs->hide();
    animateSettingPage(SETTING_ANIMATION_SYNCS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif

    SyncInfo::instance()->dismissUnattendedDisabledSyncs(MegaSync::TYPE_TWOWAY);
}


#ifndef WIN32
void SettingsDialog::on_bPermissions_clicked()
{
    mMegaApi->setDefaultFolderPermissions(mPreferences->folderPermissionsValue());
    int folderPermissions = mMegaApi->getDefaultFolderPermissions();
    mMegaApi->setDefaultFilePermissions(mPreferences->filePermissionsValue());
    int filePermissions = mMegaApi->getDefaultFilePermissions();

    QPointer<PermissionsDialog> dialog = new PermissionsDialog(this);
    dialog->setFolderPermissions(folderPermissions);
    dialog->setFilePermissions(filePermissions);
    DialogOpener::showDialog<PermissionsDialog>(dialog, [dialog, &folderPermissions, &filePermissions, this](){
        if (dialog->result() == QDialog::Accepted)
        {
            filePermissions = dialog->filePermissions();
            folderPermissions = dialog->folderPermissions();

            if (filePermissions != mPreferences->filePermissionsValue()
                    || folderPermissions != mPreferences->folderPermissionsValue())
            {
                mPreferences->setFilePermissionsValue(filePermissions);
                mPreferences->setFolderPermissionsValue(folderPermissions);
            }
        }
    });
}
#endif

void SettingsDialog::on_bAddSync_clicked()
{
    addSyncFolder(mega::INVALID_HANDLE);
}

void SettingsDialog::on_bDeleteSync_clicked()
{
    if(mUi->syncTableView->selectionModel()->hasSelection())
    {
        QModelIndex index = mUi->syncTableView->selectionModel()->selectedRows().first();
        removeSync(index.data(Qt::UserRole).value<std::shared_ptr<SyncSettings>>());
    }
}

void SettingsDialog::syncsStateInformation(SyncStateInformation state)
{
    switch (state)
    {
        case SAVING_SYNCS:
            setEnabledAllControls(false);
            //if we are on sync tab
            mUi->wSpinningIndicatorSyncs->start();
            mUi->sSyncsState->setCurrentWidget(mUi->pSavingSyncs);
            break;
        case SAVING_BACKUPS:
            setEnabledAllControls(false);
            mUi->wSpinningIndicatorBackups->start();
            mUi->sBackupsState->setCurrentWidget(mUi->pSavingBackups);
            break;
        case SAVING_SYNCS_FINISHED:
            setEnabledAllControls(true);
            mUi->wSpinningIndicatorSyncs->stop();
            // If any sync is disabled, shows warning message
            if (mModel->syncWithErrorExist(mega::MegaSync::SyncType::TYPE_TWOWAY))
            {
                mUi->sSyncsState->setCurrentWidget(mUi->pSyncsDisabled);

    #ifdef Q_OS_MACOS
                QString syncs(QString::fromUtf8("settings-syncs-error"));
                mToolBar->customizeIconToolBarItem(bSyncs, syncs);
    #else
               mUi->bSyncs->setIcon(QIcon(QString::fromUtf8(":/images/settings-syncs-warn.png")));
    #endif
             }
             else
             {
                 mUi->sSyncsState->setCurrentWidget(mUi->pNoErrorsSyncs);

    #ifdef Q_OS_MACOS
                QString syncs(QString::fromUtf8("settings-syncs"));
                mToolBar->customizeIconToolBarItem(bSyncs, syncs);
    #else
                 mUi->bSyncs->setIcon(QIcon(QString::fromUtf8(":/images/settings-syncs.png")));
    #endif
             }
            break;
    case SAVING_BACKUPS_FINISHED:
        setEnabledAllControls(true);
        mUi->wSpinningIndicatorBackups->stop();
        // If any sync is disabled, shows warning message
        if (mModel->syncWithErrorExist(mega::MegaSync::SyncType::TYPE_BACKUP))
        {
            mUi->sBackupsState->setCurrentWidget(mUi->pBackupsDisabled);

#ifdef Q_OS_MACOS
            QString backup(QString::fromUtf8("settings-backups-error"));
            mToolBar->customizeIconToolBarItem(bBackup, backup);
#else
            mUi->bBackup->setIcon(QIcon(QString::fromUtf8(":/images/settings-backups-warn.png")));
#endif
         }
         else
         {
            mUi->sBackupsState->setCurrentWidget(mUi->pNoErrorsBackups);

#ifdef Q_OS_MACOS
            QString backup(QString::fromUtf8("settings-backup"));
            mToolBar->customizeIconToolBarItem(bBackup, backup);
#else
            mUi->bBackup->setIcon(QIcon(QString::fromUtf8(":/images/settings-backup.png")));
#endif
         }
        }
}

// Backup ----------------------------------------------------------------------------------------
void SettingsDialog::connectBackupHandlers()
{
    connect(mUi->backupTableView, &BackupTableView::removeBackup, this, &SettingsDialog::removeBackup);
    connect(mUi->backupTableView, &BackupTableView::openInMEGA, this, &SettingsDialog::openHandleInMega);

    auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    connect(myBackupsHandle.get(), &UserAttributes::MyBackupsHandle::attributeReady,
            this, &SettingsDialog::onMyBackupsFolderHandleSet);
    onMyBackupsFolderHandleSet(myBackupsHandle->getMyBackupsHandle());

    connect(&mBackupController, &SyncController::syncAddStatus, this, [this](const int errorCode, const int, const QString errorMsg, QString name)
    {
        if (errorCode != MegaError::API_OK)
        {
            onSavingSyncsCompleted(SyncStateInformation::SAVING_BACKUPS_FINISHED);
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.parent = this;
            msgInfo.title = tr("Error adding backup %1").arg(name);
            msgInfo.text = GuiUtilities::decoratedWithSupportUrl(errorMsg);
            QMegaMessageBox::critical(msgInfo);
        }
    });

    connect(&mBackupController, &SyncController::syncRemoveError, this, [this](std::shared_ptr<mega::MegaError> err)
    {
        onSavingSyncsCompleted(SyncStateInformation::SAVING_BACKUPS_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = tr("Error removing backup");
        msgInfo.text = tr("Your backup can't be removed. Reason: %1")
                .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
        QMegaMessageBox::warning(msgInfo);
    });

    connect(&mBackupController, &SyncController::backupMoveOrRemoveRemoteFolderError, this, [this](std::shared_ptr<mega::MegaError> err)
    {
        onSavingSyncsCompleted(SyncStateInformation::SAVING_BACKUPS_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title = tr("Error moving or removing remote backup folder");
        msgInfo.text = tr("Failed to move or remove the remote backup folder. Reason: %1")
                .arg(QCoreApplication::translate("MegaError", err->getErrorString()));
        QMegaMessageBox::warning(msgInfo);
    });

    connect(&mBackupController, &SyncController::syncEnableError, this, [this](std::shared_ptr<SyncSettings> sync, mega::MegaSync::Error errorCode)
    {
        onSavingSyncsCompleted(SyncStateInformation::SAVING_BACKUPS_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title =  tr("Error enabling backup");
        msgInfo.text = tr("Your backup \"%1\" can't be enabled. Reason: %2")
                .arg(sync->name())
                .arg(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode)));
        QMegaMessageBox::warning(msgInfo);
    });

    connect(&mBackupController, &SyncController::syncDisableError, this, [this](std::shared_ptr<SyncSettings> sync, mega::MegaSync::Error errorCode)
    {
        onSavingSyncsCompleted(SyncStateInformation::SAVING_BACKUPS_FINISHED);
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = this;
        msgInfo.title =  tr("Error disabling backup");
        msgInfo.text = tr("Your backup \"%1\" can't be disabled. Reason: %2")
                .arg(sync->name())
                .arg(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode)));
        QMegaMessageBox::warning(msgInfo);
    });
}

void SettingsDialog::loadBackupSettings()
{
    BackupItemModel *model(new BackupItemModel(mUi->backupTableView));
    model->fillData();
    connect(model, &BackupItemModel::enableSync, this, [this](std::shared_ptr<SyncSettings> sync)
    {
        syncsStateInformation(SyncStateInformation::SAVING_BACKUPS);
        mBackupController.enableSync(sync);
    });
    connect(model, &BackupItemModel::disableSync, this, [this](std::shared_ptr<SyncSettings> sync)
    {
        syncsStateInformation(SyncStateInformation::SAVING_BACKUPS);
        mBackupController.disableSync(sync);
    });
    connect(model, &BackupItemModel::syncUpdateFinished, this, [this](std::shared_ptr<SyncSettings> syncSetting)
    {
        if(syncSetting->getType() == mega::MegaSync::SyncType::TYPE_BACKUP)
        {
            onSavingSyncsCompleted(SAVING_BACKUPS_FINISHED);
        }
    });
    SyncItemSortModel *sortModel = new SyncItemSortModel(mUi->syncTableView);
    sortModel->setSourceModel(model);
    mUi->backupTableView->setModel(sortModel);
    mBackupTableEventFilter->setSourceModel(model);
}

void SettingsDialog::on_bBackup_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pBackup)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pBackup);

#ifdef Q_OS_MACOS
    closeMenus();
    mUi->pBackup->hide();
    animateSettingPage(SETTING_ANIMATION_BACKUP_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif

    SyncInfo::instance()->dismissUnattendedDisabledSyncs(MegaSync::TYPE_BACKUP);
}

void SettingsDialog::on_bAddBackup_clicked()
{
    QPointer<AddBackupDialog> addBackup = new AddBackupDialog(this);
    DialogOpener::showDialog(addBackup,[this, addBackup]()
    {
        if(addBackup->result() == QDialog::Accepted)
        {
            mBackupController.addBackup(addBackup->getSelectedFolder(), addBackup->getBackupName());
            syncsStateInformation(SyncStateInformation::SAVING_BACKUPS);
        }
    });
}

void SettingsDialog::on_bDeleteBackup_clicked()
{
    if(!mUi->backupTableView->selectionModel()->hasSelection())
        return;

    QModelIndex index = mUi->backupTableView->selectionModel()->selectedRows().first();
    std::shared_ptr<SyncSettings> backup = index.data(Qt::UserRole).value<std::shared_ptr<SyncSettings>>();

    if(backup == nullptr)
        return;

    removeBackup(backup);
}

void SettingsDialog::removeBackup(std::shared_ptr<SyncSettings> backup)
{
    QPointer<RemoveBackupDialog> dialog = new RemoveBackupDialog(backup, this);

    DialogOpener::showDialog(dialog,[this, dialog]()
    {
        if(dialog->result() == QDialog::Accepted)
        {
            syncsStateInformation(SyncStateInformation::SAVING_BACKUPS);
            mBackupController.removeSync(dialog->backupToRemove(), dialog->targetFolder());
        }
    });
}

void SettingsDialog::removeSync(std::shared_ptr<SyncSettings> sync)
{
    syncsStateInformation(SyncStateInformation::SAVING_SYNCS);
    mSyncController.removeSync(sync);
}

void SettingsDialog::on_bOpenBackupFolder_clicked()
{
    auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    Utilities::openInMega(myBackupsHandle->getMyBackupsHandle());
}

void SettingsDialog::openHandleInMega(MegaHandle handle)
{
    Utilities::openInMega(handle);
}

void SettingsDialog::on_bBackupCenter_clicked()
{
    Utilities::openBackupCenter();
}

void SettingsDialog::onMyBackupsFolderHandleSet(mega::MegaHandle h)
{
    mUi->lBackupFolder->setText(UserAttributes::MyBackupsHandle::getMyBackupsLocalizedPath());

    if (h == mega::INVALID_HANDLE)
    {
        mUi->bOpenBackupFolder->setEnabled(false);
    }
    else
    {
        mUi->bOpenBackupFolder->setEnabled(true);
    }
}

// Security ----------------------------------------------------------------------------------------
void SettingsDialog::on_bSecurity_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pSecurity)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pSecurity);

#ifdef Q_OS_MACOS
    closeMenus();
    mUi->pSecurity->hide();
    animateSettingPage(SETTING_ANIMATION_SECURITY_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bExportMasterKey_clicked()
{
    QString defaultPath = QDir::toNativeSeparators(Utilities::getDefaultBasePath());
#ifndef _WIN32
    if (defaultPath.isEmpty())
    {
        defaultPath = QString::fromUtf8("/");
    }
#endif

    QDir dir(defaultPath);

    QFileDialog* dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setOptions(QFileDialog::ShowDirsOnly
                      | QFileDialog::DontResolveSymlinks);
    dialog->selectFile(dir.filePath(tr("MEGA-RECOVERYKEY")));
    dialog->setWindowTitle(tr("Export Master key"));
    dialog->setNameFilter(QString::fromUtf8("Txt file (*.txt)"));
    const QStringList schemes = QStringList(QStringLiteral("file"));
    dialog->setSupportedSchemes(schemes);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    DialogOpener::showDialog<QFileDialog>(dialog, [this, dialog]
    {
        if(dialog->result() == QDialog::Accepted)
        {
            auto fileNames = dialog->selectedFiles();

            if (fileNames.isEmpty())
            {
                return;
            }

            QFile file(fileNames.first());
            if (!file.open(QIODevice::WriteOnly | QFile::Truncate))
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.parent = this;
                msgInfo.title =  tr("Unable to write file");
                msgInfo.text = file.errorString();
                QMegaMessageBox::warning(msgInfo);
            }
            else
            {
                QTextStream out(&file);
                out << mMegaApi->exportMasterKey();

                file.close();

                mMegaApi->masterKeyExported();

                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.parent = this;
                msgInfo.title =  QMegaMessageBox::warningTitle();
                msgInfo.text =   tr("Exporting the master key and keeping it in a secure location"
                                    " enables you to set a new password without data loss.")
                        + QString::fromUtf8("\n")
                        + tr("Always keep physical control of your master key (e.g. on a"
                             " client device, external storage, or print).");
                QMegaMessageBox::information(msgInfo);
            }
        }
    });
}

void SettingsDialog::on_bChangePassword_clicked()
{
    QPointer<ChangePassword> cPassword = new ChangePassword(this);
    DialogOpener::showDialog<ChangePassword>(cPassword);
}

void SettingsDialog::on_bSessionHistory_clicked()
{
    Utilities::openUrl(
                      QUrl(QString::fromUtf8("mega://#fm/account/history")));
}

// Folders -----------------------------------------------------------------------------------------
void SettingsDialog::updateUploadFolder()
{
    std::unique_ptr<MegaNode> node (mMegaApi->getNodeByHandle(static_cast<uint64_t>(mPreferences->uploadFolder())));
    if (!node)
    {
        mHasDefaultUploadOption = false;
        mUi->eUploadFolder->setText(QString::fromUtf8("/MEGAsync Uploads"));
    }
    else
    {
        std::unique_ptr<const char[]> nPath(mMegaApi->getNodePath(node.get()));
        if (!nPath)
        {
            mHasDefaultUploadOption = false;
            mUi->eUploadFolder->setText(QString::fromUtf8("/MEGAsync Uploads"));
        }
        else
        {
            mHasDefaultUploadOption = mPreferences->hasDefaultUploadFolder();
            mUi->eUploadFolder->setText(QString::fromUtf8(nPath.get()));
        }
    }
}

void SettingsDialog::updateDownloadFolder()
{
    QString downloadPath = mPreferences->downloadFolder();
    if (!downloadPath.size())
    {
        downloadPath = Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGAsync Downloads");
    }
    downloadPath = QDir::toNativeSeparators(downloadPath);
    mUi->eDownloadFolder->setText(downloadPath);
    mHasDefaultDownloadOption = mPreferences->hasDefaultDownloadFolder();
}

void SettingsDialog::on_bFolders_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pFolders)
    {
        return;
    }
    mUi->wStack->setCurrentWidget(mUi->pFolders);

#ifdef Q_OS_MACOS
    closeMenus();
    mUi->pFolders->hide();
    animateSettingPage(SETTING_ANIMATION_FOLDERS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bUploadFolder_clicked()
{
    UploadNodeSelector* nodeSelector = new UploadNodeSelector(this);
    std::shared_ptr<mega::MegaNode> defaultNode(mMegaApi->getNodeByPath(mUi->eUploadFolder->text().toStdString().c_str()));
    nodeSelector->setSelectedNodeHandle(defaultNode);
    nodeSelector->setDefaultUploadOption(mHasDefaultUploadOption);
    nodeSelector->showDefaultUploadOption();

    DialogOpener::showDialog<NodeSelector>(nodeSelector, [nodeSelector,this]()
    {
        if (nodeSelector->result() == QDialog::Accepted)
        {
            MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedNodeHandle();
            std::shared_ptr<MegaNode> node(mMegaApi->getNodeByHandle(selectedMegaFolderHandle));
            if (node)
            {
                std::unique_ptr<const char[]> nPath(mMegaApi->getNodePath(node.get()));
                if (nPath && std::strlen(nPath.get()))
                {
                    mHasDefaultUploadOption = nodeSelector->getDefaultUploadOption();
                    mUi->eUploadFolder->setText(QString::fromUtf8(nPath.get()));
                    mPreferences->setHasDefaultUploadFolder(mHasDefaultUploadOption);
                    mPreferences->setUploadFolder(static_cast<long long>(node->getHandle()));
                }
            }
        }
    });
}

void SettingsDialog::on_bDownloadFolder_clicked()
{
    QPointer<DownloadFromMegaDialog> dialog = new DownloadFromMegaDialog(
                mPreferences->downloadFolder(), this);
    dialog->setDefaultDownloadOption(mHasDefaultDownloadOption);
    DialogOpener::showDialog<DownloadFromMegaDialog>(dialog, [dialog, this]()
    {
        if (dialog->result() == QDialog::Accepted)
        {
            QString fPath = dialog->getPath();
            if (!fPath.isEmpty())
            {
                QTemporaryFile test(fPath + QDir::separator());
                if(test.open())
                {
                    mHasDefaultDownloadOption = dialog->isDefaultDownloadOption();
                    mUi->eDownloadFolder->setText(fPath);
                    mPreferences->setDownloadFolder(fPath);
                    mPreferences->setHasDefaultDownloadFolder(mHasDefaultDownloadOption);
                }
                else
                {
                    QMegaMessageBox::MessageBoxInfo msgInfo;
                    msgInfo.parent = this;
                    msgInfo.title =  QMegaMessageBox::errorTitle();
                    msgInfo.text =   tr("You don't have write permissions"
                                        " in this local folder.");
                    QMegaMessageBox::critical(msgInfo);
                }
            }
        }
    });
}

void SettingsDialog::on_bAddName_clicked()
{
    QPointer<AddExclusionDialog> add(new AddExclusionDialog(this));
    DialogOpener::showDialog<AddExclusionDialog>(add, [add, this]()
    {
        if (add->result() == QDialog::Accepted)
        {
            QString text = add->textValue();

            if (!text.isEmpty())
            {
                for (int i = 0; i < mUi->lExcludedNames->count(); i++)
                {
                    if (mUi->lExcludedNames->item(i)->text() == text)
                    {
                        return;
                    }
                    else if (mUi->lExcludedNames->item(i)->text().compare(text, Qt::CaseInsensitive) > 0)
                    {
                        mUi->lExcludedNames->insertItem(i, text);
                        saveExcludeSyncNames();
                        return;
                    }
                }

                mUi->lExcludedNames->addItem(text);
                saveExcludeSyncNames();
            }
        }
    });
}

void SettingsDialog::on_bDeleteName_clicked()
{
    QList<QListWidgetItem*> selected = mUi->lExcludedNames->selectedItems();
    if (selected.size() == 0)
    {
        return;
    }

    for (int i = 0; i < selected.size(); i++)
    {
        delete selected[i];
    }

    saveExcludeSyncNames();
}

void SettingsDialog::on_cExcludeUpperThan_clicked()
{
    if (mLoadingSettings) return;
    bool enable (mUi->cExcludeUpperThan->isChecked());
    mPreferences->setUpperSizeLimit(enable);
    mUi->eUpperThan->setEnabled(enable);
    mUi->cbExcludeUpperUnit->setEnabled(enable);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: exclusions updated (upper than toggled)");
    mPreferences->setDeleteSdkCacheAtStartup(true);
}

void SettingsDialog::on_cExcludeLowerThan_clicked()
{
    if (mLoadingSettings) return;
    bool enable (mUi->cExcludeLowerThan->isChecked());
    mPreferences->setLowerSizeLimit(enable);
    mUi->eLowerThan->setEnabled(enable);
    mUi->cbExcludeLowerUnit->setEnabled(enable);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: exclusions updated (lower than toggled)");
    mPreferences->setDeleteSdkCacheAtStartup(true);
}

void SettingsDialog::on_eUpperThan_valueChanged(int i)
{
    if (mLoadingSettings) return;
    mPreferences->setUpperSizeLimitValue(i);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: exclusions updated (upper than updated)");
    mPreferences->setDeleteSdkCacheAtStartup(true);
}

void SettingsDialog::on_eLowerThan_valueChanged(int i)
{
    if (mLoadingSettings) return;
    mPreferences->setLowerSizeLimitValue(i);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: exclusions updated (lower than updated)");
    mPreferences->setDeleteSdkCacheAtStartup(true);
}

void SettingsDialog::on_cbExcludeUpperUnit_currentIndexChanged(int index)
{
    if (mLoadingSettings) return;
    mPreferences->setUpperSizeLimitUnit(index);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: exclusions updated (upper unit updated)");
    mPreferences->setDeleteSdkCacheAtStartup(true);
}

void SettingsDialog::on_cbExcludeLowerUnit_currentIndexChanged(int index)
{
    if (mLoadingSettings) return;
    mPreferences->setLowerSizeLimitUnit(index);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: exclusions updated (lower unit updated)");
    mPreferences->setDeleteSdkCacheAtStartup(true);
}

void SettingsDialog::saveExcludeSyncNames()
{
    QStringList excludedNames;
    QStringList excludedPaths;
    for (int i = 0; i < mUi->lExcludedNames->count(); i++)
    {
        if (mUi->lExcludedNames->item(i)->text().contains(QDir::separator())) // Path exclusion
        {
            excludedPaths.append(mUi->lExcludedNames->item(i)->text());
        }
        else // File name exclusion
        {
            excludedNames.append(mUi->lExcludedNames->item(i)->text());
        }
    }

    mPreferences->setExcludedSyncNames(excludedNames);
    mPreferences->setExcludedSyncPaths(excludedPaths);

    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting deleteSdkCacheAtStartup true: exclusions updated (names)");
    mPreferences->setDeleteSdkCacheAtStartup(true);
}

void SettingsDialog::onShellNotificationsProcessed()
{
    mUi->cOverlayIcons->setEnabled(true);
}

void SettingsDialog::on_bRestart_clicked()
{

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title =  tr("Restart MEGAsync");
    msgInfo.text =   tr("Do you want to restart MEGAsync now?");
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::No;
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg){
        if(msg->result() == QMessageBox::Yes)
        {
            mPreferences->setDeleteSdkCacheAtStartup(true);
            MegaSyncApp->rebootApplication(false);
        }
    };
    QMegaMessageBox::warning(msgInfo);
}

// Network -----------------------------------------------------------------------------------------
void SettingsDialog::on_bNetwork_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pNetwork)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pNetwork);

#ifdef Q_OS_MACOS
    closeMenus();
    mUi->pNetwork->hide();
    animateSettingPage(SETTING_ANIMATION_NETWORK_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bOpenProxySettings_clicked()
{
    QPointer<ProxySettings> proxySettingsDialog(new ProxySettings(mApp, this));
    DialogOpener::showDialog<ProxySettings>(proxySettingsDialog, [proxySettingsDialog, this](){
        if (proxySettingsDialog->result() == QDialog::Accepted)
        {
            mApp->applyProxySettings();
            updateNetworkTab();
        }
    });
}

void SettingsDialog::on_bOpenBandwidthSettings_clicked()
{
    QPointer<BandwidthSettings> bandwidthSettings(new BandwidthSettings(mApp, this));
    DialogOpener::showDialog<BandwidthSettings>(bandwidthSettings, [bandwidthSettings, this](){
        if (bandwidthSettings->result() == QDialog::Accepted)
        {
            mApp->setUploadLimit(std::max(mPreferences->uploadLimitKB(), 0));

            mApp->setMaxUploadSpeed(mPreferences->uploadLimitKB());
            mApp->setMaxDownloadSpeed(mPreferences->downloadLimitKB());

            mApp->setMaxConnections(MegaTransfer::TYPE_UPLOAD,
                                    mPreferences->parallelUploadConnections());
            mApp->setMaxConnections(MegaTransfer::TYPE_DOWNLOAD,
                                    mPreferences->parallelDownloadConnections());

            mApp->setUseHttpsOnly(mPreferences->usingHttpsOnly());

            updateNetworkTab();
        }
    });
}

void SettingsDialog::on_bNotifications_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pNotifications)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pNotifications);

#ifdef Q_OS_MACOS
    closeMenus();
    mUi->pNotifications->hide();
    animateSettingPage(SETTING_ANIMATION_NOTIFICATIONS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::updateNetworkTab()
{
    int uploadLimitKB = mPreferences->uploadLimitKB();
    if (uploadLimitKB < 0)
    {
        mUi->lUploadRateLimit->setText(QCoreApplication::translate("SettingsDialog_Bandwith", "Auto"));
    }
    else if (uploadLimitKB > 0)
    {
        mUi->lUploadRateLimit->setText(QStringLiteral("%1 KB/s").arg(uploadLimitKB));
    }
    else
    {
        mUi->lUploadRateLimit->setText(tr("No limit"));
    }

    int downloadLimitKB = mPreferences->downloadLimitKB();
    if (downloadLimitKB > 0)
    {
        mUi->lDownloadRateLimit->setText(QStringLiteral("%1 KB/s").arg(downloadLimitKB));
    }
    else
    {
        mUi->lDownloadRateLimit->setText(tr("No limit"));
    }

    switch (mPreferences->proxyType())
    {
        case Preferences::PROXY_TYPE_NONE:
            mUi->lProxySettings->setText(tr("No Proxy"));
            break;
        case Preferences::PROXY_TYPE_AUTO:
            mUi->lProxySettings->setText(QCoreApplication::translate("SettingsDialog_Proxies","Auto"));
            break;
        case Preferences::PROXY_TYPE_CUSTOM:
            mUi->lProxySettings->setText(tr("Manual"));
            break;
    }
}

void SettingsDialog::setShortCutsForToolBarItems()
{
    // Provide quick access shortcuts for Settings panes via Ctrl+1,2,3..
    // Ctrl is auto-magically translated to CMD key by Qt on macOS
    for (int i = 0; i < mUi->wStack->count(); ++i)
    {
        QShortcut *scGeneral = new QShortcut(QKeySequence(QString::fromLatin1("Ctrl+%1").arg(i+1)), this);
        QObject::connect(scGeneral, &QShortcut::activated, this, [=](){ openSettingsTab(i); });
    }
}

void SettingsDialog::updateCacheSchedulerDaysLabel()
{
    mUi->lCacheSchedulerSuffix->setText(tr("day", "", mPreferences->cleanerDaysLimitValue()));
}

