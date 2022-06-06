#include "mega/types.h"
#include "MegaApplication.h"
#include "SettingsDialog.h"
#include "QMegaMessageBox.h"
#include "ui_SettingsDialog.h"
#include "control/Utilities.h"
#include "platform/Platform.h"
#include "gui/AddExclusionDialog.h"
#include "gui/BugReportDialog.h"
#include "gui/QSyncItemWidget.h"
#include "gui/ProxySettings.h"
#include "gui/BandwidthSettings.h"
#include "UserAttributesRequests.h"

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>
#include <QTranslator>
#include <QMessageBox>
#include <QButtonGroup>
#include <QtConcurrent/QtConcurrent>
#include <QShortcut>

#include <assert.h>

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
constexpr auto SETTING_ANIMATION_GENERAL_TAB_HEIGHT{555};
constexpr auto SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT{295};//px height
constexpr auto SETTING_ANIMATION_SYNCS_TAB_HEIGHT{529};
constexpr auto SETTING_ANIMATION_FOLDERS_TAB_HEIGHT{525};
// FIXME: Re-evaluate sizes for Network tab
constexpr auto SETTING_ANIMATION_NETWORK_TAB_HEIGHT{196};
constexpr auto SETTING_ANIMATION_SECURITY_TAB_HEIGHT{372};
constexpr auto SETTING_ANIMATION_NOTIFICATIONS_TAB_HEIGHT{372};
#endif

const QString SYNCS_TAB_MENU_LABEL_QSS = QString::fromUtf8("QLabel{ border-image: url(%1); }");
static constexpr int NUMBER_OF_CLICKS_TO_DEBUG {5};
static constexpr int NETWORK_LIMITS_MAX {9999};

long long calculateCacheSize()
{
    Model* mModel = Model::instance();
    long long cacheSize = 0;
    for (int i = 0; i < mModel->getNumSyncedFolders(); i++)
    {
        auto syncSetting = mModel->getSyncSetting(i);
        QString syncPath = syncSetting->getLocalFolder();
        if (!syncPath.isEmpty())
        {
            Utilities::getFolderSize(syncPath + QDir::separator()
                                     + QString::fromUtf8(MEGA_DEBRIS_FOLDER), &cacheSize);
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
    mController (Controller::instance()),
    mModel (Model::instance()),
    mMegaApi (app->getMegaApi()),
    mLoadingSettings (0),
    mThreadPool (ThreadPoolSingleton::getInstance()),
    mAccountDetailsDialog (nullptr),
    mCacheSize (-1),
    mRemoteCacheSize (-1),
    mDebugCounter (0),
    mAreSyncsDisabled (false),
    mIsSavingSyncsOnGoing (false),
    mSelectedSyncRow(-1)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

    setProxyOnly(proxyOnly);

#ifdef Q_OS_LINUX
    mUi->wUpdateSection->hide();
#endif

    mUi->gExcludedFilesInfo->hide();

    // No more viewing/editing of the old set of exclusions.  .megaingore is taking over
#ifdef Q_OS_MACOS
    mUi->wContainerExclusions->hide();
#else // Q_OS_MACOS
    mUi->bAddName->hide();
    mUi->bDeleteName->hide();
    mUi->excludeButtonsContainer->hide();
    mUi->gExcludeByName->hide();
#endif // ! Q_OS_MACOS
    mUi->lExcludedNames->hide();
    mUi->gExcludeBySize->hide();
    mUi->cbExcludeUpperUnit->hide();
    mUi->eLowerThan->hide();
    mUi->cbExcludeLowerUnit->hide();
    mUi->cExcludeUpperThan->hide();
    mUi->eUpperThan->hide();
    mUi->cExcludeLowerThan->hide();
    mUi->gExcludedFilesInfo->hide();
    mUi->pWarningRestart->hide();
    mUi->lExcludedFilesInfo->hide();


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
    this->setWindowTitle(tr("Preferences"));
    mUi->tSyncs->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
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

    mHighDpiResize.init(this);
    mApp->attachStorageObserver(*this);
    mApp->attachBandwidthObserver(*this);
    mApp->attachAccountObserver(*this);

    connect(mApp, &MegaApplication::avatarReady, this, &SettingsDialog::setAvatar);
    setAvatar();

    connect(mApp, &MegaApplication::storageStateChanged, this, &SettingsDialog::storageStateChanged);
    storageStateChanged(app->getAppliedStorageState());

    connect(mModel, &Model::syncStateChanged, this, &SettingsDialog::onSyncStateChanged);
    connect(mModel, &Model::syncRemoved, this, &SettingsDialog::onSyncStateChanged);

    connect(mUi->tSyncs, &QTableWidget::cellClicked,
            this, &SettingsDialog::onCellClicked);

    connect(mUi->tSyncs->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &SettingsDialog::onSyncSelected);
    mUi->tSyncs->setMouseTracking(true);
    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
}

SettingsDialog::~SettingsDialog()
{
    mApp->dettachStorageObserver(*this);
    mApp->dettachBandwidthObserver(*this);
    mApp->dettachAccountObserver(*this);

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
        mToolBar->setSelectedItem(bGeneral.get());
        emit bGeneral.get()->activated();
#endif
        break;

    case ACCOUNT_TAB:
#ifndef Q_OS_MACOS
        mUi->bAccount->click();
#else
        mToolBar->setSelectedItem(bAccount.get());
        emit bAccount.get()->activated();
#endif
        break;

    case SYNCS_TAB:
#ifndef Q_OS_MACOS
        mUi->bSyncs->click();
#else
        mToolBar->setSelectedItem(bSyncs.get());
        emit bSyncs.get()->activated();
#endif
        break;

    case SECURITY_TAB:
#ifndef Q_OS_MACOS
        mUi->bSecurity->click();
#else
        mToolBar->setSelectedItem(bSecurity.get());
        emit bSecurity.get()->activated();
#endif
        break;

    case FOLDERS_TAB:
#ifndef Q_OS_MACOS
        mUi->bFolders->click();
#else
        mToolBar->setSelectedItem(bFolders.get());
        emit bFolders.get()->activated();
#endif
        break;

    case NETWORK_TAB:
#ifndef Q_OS_MACOS
        mUi->bNetwork->click();
#else
        mToolBar->setSelectedItem(bNetwork.get());
        emit bNetwork.get()->activated();
#endif
        break;

    case NOTIFICATIONS_TAB:
#ifndef Q_OS_MACOS
        mUi->bNotifications->click();
#else
        mToolBar->setSelectedItem(bNotifications.get());
        emit bNotifications.get()->activated();
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
        mToolBar->setSelectedItem(bNetwork.get());
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
    ProxySettings* proxySettingsDialog = new ProxySettings(mApp, this);
    proxySettingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    proxySettingsDialog->setWindowModality(Qt::WindowModal);
    proxySettingsDialog->open();
    connect(proxySettingsDialog, &ProxySettings::finished, this, [this](int result)
    {
        if (result == QDialog::Accepted)
        {
            mApp->applyProxySettings();
            if (mProxyOnly) accept(); // close Settings in guest mode
        }
        else
        {
            if (mProxyOnly) reject(); // close Settings in guest mode
        }
        delete mProxySettingsDialog;
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
    mToolBar = ::mega::make_unique<QCustomMacToolbar>();

    QString general(QString::fromUtf8("settings-general"));
    QString account(QString::fromUtf8("settings-account"));
    QString syncs(QString::fromUtf8("settings-syncs"));
    QString security(QString::fromUtf8("settings-security"));
    QString folders(QString::fromUtf8("settings-folders"));
    QString network(QString::fromUtf8("settings-network"));
    QString notifications(QString::fromUtf8("settings-notifications"));

    // add Items
    bGeneral.reset(mToolBar->addItem(QIcon(), tr("General")));
    mToolBar->customizeIconToolBarItem(bGeneral.get(), general);
    connect(bGeneral.get(), &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bGeneral_clicked);

    bAccount.reset(mToolBar->addItem(QIcon(), tr("Account")));
    mToolBar->customizeIconToolBarItem(bAccount.get(), account);
    connect(bAccount.get(), &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bAccount_clicked);

    bSyncs.reset(mToolBar->addItem(QIcon(), tr("Sync")));
    mToolBar->customizeIconToolBarItem(bSyncs.get(), syncs);
    connect(bSyncs.get(), &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bSyncs_clicked);

    bSecurity.reset(mToolBar->addItem(QIcon(), tr("Security")));
    mToolBar->customizeIconToolBarItem(bSecurity.get(), security);
    connect(bSecurity.get(), &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bSecurity_clicked);

    bFolders.reset(mToolBar->addItem(QIcon(), tr("Folders")));
    mToolBar->customizeIconToolBarItem(bFolders.get(), folders);
    connect(bFolders.get(), &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bFolders_clicked);

    bNetwork.reset(mToolBar->addItem(QIcon(), tr("Network")));
    mToolBar->customizeIconToolBarItem(bNetwork.get(), network);
    connect(bNetwork.get(), &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bNetwork_clicked);

    bNotifications.reset(mToolBar->addItem(QIcon(), tr("Notifications")));
    mToolBar->customizeIconToolBarItem(bNotifications.get(), notifications);
    connect(bNotifications.get(), &QMacToolBarItem::activated,
            this, &SettingsDialog::on_bNotifications_clicked);

    mToolBar->setSelectableItems(true);
    mToolBar->setAllowsUserCustomization(false);
    mToolBar->setSelectedItem(bGeneral.get());

    // Attach to the window according Qt docs
    this->window()->winId(); // create window->windowhandle()
    mToolBar->attachToWindowWithStyle(window()->windowHandle(), QCustomMacToolbar::StylePreference);

    // Configure segmented control for +/- syncs
    mUi->wSyncsSegmentedControl->configureTableSegment();
    connect(mUi->wSyncsSegmentedControl, &QSegmentedControl::addButtonClicked,
            this, &SettingsDialog::on_bAdd_clicked);
    connect(mUi->wSyncsSegmentedControl, &QSegmentedControl::removeButtonClicked,
            this, &SettingsDialog::on_bDelete_clicked);

    mUi->wExclusionsSegmentedControl->configureTableSegment();

#ifndef Q_OS_MACOS
    connect(mUi->wExclusionsSegmentedControl, &QSegmentedControl::addButtonClicked,
            this, &SettingsDialog::on_bAddName_clicked);
    connect(mUi->wExclusionsSegmentedControl, &QSegmentedControl::removeButtonClicked,
            this, &SettingsDialog::on_bDeleteName_clicked);
#endif // ! Q_OS_MACOS
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
                                     && Platform::isStartOnStartupActive());

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
    auto fullNameRequest = UserAttributes::FullNameAttributeRequest::requestFullName(mPreferences->email().toStdString().c_str());
    connect(fullNameRequest.get(), &UserAttributes::FullNameAttributeRequest::attributeReady, this, [this](const QString& fullName){
        mUi->lName->setText(fullName);
    });

    // account type and details
    updateAccountElements();
    updateStorageElements();
    updateBandwidthElements();

    if (mAccountDetailsDialog)
    {
        mAccountDetailsDialog->refresh();
    }

    updateUploadFolder();
    updateDownloadFolder();

    //Syncs
    loadSyncSettings();

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

void SettingsDialog::on_bHelp_clicked()
{
    QString helpUrl = Preferences::BASE_URL + QString::fromUtf8("/help/client/megasync");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(helpUrl));
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

void SettingsDialog::closeEvent(QCloseEvent *event)
{
    emit closeMenus();
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
    bGeneral.get()->setText(tr("General"));
    bAccount.get()->setText(tr("Account"));
    bSyncs.get()->setText(tr("Sync"));
    bSecurity.get()->setText(tr("Security"));
    bFolders.get()->setText(tr("Folders"));
    bNetwork.get()->setText(tr("Network"));
    bNotifications.get()->setText(tr("Notifications"));
}
#endif

void SettingsDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);

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
    emit closeMenus();
    onCacheSizeAvailable();

    mUi->pGeneral->hide();
    animateSettingPage(SETTING_ANIMATION_GENERAL_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bClearCache_clicked()
{
    QString syncs;
    for (int i = 0; i < mModel->getNumSyncedFolders(); i++)
    {
        auto syncSetting = mModel->getSyncSetting(i);
        QFileInfo fi(syncSetting->getLocalFolder() + QDir::separator()
                     + QString::fromUtf8(MEGA_DEBRIS_FOLDER));
        if (fi.exists() && fi.isDir())
        {
            syncs += QString::fromUtf8("<br/><a href=\"local://#%1\">%2</a>")
                    .arg(fi.absoluteFilePath() + QDir::separator()).arg(syncSetting->name());
        }
    }

    QPointer<QMessageBox> warningDel = new QMessageBox(this);
    warningDel->setIcon(QMessageBox::Warning);
    warningDel->setWindowTitle(tr("Clear local backup"));
    warningDel->setTextFormat(Qt::RichText);
    warningDel->setTextInteractionFlags(Qt::NoTextInteraction | Qt::LinksAccessibleByMouse);

    warningDel->setText(tr("Backups of the previous versions of your synced files in your computer"
                           " will be permanently deleted. Please, check your backup folders to see"
                           " if you need to rescue something before continuing:")
                        + QString::fromUtf8("<br/>") + syncs
                        + QString::fromUtf8("<br/><br/>")
                        + tr("Do you want to delete your local backup now?"));
    warningDel->setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    warningDel->setDefaultButton(QMessageBox::No);
    int result = warningDel->exec();
    if (!warningDel || (result != QMessageBox::Yes))
    {
        delete warningDel;
        return;
    }
    delete warningDel;

    QtConcurrent::run(deleteCache);
    mCacheSize = 0;
    onCacheSizeAvailable();
}

void SettingsDialog::on_bClearRemoteCache_clicked()
{
    MegaNode* syncDebris = mMegaApi->getNodeByPath("//bin/SyncDebris");
    if (!syncDebris)
    {
        mRemoteCacheSize = 0;
        return;
    }

    QPointer<QMessageBox> warningDel = new QMessageBox(this);
    warningDel->setIcon(QMessageBox::Warning);
    warningDel->setWindowTitle(tr("Clear remote backup"));
    warningDel->setTextFormat(Qt::RichText);
    warningDel->setTextInteractionFlags(Qt::NoTextInteraction | Qt::LinksAccessibleByMouse);

    char* base64Handle = syncDebris->getBase64Handle();
    warningDel->setText(tr("Backups of the previous versions of your synced files in MEGA will be"
                           " permanently deleted. Please, check your [A] folder in the Rubbish Bin"
                           " of your MEGA account to see if you need to rescue something"
                           " before continuing.")
                        .replace(QString::fromUtf8("[A]"),
                                 QString::fromUtf8("<a href=\"mega://#fm/%1\">SyncDebris</a>")
                                 .arg(QString::fromUtf8(base64Handle)))
                        + QString::fromUtf8("<br/><br/>")
                        + tr("Do you want to delete your remote backup now?"));
    delete [] base64Handle;

    warningDel->setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    warningDel->setDefaultButton(QMessageBox::No);
    int result = warningDel->exec();
    if (!warningDel || (result != QMessageBox::Yes))
    {
        delete warningDel;
        delete syncDebris;
        return;
    }
    delete warningDel;
    delete syncDebris;

    QtConcurrent::run(deleteRemoteCache, mMegaApi);
    mRemoteCacheSize = 0;
    onCacheSizeAvailable();
}

void SettingsDialog::on_bClearFileVersions_clicked()
{
    QPointer<SettingsDialog> dialog = QPointer<SettingsDialog>(this);
    if (QMegaMessageBox::warning(nullptr,
                             QString::fromUtf8("MEGAsync"),
                             tr("You are about to permanently remove all file versions."
                                " Would you like to proceed?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes || !dialog)
    {
        return;
    }

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
    if (!Platform::startOnStartup(checked))
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
        auto answer = QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                                               tr("Disabling file versioning will prevent"
                                                  " the creation and storage of new file versions."
                                                  " Do you want to continue?"),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer == QMessageBox::No)
        {
            mUi->cFileVersioning->blockSignals(true);
            mUi->cFileVersioning->setChecked(true);
            mUi->cFileVersioning->blockSignals(false);
            return;
        }
    }
    // This is actually saved to Preferences after the MegaApi call succeeds;
    mMegaApi->setFileVersionsOption(!checked);
}

void SettingsDialog::on_cOverlayIcons_toggled(bool checked)
{
    if (mLoadingSettings) return;
    mPreferences->disableOverlayIcons(!checked);
#ifdef Q_OS_MACOS
    Platform::notifyRestartSyncFolders();
#else
    for (int i = 0; i < mModel->getNumSyncedFolders(); i++)
    {
        auto syncSetting = mModel->getSyncSetting(i);
        mApp->notifyItemChange(syncSetting->getLocalFolder(), MegaApi::STATE_NONE);
    }
#endif
}

#ifdef Q_OS_WINDOWS
void SettingsDialog::on_cFinderIcons_toggled(bool checked)
{
    if (mLoadingSettings) return;
    if (checked)
    {
        for (int i = 0; i < mModel->getNumSyncedFolders(); i++)
        {
            auto syncSetting = mModel->getSyncSetting(i);
            Platform::addSyncToLeftPane(syncSetting->getLocalFolder(),
                                        syncSetting->name(),
                                        syncSetting->getSyncID());
        }
    }
    else
    {
        Platform::removeAllSyncsFromLeftPane();
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
    mMegaApi->rescanSync(INVALID_HANDLE);
}

void SettingsDialog::on_bSendBug_clicked()
{
    QPointer<BugReportDialog> dialog = new BugReportDialog(this, mApp->getLogger());
    int result = dialog->exec();
    if (!dialog || (result != QDialog::Accepted))
    {
        delete dialog;
        return;
    }

    delete dialog;
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
    int accountType = mPreferences->accountType();

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

        if (accountType == Preferences::ACCOUNT_TYPE_BUSINESS)
        {
            mUi->lStorage->setText(tr("%1 used").arg(Utilities::getSizeString(usedStorage)));
        }
        else
        {
            int percentage = Utilities::partPer(usedStorage, totalStorage);
            mUi->pStorageQuota->setValue(std::min(percentage, mUi->pStorageQuota->maximum()));
            mUi->lStorage->setText(tr("%1 (%2%) of %3 used").arg(
                                      Utilities::getSizeString(usedStorage),
                                      QString::number(percentage),
                                      Utilities::getSizeString(totalStorage)));
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
    else if (accountType == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        mUi->lBandwidth->setText(tr("%1 used").arg(Utilities::getSizeString(usedBandwidth)));
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
            mUi->lBandwidth->setText(tr("%1 (%2%) of %3 used").arg(
                                        Utilities::getSizeString(usedBandwidth),
                                        QString::number(std::min(percentage, 100)),
                                        Utilities::getSizeString(totalBandwidth)));
        }
    }
}

void SettingsDialog::updateAccountElements()
{
    QIcon icon;
    switch(mPreferences->accountType())
    {
        case Preferences::ACCOUNT_TYPE_FREE:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Free.png"));
            mUi->lAccountType->setText(tr("Free"));
            mUi->bUpgrade->show();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->hide();
            break;
        case Preferences::ACCOUNT_TYPE_PROI:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_I.png"));
            mUi->lAccountType->setText(tr("Pro I"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_PROII:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_II.png"));
            mUi->lAccountType->setText(tr("Pro II"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_PROIII:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_III.png"));
            mUi->lAccountType->setText(tr("Pro III"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_LITE:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Lite.png"));
            mUi->lAccountType->setText(tr("Pro Lite"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->show();
            mUi->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_BUSINESS:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Business.png"));
            mUi->lAccountType->setText(tr("Business"));
            mUi->bUpgrade->hide();
            mUi->pStorageQuota->hide();
            mUi->pTransferQuota->hide();
            break;
        default:
        // FIXME: is this correct?
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
    emit closeMenus();
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
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
}

void SettingsDialog::on_bBuyMoreSpace_clicked()
{
    on_bUpgrade_clicked();
}

void SettingsDialog::on_bMyAccount_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#fm/account")));
}

void SettingsDialog::on_bStorageDetails_clicked()
{
    mAccountDetailsDialog = new AccountDetailsDialog(this);
    mApp->updateUserStats(true, true, true, true, USERSTATS_STORAGECLICKED);
    QPointer<AccountDetailsDialog> dialog = mAccountDetailsDialog;
    dialog->exec();
    if (!dialog)
    {
        return;
    }

    delete mAccountDetailsDialog;
    mAccountDetailsDialog = nullptr;
}

void SettingsDialog::on_bLogout_clicked()
{
    QPointer<SettingsDialog> currentDialog = this;
    if (QMegaMessageBox::question(nullptr, tr("Log out"),
                                  tr("Synchronization will stop working. Are you sure?"),
                                  QMessageBox::Yes|QMessageBox::No)
            == QMessageBox::Yes)
    {
        if (currentDialog)
        {
            close();
            mApp->unlink();
        }
    }
}

void SettingsDialog::setAvatar()
{
    const char* email = mMegaApi->getMyEmail();
    if (email)
    {
        mUi->wAvatar->setUserEmail(email);
        delete [] email;
    }
}

// Syncs -------------------------------------------------------------------------------------------

void SettingsDialog::loadSyncSettings()
{
    mUi->tSyncs->clearContents();
    mSyncNames.clear();

    // TODO: incomplete: we also neeed a copy of configuredSyncs from theh model to iterate
    // in the right order.
    //Get a snapshot of current syncs to avoid possible issues if some of them are removed during loop op
//    QMap<MegaHandle, std::shared_ptr<SyncSetting>> syncs = mModel->getCopyOfSettings();

    mUi->tSyncs->horizontalHeader()->setVisible(true);
    mUi->tSyncs->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    int numFolders = mModel->getNumSyncedFolders();
    mUi->tSyncs->setRowCount(numFolders);
    mUi->tSyncs->setColumnCount(SYNC_COL_NB);
    mUi->tSyncs->setColumnHidden(SYNC_COL_TAG, true);    //hidden tag
    mUi->tSyncs->setColumnHidden(SYNC_COL_HANDLE, true); //hidden handle
    mUi->tSyncs->setColumnHidden(SYNC_COL_NAME, true);   //hidden name
    mUi->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_ENABLE_CB, QHeaderView::Fixed);
    mUi->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_MENU, QHeaderView::Fixed);
    mUi->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_DISPLAY_NAME,QHeaderView::Stretch);
    mUi->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_RUN_STATE,QHeaderView::Stretch);
    int miniMumSectionSize (mUi->tSyncs->horizontalHeader()->minimumSectionSize());
    mUi->tSyncs->horizontalHeader()->resizeSection(SYNC_COL_ENABLE_CB, miniMumSectionSize);
    mUi->tSyncs->horizontalHeader()->resizeSection(SYNC_COL_MENU, miniMumSectionSize);

    // New check up. Need to reset, syncs state could have changed
    mAreSyncsDisabled = false;

    for (int i = 0; i < numFolders; i++)
    {
        auto syncSetting = mModel->getSyncSetting(i);
        if (!syncSetting)
        {
            assert("A sync has been deleting while trying to loop in");
            continue;
        }

        // Check if current sync is disabled by an error.
        mAreSyncsDisabled = mAreSyncsDisabled || static_cast<bool>(syncSetting->getError());

        addSyncRow(i, syncSetting->name(), syncSetting->getLocalFolder(),
                   syncSetting->getMegaFolder(),
                   syncSetting->getSync()->getRunState() != MegaSync::RUNSTATE_SUSPENDED  &&
                   syncSetting->getSync()->getRunState() != MegaSync::RUNSTATE_DISABLED,
                   syncSetting->getRunStateAsString(),
                   syncSetting->getError(), syncSetting->getMegaHandle(),
                   syncSetting->backupId(), syncSetting);
    }
    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
}

void SettingsDialog::addSyncFolder(MegaHandle megaFolderHandle)
{
    const bool dismissed{mApp->showSyncOverquotaDialog()};
    if(!dismissed)
    {
        return;
    }

    QStringList currentLocalFolders;
    QStringList currentMegaFoldersPaths;
    for (int i = 0; i < mUi->tSyncs->rowCount(); i++)
    {
        //notice: this also takes into account !active ones
        QWidget* w = mUi->tSyncs->cellWidget(i, SYNC_COL_DISPLAY_NAME);
        currentLocalFolders.append(static_cast<QSyncItemWidget*>(w)->getLocalPath());
        currentMegaFoldersPaths.append(static_cast<QSyncItemWidget*>(w)->getRemotePath());
    }

    QPointer<BindFolderDialog> dialog = new BindFolderDialog(mApp, mSyncNames, currentLocalFolders,
                                                             currentMegaFoldersPaths, this);
    if (megaFolderHandle != mega::INVALID_HANDLE)
    {
        dialog->setMegaFolder(megaFolderHandle);
    }

    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder())
                                                       .canonicalPath());
    if (!localFolderPath.length() || !dialog->getMegaPath().size())
    {
        delete dialog;
        return;
    }

    ActionProgress* action
        = new ActionProgress(true, QString::fromUtf8("Add sync"));

    connect(action, &ActionProgress::failedRequest,
        this, [this](MegaRequest* request, MegaError* error)
        {
            Q_UNUSED(request)
            if (error->getErrorCode())
                showUnexpectedSyncError(tr("Unexpected error adding sync"));
        }, Qt::DirectConnection);

    mController->addSync(localFolderPath, dialog->megaPathHandle, dialog->getSyncName(), action);

    delete dialog;
}

void SettingsDialog::onSyncStateChanged(std::shared_ptr<SyncSetting>)
{
    loadSyncSettings();
}

// errorCode reffers to the requestErrorCode.
void SettingsDialog::onEnableSyncFailed(int errorCode, std::shared_ptr<SyncSetting> syncSetting)
{
    switch (errorCode)
    {
        case MegaSync::Error::NO_SYNC_ERROR:
        {
            assert(false && "unexpected no error after enabling failed");
            return;
        }
        default:
        {
            QMegaMessageBox::critical(nullptr, tr("Error enabling sync"),
                                      tr("Your sync \"%1\" can't be enabled. Reason: %2").arg(
                                          syncSetting->name(),
                                          tr(MegaSync::getMegaSyncErrorCode(errorCode))));
            break;
        }
    }

    loadSyncSettings();
}

void SettingsDialog::onDisableSyncFailed(std::shared_ptr<SyncSetting> syncSetting)
{
    QMegaMessageBox::critical(nullptr, tr("Error"),
                              tr("Unexpected error disabling sync %1").arg(syncSetting->name()));
}

void SettingsDialog::onSyncSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)

    int row (-1);

    // Get selected row
    if (!selected.isEmpty())
    {
        row = selected.first().indexes().first().row();
        mSelectedSyncRow = row;
    }
    else if (mSelectedSyncRow != -1)
    {
        row = mSelectedSyncRow;
    }
    else
    {
        mSelectedSyncRow = -1;
    }

    for (int i = 0; i < mUi->tSyncs->rowCount(); ++i)
    {
        bool select (false);
        QString menuRsc (QString::fromUtf8("://images/Item_options_rest.png"));

        if (i == row)
        {
            menuRsc = QString::fromUtf8("://images/Item_options_press.png");
            select = true;
        }

        // Paths
        auto w (qobject_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(i, SYNC_COL_DISPLAY_NAME)));
        // Check if the row has not been deleted
        if (w)
        {
            w->setSelected(select);
            w = qobject_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(i, SYNC_COL_RUN_STATE));
            w->setSelected(select);

            // Menu
            auto lMenu (qobject_cast<QWidget*>(mUi->tSyncs->cellWidget(i, SYNC_COL_MENU)));
            lMenu->setStyleSheet(SYNCS_TAB_MENU_LABEL_QSS.arg(menuRsc));
        }
    }
}

void SettingsDialog::on_bSyncs_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pSyncs)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pSyncs);
    mUi->tSyncs->horizontalHeader()->setVisible(true);

#ifdef Q_OS_MACOS
    emit closeMenus();
    mUi->pSyncs->hide();
    animateSettingPage(SETTING_ANIMATION_SYNCS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bAdd_clicked()
{
    addSyncFolder(mega::INVALID_HANDLE);
}

void SettingsDialog::on_bDelete_clicked()
{
    if (mSelectedSyncRow >= 0 && mSelectedSyncRow < mUi->tSyncs->rowCount())
        onDeleteSync();
}

void SettingsDialog::on_tSyncs_doubleClicked(const QModelIndex& index)
{
    //FIXME: When using custom widget for row items, remove double check or use cellwidget to fix it.
    auto widget = static_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(index.row(), SYNC_COL_DISPLAY_NAME));

    if (!index.column())
    {
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(widget->getLocalPath()));
        return;
    }

    auto remotePath = widget->getRemotePath();
    auto node = mMegaApi->getNodeByPath(remotePath.toUtf8().constData());

    if (!node)
        return;

    auto handle = node->getBase64Handle();
    auto url = QString::fromUtf8("mega://#fm/") + QString::fromUtf8(handle);

    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));

    delete[] handle;
    delete node;
}

void SettingsDialog::onCellClicked(int row, int column)
{
    if (column != SYNC_COL_MENU)
        return;

    mSelectedSyncRow = row;

    QMenu *menu(new QMenu(mUi->tSyncs));
    menu->setAttribute(Qt::WA_TranslucentBackground);
#if defined(Q_OS_WINDOWS)
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
#elif defined(Q_OS_MACOS)
    connect(this, &SettingsDialog::closeMenus, menu, &SettingsDialog::close);
#endif

    // Show in explorer action
    auto showLocalAction (new MenuItemAction(QCoreApplication::translate("Platform", Platform::fileExplorerString),
                                             QIcon(QString::fromUtf8("://images/show_in_folder_ico.png"))));
    connect(showLocalAction, &MenuItemAction::triggered,
            this, &SettingsDialog::showInFolderClicked);

    // Show in Mega action
    auto showRemoteAction (new MenuItemAction(tr("Open in MEGA"),
                                              QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    connect(showRemoteAction, &MenuItemAction::triggered,
            this, &SettingsDialog::showInMegaClicked);
    // Delete Sync action
    auto delAction (new MenuItemAction(tr("Remove synced folder"),
                                       QIcon(QString::fromUtf8("://images/ico_Delete.png"))));
    delAction->setAccent(true);
    connect(delAction, &MenuItemAction::triggered,
            this, &SettingsDialog::onDeleteSync, Qt::QueuedConnection);


    auto syncRun (new MenuItemAction(tr("Run"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    auto syncPause (new MenuItemAction(tr("Pause"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    auto syncSuspend (new MenuItemAction(tr("Suspend"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    auto syncDisable (new MenuItemAction(tr("Disable"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));

    connect(syncRun, &MenuItemAction::triggered, this, &SettingsDialog::setSyncToRun);
    connect(syncPause, &MenuItemAction::triggered, this, &SettingsDialog::setSyncToPause);
    connect(syncSuspend, &MenuItemAction::triggered, this, &SettingsDialog::setSyncToSuspend);
    connect(syncDisable, &MenuItemAction::triggered, this, &SettingsDialog::setSyncToDisabled);


    showLocalAction->setParent(menu);
    showRemoteAction->setParent(menu);
    delAction->setParent(menu);

    syncRun->setParent(menu);
    syncPause->setParent(menu);
    syncSuspend->setParent(menu);
    syncDisable->setParent(menu);

    menu->addAction(showLocalAction);
    menu->addAction(showRemoteAction);
    menu->addSeparator();
    menu->addAction(delAction);

    menu->addAction(syncRun);
    menu->addAction(syncPause);
    menu->addAction(syncSuspend);
    menu->addAction(syncDisable);

    QWidget* w (mUi->tSyncs->cellWidget(row, column));
    menu->popup(w->mapToGlobal(w->rect().center()));
}


void SettingsDialog::setSyncToRun()
{
    ActionProgress* action
        = new ActionProgress(true, QString::fromUtf8("Run sync"));

    connect(action, &ActionProgress::failedRequest,
        this, [this](MegaRequest* request, MegaError* error)
        {
            Q_UNUSED(request)
            if (error->getErrorCode())
                showUnexpectedSyncError(tr("Unexpected error running sync"));
        },
        Qt::DirectConnection);

    auto syncSetting = Model::instance()->getSyncSetting(mSelectedSyncRow);
    mController->setSyncRunState(mega::MegaSync::RUNSTATE_RUNNING, syncSetting, action);
}

void SettingsDialog::setSyncToPause()
{
    ActionProgress* action
        = new ActionProgress(true, QString::fromUtf8("Pause sync"));

    connect(action, &ActionProgress::failedRequest,
        this, [this](MegaRequest* request, MegaError* error)
        {
            Q_UNUSED(request)
            if (error->getErrorCode())
                showUnexpectedSyncError(tr("Unexpected error pausing sync"));
        }, Qt::DirectConnection);

    auto syncSetting = Model::instance()->getSyncSetting(mSelectedSyncRow);
    mController->setSyncRunState(mega::MegaSync::RUNSTATE_PAUSED, syncSetting, action);
}


void SettingsDialog::setSyncToSuspend()
{
    ActionProgress* action
        = new ActionProgress(true, QString::fromUtf8("Suspend sync"));

    connect(action, &ActionProgress::failedRequest,
        this, [this](MegaRequest* request, MegaError* error)
        {
            Q_UNUSED(request)
            if (error->getErrorCode())
                showUnexpectedSyncError(tr("Unexpected error suspending sync"));
        }, Qt::DirectConnection);

    auto syncSetting = Model::instance()->getSyncSetting(mSelectedSyncRow);
    mController->setSyncRunState(mega::MegaSync::RUNSTATE_SUSPENDED, syncSetting, action);
}


void SettingsDialog::setSyncToDisabled()
{
    ActionProgress* action
            = new ActionProgress(true, QString::fromUtf8("Disabling sync"));

    connect(action, &ActionProgress::failedRequest,
            this, [this](MegaRequest* request, MegaError* error)
    {
        Q_UNUSED(request)
        if (error->getErrorCode())
            showUnexpectedSyncError(tr("Unexpected error disabling sync"));
    }, Qt::DirectConnection);

    auto syncSetting = Model::instance()->getSyncSetting(mSelectedSyncRow);
    mController->setSyncRunState(mega::MegaSync::RUNSTATE_DISABLED, syncSetting, action);
}

void SettingsDialog::showInFolderClicked()
{
    QWidget* w (mUi->tSyncs->cellWidget(mSelectedSyncRow, SYNC_COL_DISPLAY_NAME));
    QString localFolderPath (qobject_cast<QSyncItemWidget*>(w)->getLocalPath());
    QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localFolderPath));
}

void SettingsDialog::showInMegaClicked()
{
    auto syncSetting = Model::instance()->getSyncSetting(mSelectedSyncRow);

    std::unique_ptr<char[]> np (MegaSyncApp->getMegaApi()->getNodePathByNodeHandle(
                                    syncSetting->getMegaHandle()));
    if (np)
    {
        MegaNode* node (mMegaApi->getNodeByPath(np.get()));
        if (node)
        {
            const char* handle = node->getBase64Handle();
            QString url = QString::fromUtf8("mega://#fm/") + QString::fromUtf8(handle);
            QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
            delete [] handle;
            delete node;
        }
    }
    Model::instance()->updateMegaFolder(np ? QString::fromUtf8(np.get())
                                           : QString(),
                                        syncSetting);
}

void SettingsDialog::onDeleteSync()
{
    ActionProgress* action
        = new ActionProgress(true, QString::fromUtf8("Remove sync"));

    connect(action, &ActionProgress::failedRequest,
        this, [this](MegaRequest* request, MegaError* error)
        {
            Q_UNUSED(request)
            if (error->getErrorCode())
                showUnexpectedSyncError(tr("Unexpected error removing sync"));
        }, Qt::DirectConnection);

    auto syncSetting = Model::instance()->getSyncSetting(mSelectedSyncRow);
    mController->removeSync(syncSetting, action);
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

    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    filePermissions = dialog->filePermissions();
    folderPermissions = dialog->folderPermissions();
    delete dialog;

    if (filePermissions != mPreferences->filePermissionsValue()
        || folderPermissions != mPreferences->folderPermissionsValue())
    {
        mPreferences->setFilePermissionsValue(filePermissions);
        mPreferences->setFolderPermissionsValue(folderPermissions);
    }
}
#endif

void SettingsDialog::syncsStateInformation(int state)
{
    // If saving syncs are still in progress, wait the timeout for setting state widget
    if (mIsSavingSyncsOnGoing)
    {
        return;
    }

    switch (state)
    {
        case SAVING_SYNCS:
            mUi->wSpinningIndicator->start();
            mUi->sSyncsState->setCurrentWidget(mUi->pSavingSyncs);
            break;
        default:
        {
            mUi->wSpinningIndicator->stop();
            // If any sync is disabled, shows warning message
            if (mAreSyncsDisabled)
            {
                mUi->sSyncsState->setCurrentWidget(mUi->pSyncsDisabled);

#ifdef Q_OS_MACOS
                QString syncs(QString::fromUtf8("settings-syncs-error"));
                mToolBar->customizeIconToolBarItem(bSyncs.get(), syncs);
#else
                mUi->bSyncs->setIcon(QIcon(QString::fromUtf8(":/images/settings-syncs-warn.png")));
#endif
            }
            else
            {
                mUi->sSyncsState->setCurrentWidget(mUi->pNoErrors);

#ifdef Q_OS_MACOS
                QString syncs(QString::fromUtf8("settings-syncs"));
                mToolBar->customizeIconToolBarItem(bSyncs.get(), syncs);
#else
                mUi->bSyncs->setIcon(QIcon(QString::fromUtf8(":/images/settings-syncs.png")));
#endif
            }
        }
            break;
    }
}

void SettingsDialog::addSyncRow(int row, const QString& name, const QString& lPath,
                                const QString& rPath, bool boxTicked, QString runStateString, int error,
                                MegaHandle megaHandle, MegaHandle tag,
                                std::shared_ptr<SyncSetting> syncSetting)
{
    // Col 0: Enabled/Disabled checkbox
    QWidget* w = new QWidget();
    QHBoxLayout* hl = new QHBoxLayout();
    //QCheckBox* c = new QCheckBox();

#ifdef Q_OS_MACOS
    //Set fixed size to avoid misplaced of checkbox for sync row items
    hl->setContentsMargins(0, 0, 11, 0);
    //c->setFixedSize(16, 16);
#else
    hl->setContentsMargins(0, 0, 0, 0);
#endif

    w->setLayout(hl);
    //hl->addWidget(c);
    //hl->setAlignment(c, Qt::AlignCenter);

    // Note: isEnabled refers to enable/disabled by the user. It could be temporary
    //       disabled or even failed. This should be shown in the UI.
    //c->setChecked(boxTicked);
    //c->setToolTip(tr("Enable / disable"));
    //connect(c, &QCheckBox::stateChanged,
    //        this, &SettingsDialog::syncStateChanged, Qt::QueuedConnection);

    mUi->tSyncs->setCellWidget(row, SYNC_COL_ENABLE_CB, w);

    // Col 1: Name (and paths.)
    {
        assert(!rPath.isEmpty() && "remote folder lacks path");

        auto widget = new QSyncItemWidget(QSyncItemWidget::NAME);

        // Remove namespace prefix if presnet.
        auto localPath = lPath;

#ifdef Q_OS_WINDOWS
        if (lPath.startsWith(QString::fromUtf8("\\\\?\\")))
            localPath.remove(0, 4);
#endif // Q_OS_WINDOWS

        widget->mSyncRootHandle = megaHandle;
        widget->setError(error);
        widget->setLocalPath(localPath);
        widget->setName(name);
        widget->setRemotePath(rPath);
        widget->setSyncSetting(syncSetting);
        widget->setToolTip(
            QString::fromUtf8("Local Path: ")
            + localPath
            + QString::fromUtf8("\r\nRemote Path: ")
            + rPath);

        mUi->tSyncs->setCellWidget(row, SYNC_COL_DISPLAY_NAME, widget);
    }

    // Col 2: Run State.
    {
        auto widget = new QSyncItemWidget(QSyncItemWidget::RUN_STATE);

        widget->setRunState(runStateString);
        widget->setSyncSetting(syncSetting);

        mUi->tSyncs->setCellWidget(row, SYNC_COL_RUN_STATE, widget);
    }

    // Col 3: menu
    QLabel* lMenu (new QLabel);
    QString menuRsc (QString::fromUtf8("://images/Item_options_rest.png"));
    lMenu->setFixedSize(16, 16);

    QWidget* menuWidget = new QWidget();
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(lMenu);
    menuWidget->setStyleSheet(SYNCS_TAB_MENU_LABEL_QSS.arg(menuRsc));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setAlignment(lMenu, Qt::AlignCenter);
    menuWidget->setLayout(horizontalLayout);

    mUi->tSyncs->setCellWidget(row, SYNC_COL_MENU, menuWidget);

    // Col 4: tag. HIDDEN
    if (tag != INVALID_HANDLE)
    {
        QLabel* lTag = new QLabel(QString::number(tag));
        mUi->tSyncs->setCellWidget(row, SYNC_COL_TAG, lTag);
    }

    // Col 5: MegaHandle. HIDDEN
    QLabel* lHandle = new QLabel(QString::number(megaHandle));
    mUi->tSyncs->setCellWidget(row, SYNC_COL_HANDLE, lHandle);

    // Col 6: SyncName. HIDDEN
    QLabel* lName = new QLabel(name);
    mUi->tSyncs->setCellWidget(row, SYNC_COL_NAME, lName);

    mSyncNames.append(name);
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
    emit closeMenus();
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
    QString fileName = QFileDialog::getSaveFileName(0, tr("Export Master key"),
                                                    dir.filePath(tr("MEGA-RECOVERYKEY")),
                                                    QString::fromUtf8("Txt file (*.txt)"), nullptr,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Truncate))
    {
        QMegaMessageBox::information(this, tr("Unable to write file"), file.errorString());
        return;
    }

    QTextStream out(&file);
    out << mMegaApi->exportMasterKey();

    file.close();

    mMegaApi->masterKeyExported();

    QMegaMessageBox::information(this, tr("Warning"),
                                 tr("Exporting the master key and keeping it in a secure location"
                                    " enables you to set a new password without data loss.")
                                 + QString::fromUtf8("\n")
                                 + tr("Always keep physical control of your master key (e.g. on a"
                                      " client device, external storage, or print)."),
                                 QMessageBox::Ok);
}

void SettingsDialog::on_bChangePassword_clicked()
{
    QPointer<ChangePassword> cPassword = new ChangePassword(this);
    int result = cPassword->exec();
    if (!cPassword || (result != QDialog::Accepted))
    {
        delete cPassword;
        return;
    }

    delete cPassword;
}

void SettingsDialog::on_bSessionHistory_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl,
                      QUrl(QString::fromUtf8("mega://#fm/account/history")));
}

// Folders -----------------------------------------------------------------------------------------
void SettingsDialog::updateUploadFolder()
{
    MegaNode* node (mMegaApi->getNodeByHandle(static_cast<uint64_t>(mPreferences->uploadFolder())));
    if (!node)
    {
        mHasDefaultUploadOption = false;
        mUi->eUploadFolder->setText(QString::fromUtf8("/MEGAsync Uploads"));
    }
    else
    {
        const char* nPath = mMegaApi->getNodePath(node);
        if (!nPath)
        {
            mHasDefaultUploadOption = false;
            mUi->eUploadFolder->setText(QString::fromUtf8("/MEGAsync Uploads"));
        }
        else
        {
            mHasDefaultUploadOption = mPreferences->hasDefaultUploadFolder();
            mUi->eUploadFolder->setText(QString::fromUtf8(nPath));
            delete [] nPath;
        }
        delete node;
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
    emit closeMenus();
    mUi->pFolders->hide();
    animateSettingPage(SETTING_ANIMATION_FOLDERS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bUploadFolder_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(NodeSelector::UPLOAD_SELECT, this);
    MegaNode* defaultNode = mMegaApi->getNodeByPath(mUi->eUploadFolder->text()
                                                    .toUtf8().constData());
    if (defaultNode)
    {
        nodeSelector->setSelectedNodeHandle(defaultNode->getHandle());
        delete defaultNode;
    }

    nodeSelector->setDefaultUploadOption(mHasDefaultUploadOption);
    nodeSelector->showDefaultUploadOption();
    int result = nodeSelector->exec();
    if (!nodeSelector || (result != QDialog::Accepted))
    {
        delete nodeSelector;
        return;
    }

    MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedNodeHandle();
    MegaNode* node = mMegaApi->getNodeByHandle(selectedMegaFolderHandle);
    if (!node)
    {
        delete nodeSelector;
        return;
    }

    const char* nPath = mMegaApi->getNodePath(node);
    if (!nPath || !std::strlen(nPath))
    {
        delete nodeSelector;
        delete node;
        return;
    }

    mHasDefaultUploadOption = nodeSelector->getDefaultUploadOption();
    mUi->eUploadFolder->setText(QString::fromUtf8(nPath));
    mPreferences->setHasDefaultUploadFolder(mHasDefaultUploadOption);
    mPreferences->setUploadFolder(static_cast<long long>(node->getHandle()));

    delete nodeSelector;
    delete [] nPath;
    delete node;
}

void SettingsDialog::on_bDownloadFolder_clicked()
{
    QPointer<DownloadFromMegaDialog> dialog = new DownloadFromMegaDialog(
                                                  mPreferences->downloadFolder(), this);
    dialog->setDefaultDownloadOption(mHasDefaultDownloadOption);

    int result = dialog->exec();
    if (!dialog || (result != QDialog::Accepted))
    {
        delete dialog;
        return;
    }

    QString fPath = dialog->getPath();
    if (!fPath.isEmpty())
    {
        QTemporaryFile test(fPath + QDir::separator());
        if (!test.open())
        {
            QMegaMessageBox::critical(nullptr, tr("Error"), tr("You don't have write permissions"
                                                               " in this local folder."));
            delete dialog;
            return;
        }

        mHasDefaultDownloadOption = dialog->isDefaultDownloadOption();
        mUi->eDownloadFolder->setText(fPath);
        mPreferences->setDownloadFolder(fPath);
        mPreferences->setHasDefaultDownloadFolder(mHasDefaultDownloadOption);
    }

    delete dialog;
}

//void SettingsDialog::on_bAddName_clicked()
//{
//    QPointer<AddExclusionDialog> add = new AddExclusionDialog(this);
//    int result = add->exec();
//    if (!add || (result != QDialog::Accepted))
//    {
//        delete add;
//        return;
//    }
//
//    QString text = add->textValue();
//    delete add;
//
//    if (text.isEmpty())
//    {
//        return;
//    }
//
//    for (int i = 0; i < mUi->lExcludedNames->count(); i++)
//    {
//        if (mUi->lExcludedNames->item(i)->text() == text)
//        {
//            return;
//        }
//        else if (mUi->lExcludedNames->item(i)->text().compare(text, Qt::CaseInsensitive) > 0)
//        {
//            mUi->lExcludedNames->insertItem(i, text);
//            saveExcludeSyncNames();
//            return;
//        }
//    }
//
//    mUi->lExcludedNames->addItem(text);
//    saveExcludeSyncNames();
//}
//
//void SettingsDialog::on_bDeleteName_clicked()
//{
//    QList<QListWidgetItem*> selected = mUi->lExcludedNames->selectedItems();
//    if (selected.size() == 0)
//    {
//        return;
//    }
//
//    for (int i = 0; i < selected.size(); i++)
//    {
//        delete selected[i];
//    }
//
//    saveExcludeSyncNames();
//}
//
//void SettingsDialog::on_cExcludeUpperThan_clicked()
//{
//    if (mLoadingSettings) return;
//    bool enable (mUi->cExcludeUpperThan->isChecked());
//    mPreferences->setUpperSizeLimit(enable);
//    mPreferences->setCrashed(true);
//    mUi->eUpperThan->setEnabled(enable);
//    mUi->cbExcludeUpperUnit->setEnabled(enable);
//    mUi->gExcludedFilesInfo->show();
//    mUi->bRestart->show();
//}
//
//void SettingsDialog::on_cExcludeLowerThan_clicked()
//{
//    if (mLoadingSettings) return;
//    bool enable (mUi->cExcludeLowerThan->isChecked());
//    mPreferences->setLowerSizeLimit(enable);
//    mPreferences->setCrashed(true);
//    mUi->eLowerThan->setEnabled(enable);
//    mUi->cbExcludeLowerUnit->setEnabled(enable);
//    mUi->gExcludedFilesInfo->show();
//    mUi->bRestart->show();
//}
//
//void SettingsDialog::on_eUpperThan_valueChanged(int i)
//{
//    if (mLoadingSettings) return;
//    mPreferences->setUpperSizeLimitValue(i);
//    mPreferences->setCrashed(true);
//    mUi->gExcludedFilesInfo->show();
//    mUi->bRestart->show();
//}
//
//void SettingsDialog::on_eLowerThan_valueChanged(int i)
//{
//    if (mLoadingSettings) return;
//    mPreferences->setLowerSizeLimitValue(i);
//    mPreferences->setCrashed(true);
//    mUi->gExcludedFilesInfo->show();
//    mUi->bRestart->show();
//}
//
//void SettingsDialog::on_cbExcludeUpperUnit_currentIndexChanged(int index)
//{
//    if (mLoadingSettings) return;
//    mPreferences->setUpperSizeLimitUnit(index);
//    mPreferences->setCrashed(true);
//    mUi->gExcludedFilesInfo->show();
//    mUi->bRestart->show();
//}
//
//void SettingsDialog::on_cbExcludeLowerUnit_currentIndexChanged(int index)
//{
//    if (mLoadingSettings) return;
//    mPreferences->setLowerSizeLimitUnit(index);
//    mPreferences->setCrashed(true);
//    mUi->gExcludedFilesInfo->show();
//    mUi->bRestart->show();
//}

//void SettingsDialog::saveExcludeSyncNames()
//{
//    QStringList excludedNames;
//    QStringList excludedPaths;
//    for (int i = 0; i < mUi->lExcludedNames->count(); i++)
//    {
//        if (mUi->lExcludedNames->item(i)->text().contains(QDir::separator())) // Path exclusion
//        {
//            excludedPaths.append(mUi->lExcludedNames->item(i)->text());
//        }
//        else // File name exclusion
//        {
//            excludedNames.append(mUi->lExcludedNames->item(i)->text());
//        }
//    }
//
//    mPreferences->setExcludedSyncNames(excludedNames);
//    mPreferences->setExcludedSyncPaths(excludedPaths);
//    mPreferences->setCrashed(true);
//
//    mUi->gExcludedFilesInfo->show();
//    mUi->bRestart->show();
//}

void SettingsDialog::restartApp()
{
    // Restart MEGAsync
#if defined(Q_OS_MACX)
    mApp->rebootApplication(false);
#else
    //we enqueue this call, so as not to close before properly
    // handling the exit of Settings Dialog
    QTimer::singleShot(0, [] () {MegaSyncApp->rebootApplication(false);});
#endif
}

//void SettingsDialog::on_bRestart_clicked()
//{
//    QPointer<SettingsDialog> currentDialog = this;
//    if (QMegaMessageBox::warning(nullptr, tr("Restart MEGAsync"),
//                                 tr("Do you want to restart MEGAsync now?"),
//                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
//            == QMessageBox::Yes)
//    {
//        if (currentDialog)
//        {
//            restartApp();
//        }
//    }
//}

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
    emit closeMenus();
    mUi->pNetwork->hide();
    animateSettingPage(SETTING_ANIMATION_NETWORK_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bOpenProxySettings_clicked()
{
    ProxySettings* proxySettingsDialog = new ProxySettings(mApp, this);
    if (proxySettingsDialog->exec() == QDialog::Accepted)
    {
        mApp->applyProxySettings();
        updateNetworkTab();
    }
}

void SettingsDialog::on_bOpenBandwidthSettings_clicked()
{
    BandwidthSettings* bandwidthSettings = new BandwidthSettings(mApp, this);
    if (bandwidthSettings->exec() == QDialog::Rejected)
    {
        return;
    }

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

void SettingsDialog::on_bNotifications_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pNotifications)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pNotifications);

#ifdef Q_OS_MACOS
    emit closeMenus();
    mUi->pNotifications->hide();
    animateSettingPage(SETTING_ANIMATION_NOTIFICATIONS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::updateNetworkTab()
{
    int uploadLimitKB = mPreferences->uploadLimitKB();
    if (uploadLimitKB < 0)
    {
        mUi->lUploadRateLimit->setText(tr("Auto"));
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
            mUi->lProxySettings->setText(tr("Auto"));
            break;
        case Preferences::PROXY_TYPE_CUSTOM:
            mUi->lProxySettings->setText(tr("Manual"));
            break;
    }
}

void SettingsDialog::showUnexpectedSyncError(const QString& message)
{
    QObject temporary;

    auto completion = [message]() {
        QMegaMessageBox::critical(nullptr, tr("Error"), message);
    };

    QObject::connect(&temporary,
                     &QObject::destroyed,
                     this,
                     std::move(completion),
                     Qt::QueuedConnection);
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
