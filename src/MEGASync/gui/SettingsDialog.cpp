#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>
#include <QTranslator>
#include <QMessageBox>
#include <QButtonGroup>
#include <QtConcurrent/QtConcurrent>

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

#ifdef Q_OS_MACOS
//Const values used for macOS Settings dialog resize animation
constexpr auto SETTING_ANIMATION_PAGE_TIMEOUT{150};//ms
constexpr auto SETTING_ANIMATION_GENERAL_TAB_HEIGHT{590};
constexpr auto SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT{466};//px height
constexpr auto SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT_BUSINESS{446};
constexpr auto SETTING_ANIMATION_SYNCS_TAB_HEIGHT{344};
constexpr auto SETTING_ANIMATION_IMPORTS_TAB_HEIGHT{513};
// FIXME: Re-evaluate sizes for Network tab
constexpr auto SETTING_ANIMATION_NETWORK_TAB_HEIGHT{190};
constexpr auto SETTING_ANIMATION_SECURITY_TAB_HEIGHT{400};
#endif

long long calculateCacheSize()
{
    Model* model = Model::instance();
    long long cacheSize = 0;
    for (int i = 0; i < model->getNumSyncedFolders(); i++)
    {
        auto syncSetting = model->getSyncSetting(i);
        QString syncPath = syncSetting->getLocalFolder();
        if (!syncPath.isEmpty())
        {
            Utilities::getFolderSize(syncPath + QDir::separator()
                                     + QString::fromUtf8(MEGA_DEBRIS_FOLDER), &cacheSize);
        }
    }
    return cacheSize;
}

void deleteCache()
{
    static_cast<MegaApplication*>(qApp)->cleanLocalCaches(true);
}

long long calculateRemoteCacheSize(MegaApi* megaApi)
{
    MegaNode* n = megaApi->getNodeByPath("//bin/SyncDebris");
    long long toret = megaApi->getSize(n);
    delete n;
    return toret;
}

void deleteRemoteCache(MegaApi* megaApi)
{
    MegaNode* n = megaApi->getNodeByPath("//bin/SyncDebris");
    megaApi->remove(n);
    delete n;
}

#ifdef Q_OS_MACOS
void SettingsDialog::initializeNativeUIComponents()
{
    CocoaHelpButton *helpButton = new CocoaHelpButton(this);
    ui->layoutBottom->insertWidget(0, helpButton);
    connect(helpButton, SIGNAL(clicked()), this, SLOT(on_bHelp_clicked()));   

    // Set native NSToolBar for settings.
    toolBar = ::mega::make_unique<QCustomMacToolbar>(this);

    QString general(QString::fromUtf8("settings-general"));
    QString account(QString::fromUtf8("settings-account"));
    QString syncs(QString::fromUtf8("settings-syncs"));
    QString security(QString::fromUtf8("settings-security"));
    QString imports(QString::fromUtf8("settings-imports"));
    QString network(QString::fromUtf8("settings-network"));

    // add Items
    bGeneral.reset(toolBar->addItem(QIcon(), tr("General")));
    toolBar->customizeIconToolBarItem(bGeneral.get(), general);
    connect(bGeneral.get(), &QMacToolBarItem::activated, this, &SettingsDialog::on_bGeneral_clicked);

    bAccount.reset(toolBar->addItem(QIcon(), tr("Account")));
    toolBar->customizeIconToolBarItem(bAccount.get(), account);
    connect(bAccount.get(), &QMacToolBarItem::activated, this, &SettingsDialog::on_bAccount_clicked);

    bSyncs.reset(toolBar->addItem(QIcon(), tr("Syncs")));
    toolBar->customizeIconToolBarItem(bSyncs.get(), syncs);
    connect(bSyncs.get(), &QMacToolBarItem::activated, this, &SettingsDialog::on_bSyncs_clicked);

    bSecurity.reset(toolBar->addItem(QIcon(), tr("Security")));
    toolBar->customizeIconToolBarItem(bSecurity.get(), security);
    connect(bSecurity.get(), &QMacToolBarItem::activated, this, &SettingsDialog::on_bSecurity_clicked);

    bImports.reset(toolBar->addItem(QIcon(), tr("Imports")));
    toolBar->customizeIconToolBarItem(bImports.get(), imports);
    connect(bImports.get(), &QMacToolBarItem::activated, this, &SettingsDialog::on_bImports_clicked);

    bNetwork.reset(toolBar->addItem(QIcon(), tr("Network")));
    toolBar->customizeIconToolBarItem(bNetwork.get(), network);
    connect(bNetwork.get(), &QMacToolBarItem::activated, this, &SettingsDialog::on_bNetwork_clicked);    

    toolBar->setSelectableItems(true);
    toolBar->setAllowsUserCustomization(false);
    toolBar->setSelectedItem(bGeneral.get());

    // Attach to the window according Qt docs
    this->window()->winId(); // create window->windowhandle()
    toolBar->attachToWindowWithStyle(window()->windowHandle(), QCustomMacToolbar::StylePreference);

    //Configure segmented control for +/- syncs
    ui->wSyncsSegmentedControl->configureTableSegment();
    connect(ui->wSyncsSegmentedControl, &QSegmentedControl::addButtonClicked, this, &SettingsDialog::on_bAdd_clicked);
    connect(ui->wSyncsSegmentedControl, &QSegmentedControl::removeButtonClicked, this, &SettingsDialog::on_bDelete_clicked);

    ui->wExclusionsSegmentedControl->configureTableSegment();
    connect(ui->wExclusionsSegmentedControl, &QSegmentedControl::addButtonClicked, this, &SettingsDialog::on_bAddName_clicked);
    connect(ui->wExclusionsSegmentedControl, &QSegmentedControl::removeButtonClicked, this, &SettingsDialog::on_bDeleteName_clicked);
}
#endif

SettingsDialog::SettingsDialog(MegaApplication* app, bool proxyOnly, QWidget* parent) :
    QDialog (parent),
    mUi (new Ui::SettingsDialog),
    mApp (app),
    mPreferences (Preferences::instance()),
    mController (Controller::instance()),
    mModel (Model::instance()),
    mMegaApi (app->getMegaApi()),
    mAccountDetailsDialog (nullptr),
    mLoadingSettings (0),
    mCacheSize (-1),
    mRemoteCacheSize (-1),
    mReloadUIpage (false),
    mThreadPool (ThreadPoolSingleton::getInstance()),
    mAreSyncsDisabled (false),
    mIsSavingSyncsOnGoing (false),
    mDebugCounter (0)
{
    mUi->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(mUi->wStack, &QStackedWidget::currentChanged,
            mUi->wStackFooter, &QStackedWidget::setCurrentIndex);
    mUi->wStack->setCurrentWidget(mUi->pGeneral); // override whatever might be set in .ui

#ifndef Q_OS_MAC
    mUi->bGeneral->setChecked(true); // override whatever might be set in .ui
    mUi->gCache->setTitle(mUi->gCache->title().arg(QString::fromUtf8(MEGA_DEBRIS_FOLDER)));
#endif

    setProxyOnly(proxyOnly);

#ifdef Q_OS_LINUX
    mUi->wUpdateSection->hide();
#endif

    mUi->gExcludedFilesInfo->hide();

#ifdef Q_OS_WINDOWS
    ui->cFinderIcons->hide();

    typedef LONG MEGANTSTATUS;
    typedef struct _MEGAOSVERSIONINFOW {
        DWORD dwOSVersionInfoSize;
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;
        DWORD dwBuildNumber;
        DWORD dwPlatformId;
        WCHAR  szCSDVersion[ 128 ];     // Maintenance string for PSS usage
    } MEGARTL_OSVERSIONINFOW, *PMEGARTL_OSVERSIONINFOW;

    typedef MEGANTSTATUS (WINAPI* RtlGetVersionPtr)(PMEGARTL_OSVERSIONINFOW);
    MEGARTL_OSVERSIONINFOW version = { 0 };
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod)
    {
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (RtlGetVersion)
        {
            RtlGetVersion(&version);
            if (version.dwMajorVersion >= 10)
            {
                ui->cFinderIcons->show();
            }
        }
    }
#endif

#ifdef Q_OS_MACOS
    this->setWindowTitle(tr("Preferences - MEGAsync"));
    ui->tSyncs->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->cStartOnStartup->setText(tr("Open at login"));
    ui->lLocalDebris->setText(ui->lLocalDebris->text().arg(QString::fromAscii(MEGA_DEBRIS_FOLDER)));

    if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_9) //FinderSync API support from 10.10+
    {
        ui->cOverlayIcons->hide();
    }

    initializeNativeUIComponents();
#endif

    setProxyOnly(proxyOnly);

#ifdef Q_OS_MACOS
    minHeightAnimation = new QPropertyAnimation();
    maxHeightAnimation = new QPropertyAnimation();
    animationGroup = new QParallelAnimationGroup();
    animationGroup->addAnimation(minHeightAnimation);
    animationGroup->addAnimation(maxHeightAnimation);
    connect(animationGroup, SIGNAL(finished()), this, SLOT(onAnimationFinished()));

    ui->pSyncs->hide();

    if (!proxyOnly)
    {
        setMinimumHeight(SETTING_ANIMATION_GENERAL_TAB_HEIGHT);
        setMaximumHeight(SETTING_ANIMATION_GENERAL_TAB_HEIGHT);
        ui->pNetwork->hide();
    }
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

    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
}

SettingsDialog::~SettingsDialog()
{
    mApp->dettachStorageObserver(*this);
    mApp->dettachBandwidthObserver(*this);
    mApp->dettachAccountObserver(*this);

    delete mUi;
}

void SettingsDialog::setProxyOnly(bool proxyOnly)
{
    mProxyOnly = proxyOnly;

#ifndef Q_OS_MACOS
    mUi->bGeneral->setEnabled(!proxyOnly);
    mUi->bAccount->setEnabled(!proxyOnly);
    mUi->bSyncs->setEnabled(!proxyOnly);
    mUi->bSecurity->setEnabled(!proxyOnly);
    mUi->bImports->setEnabled(!proxyOnly);
#endif

    if (proxyOnly)
    {
#ifdef Q_OS_MACOS
        // TODO: Re-evaluate sizes for Network tab
        setMinimumHeight(435);
        setMaximumHeight(435);
        toolBar->setSelectedItem(bNetwork.get());
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

void SettingsDialog::setOverQuotaMode(bool mode)
{
    if (mode)
    {
        QString url = QString::fromUtf8("mega://#pro");
        Utilities::getPROurlWithParameters(url);
        mUi->lOQWarning->setText(tr("Your MEGA account is full. All uploads are disabled,"
                                    " which may affect your synced folders. [A]Buy more space[/A]")
                                .replace(QString::fromUtf8("[A]"),
                                         QString::fromUtf8("<a href=\"%1\">"
                                                           "<span style=\"color:#d90007;"
                                                           "text-decoration:none;\">").arg(url))
                                .replace(QString::fromUtf8("[/A]"),
                                         QString::fromUtf8("</span></a>")));
        mUi->wOQError->show();
    }
    else
    {
        mUi->lOQWarning->setText(QString());
        mUi->wOQError->hide();
    }

    return;
}

void SettingsDialog::syncStateChanged(int state)
{
    if (state)
    {
        Platform::prepareForSync();
    }

    saveSyncSettings();
}

void SettingsDialog::onDisableSyncFailed(std::shared_ptr<SyncSetting> syncSetting)
{
    QMegaMessageBox::critical(nullptr, tr("Error"),
                              tr("Unexpected error disabling sync %1").arg(syncSetting->name()));
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

// errorCode referrs to the requestErrorCode.
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

void SettingsDialog::storageChanged()
{
    onCacheSizeAvailable();
}

void SettingsDialog::onSyncStateChanged(std::shared_ptr<SyncSetting>)
{
    loadSyncSettings();
}

void SettingsDialog::onSavingSettingsProgress(double progress)
{
    Q_UNUSED(progress)
    syncsStateInformation(SyncStateInformation::SAVING_SYNCS);
    savingSyncs(false, mUi->pSyncs);
    mIsSavingSyncsOnGoing = true;
}

void SettingsDialog::onSavingSettingsCompleted()
{
    auto closeDelay = std::max(0ll, 350ll - (QDateTime::currentMSecsSinceEpoch()
                                             - mUi->wSpinningIndicator->getStartTime()));
    QTimer::singleShot(closeDelay, this, [this] () {
        mIsSavingSyncsOnGoing = false;
        syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
        savingSyncs(true, mUi->pSyncs);
    });
}

void SettingsDialog::storageStateChanged(int newStorageState)
{
     setOverQuotaMode(newStorageState == MegaApi::STORAGE_STATE_RED
                      || newStorageState == MegaApi::STORAGE_STATE_PAYWALL);
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

void SettingsDialog::on_bGeneral_clicked()
{
    emit userActivity();

    if ((mUi->wStack->currentWidget() == mUi->pGeneral) && !mReloadUIpage)
    {
        return;
    }

    mReloadUIpage = false;

    mUi->wStack->setCurrentWidget(mUi->pGeneral);

#ifdef Q_OS_MACOS
    onCacheSizeAvailable();

    ui->pGeneral->hide();
    animateSettingPage(SETTING_ANIMATION_GENERAL_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bAccount_clicked()
{
    emit userActivity();

    if ((mUi->wStack->currentWidget() == mUi->pAccount) && !mReloadUIpage)
    {
        return;
    }

    mReloadUIpage = false;

    mUi->wStack->setCurrentWidget(mUi->pAccount);

#ifdef Q_OS_MACOS

    ui->pAccount->hide();
    if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->gStorageSpace->setMinimumHeight(83);
        animateSettingPage(SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT_BUSINESS, SETTING_ANIMATION_PAGE_TIMEOUT);
    }
    else
    {
        ui->gStorageSpace->setMinimumHeight(103);
        animateSettingPage(SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
    }

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
    mUi->tSyncs->horizontalHeader()->setVisible(true);

#ifdef Q_OS_MACOS
    ui->pSyncs->hide();
    animateSettingPage(SETTING_ANIMATION_SYNCS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bSecurity_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pSecurity)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pSecurity);

#ifdef Q_OS_MACOS
    ui->pSecurity->hide();
    animateSettingPage(SETTING_ANIMATION_SECURITY_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bImports_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pImports)
    {
        return;
    }
    mUi->wStack->setCurrentWidget(mUi->pImports);

#ifdef Q_OS_MACOS
    ui->pImports->hide();
    animateSettingPage(SETTING_ANIMATION_IMPORTS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bNetwork_clicked()
{
    emit userActivity();

    if (mUi->wStack->currentWidget() == mUi->pNetwork)
    {
        return;
    }

    mUi->wStack->setCurrentWidget(mUi->pNetwork);

#ifdef Q_OS_MACOS
    ui->pNetwork->hide();
    animateSettingPage(SETTING_ANIMATION_NETWORK_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bHelp_clicked()
{
    QString helpUrl = Preferences::BASE_URL + QString::fromUtf8("/help/client/megasync");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(helpUrl));
}

void SettingsDialog::on_bUpgrade_clicked()
{
    QString url = QString::fromUtf8("mega://#pro");
    Utilities::getPROurlWithParameters(url);
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
}

void SettingsDialog::on_bMyAccount_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#fm/account")));
}

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

    mUi->cShowNotifications->setChecked(mPreferences->showNotifications());

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
    mUi->cStartOnStartup->setChecked(mPreferences->startOnStartup() && Platform::isStartOnStartupActive());

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

    // account type and details
    updateAccountElements();
    updateStorageElements();
    updateBandwidthElements();

    if (mAccountDetailsDialog)
    {
        mAccountDetailsDialog->refresh(mPreferences);
    }

    updateUploadFolder();
    updateDownloadFolder();

    //Syncs
    loadSyncSettings();

#ifdef Q_OS_WINDOWS
    ui->cFinderIcons->setChecked(!preferences->leftPaneIconsDisabled());
#endif

    updateNetworkTab();

    // Imports tab
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
        cb->addItem(tr("Bytes"));
        cb->addItem(tr("KB"));
        cb->addItem(tr("MB"));
        cb->addItem(tr("GB"));
    }

    bool upperSizeLimit (mPreferences->upperSizeLimit());
    mUi->eLowerThan->setMaximum(9999);
    mUi->cExcludeUpperThan->setChecked(upperSizeLimit);
    mUi->eUpperThan->setEnabled(upperSizeLimit);
    mUi->cbExcludeUpperUnit->setEnabled(upperSizeLimit);
    mUi->eUpperThan->setValue(static_cast<int>(mPreferences->upperSizeLimitValue()));
    mUi->cbExcludeUpperUnit->setCurrentIndex(mPreferences->upperSizeLimitUnit());

    bool lowerSizeLimit (mPreferences->lowerSizeLimit());
    mUi->eUpperThan->setMaximum(9999);
    mUi->cExcludeLowerThan->setChecked(lowerSizeLimit);
    mUi->eLowerThan->setEnabled(lowerSizeLimit);
    mUi->cbExcludeLowerUnit->setEnabled(lowerSizeLimit);
    mUi->eLowerThan->setValue(static_cast<int>(mPreferences->lowerSizeLimitValue()));
    mUi->cbExcludeLowerUnit->setCurrentIndex(mPreferences->lowerSizeLimitUnit());

    mLoadingSettings--;
}

void SettingsDialog::saveSyncSettings()
{
    mSaveSettingsProgress.reset(new ProgressHelper(false, tr("Saving Sync settings")));
    connect(mSaveSettingsProgress.get(), &ProgressHelper::progress,
            this, &SettingsDialog::onSavingSettingsProgress);
    connect(mSaveSettingsProgress.get(), &ProgressHelper::completed,
            this, &SettingsDialog::onSavingSettingsCompleted);

    // Uncomment the following to see a progress bar when saving
    // (which being modal will prevent from modifying while changing)
    //Utilities::showProgressDialog(saveSettingsProgress.get(), this);

    ProgressHelperCompletionGuard g(mSaveSettingsProgress.get());

    if (!mProxyOnly)
    {
        onSavingSettingsProgress(0);

        // 1 - loop through the syncs in the model to remove or update
        for (int i = 0; i < mModel->getNumSyncedFolders(); i++)
        {
            auto syncSetting = mModel->getSyncSetting(i);
            if (!syncSetting)
            {
                assert("missing setting when looping for saving");
                continue;
            }

            // 1.1 - remove no longer present:
            bool found = false;
            for (int j = 0; j < mUi->tSyncs->rowCount(); j++)
            {
                auto tagItem = mUi->tSyncs->cellWidget(j,3);
                if (tagItem && static_cast<QLabel *>(tagItem)->text().toULongLong() == syncSetting->backupId())
                {
                    found = true;
                    break;
                }
            }
            if (!found) //sync no longer found in settings: needs removing
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing sync: %1").arg(syncSetting->name()).toUtf8().constData());
                ActionProgress *removeSyncStep = new ActionProgress(true, QString::fromUtf8("Removing sync: %1 - %2")
                                                                    .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()));
                mSaveSettingsProgress->addStep(removeSyncStep);
                mController->removeSync(syncSetting, removeSyncStep);
            }

            // 1.2 - enable/disable changed syncs
            for (int j = 0; j < mUi->tSyncs->rowCount(); j++)
            {
                bool enabled = ((QCheckBox *)mUi->tSyncs->cellWidget(j, 2))->isChecked();
                bool disabled = !enabled;

                auto tagItem = mUi->tSyncs->cellWidget(j,3);

                if (tagItem && static_cast<QLabel *>(tagItem)->text().toULongLong() == syncSetting->backupId())
                {
                    if (disabled && syncSetting->isActive()) //sync disabled
                    {
                        ActionProgress *disableSyncStep = new ActionProgress(true, QString::fromUtf8("Disabling sync: %1 - %2")
                                                                             .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()));
                        mSaveSettingsProgress->addStep(disableSyncStep);

                        connect(disableSyncStep, &ActionProgress::failedRequest, this, [this, syncSetting](MegaRequest *request, MegaError *error)
                        {
                            Q_UNUSED(request)
                            if (error->getErrorCode())
                            {
                                QObject temporary;
                                QObject::connect(&temporary, &QObject::destroyed, this, [this, syncSetting](){
                                    onDisableSyncFailed(syncSetting); //Note: this might get executed before onSyncStateChanged!
                                    loadSyncSettings(); //to no longer show the failed line
                                }, Qt::QueuedConnection);
                            }
                        }, Qt::DirectConnection); //Note, we need direct connection to use request & error

                        mController->disableSync(syncSetting, disableSyncStep);
                    }
                    else if (enabled && !syncSetting->isActive()) //sync re-enabled!
                    {
                        ActionProgress *enableSyncStep = new ActionProgress(true, QString::fromUtf8("Enabling sync: %1 - %2")
                                                                            .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()));
                        mSaveSettingsProgress->addStep(enableSyncStep);
                        connect(enableSyncStep, &ActionProgress::failedRequest, this, [this, syncSetting](MegaRequest *request, MegaError *error)
                        {
                            if (error->getErrorCode())
                            {
                                auto syncError = request->getNumDetails();
                                QObject temporary;
                                QObject::connect(&temporary, &QObject::destroyed, this, [this, syncError, syncSetting](){
                                    onEnableSyncFailed(syncError, syncSetting); //Note: this might get executed before onSyncStateChanged!
                                    //(syncSettings might have some old values), that's why we don't use syncSetting->getError.

                                    loadSyncSettings(); //to no longer show the failed line

                                }, Qt::QueuedConnection);
                            }
                        }, Qt::DirectConnection); //Note, we need direct connection to use request & error

                        mController->enableSync(syncSetting, enableSyncStep);
                    }
                    break;
                }
                else
                {
                    assert("paths changed for an already configured sync");
                }
            }
        }

        // 2 - look for new syncs
        for (int j = 0; j < mUi->tSyncs->rowCount(); j++)
            {
                auto tagItem = mUi->tSyncs->cellWidget(j, 3);
                if (!tagItem) //not found: new sync
                {
                    bool enabled = ((QCheckBox *)mUi->tSyncs->cellWidget(j, 2))->isChecked();
                    if (enabled)
                    {
                        QString localFolderPath = static_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(j, 0))->fullPath();
                        QString megaFolderPath = static_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(j, 1))->fullPath();
                        MegaHandle nodeHandle = static_cast<QLabel *>(mUi->tSyncs->cellWidget(j, 4))->text().toULongLong();
                        QString syncName =static_cast<QLabel *>(mUi->tSyncs->cellWidget(j, 5))->text();

                        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Adding sync from Settings: %1 - %2")
                                     .arg(localFolderPath).arg(megaFolderPath).toUtf8().constData());


                        ActionProgress *addSyncStep = new ActionProgress(true, QString::fromUtf8("Adding sync: %1 - %2")
                                                                         .arg(localFolderPath).arg(megaFolderPath));
                        mSaveSettingsProgress->addStep(addSyncStep);

                        //Connect failing signals
                        connect(addSyncStep, &ActionProgress::failed, this, [this, localFolderPath, megaFolderPath](int errorCode)
                        {
                            mApp->showAddSyncError(errorCode, localFolderPath, megaFolderPath);
                            loadSyncSettings(); //to no longer show the failed line
                        }, Qt::QueuedConnection);

                        connect(addSyncStep, &ActionProgress::failedRequest, this, [this, localFolderPath, megaFolderPath](MegaRequest *request, MegaError *error)
                        {
                            if (error->getErrorCode())
                            {
                                auto reqCopy = request->copy();
                                auto errCopy = error->copy();

                                QObject temporary;
                                QObject::connect(&temporary, &QObject::destroyed, this, [this, reqCopy, errCopy, localFolderPath, megaFolderPath](){

                                    // we might want to handle this separately (i.e: indicate errors in SyncSettings engine)
                                    mApp->showAddSyncError(reqCopy, errCopy, localFolderPath, megaFolderPath);
                                    loadSyncSettings();  //to no longer show the failed line

                                    delete reqCopy;
                                    delete errCopy;
                                    //(syncSettings might have some old values), that's why we don't use syncSetting->getError.
                                }, Qt::QueuedConnection);
                            }
                        }, Qt::DirectConnection); //Note, we need direct connection to use request & error

                        mController->addSync(localFolderPath, nodeHandle, syncName, addSyncStep);
                    }
                    else
                    {
                        assert("adding a disabled sync is not allowed");
                    }

                    continue;
                }
            }
    }
}

void SettingsDialog::on_bDelete_clicked()
{
    QList<QTableWidgetSelectionRange> selected = mUi->tSyncs->selectedRanges();
    if (selected.size() == 0)
    {
        return;
    }

    int index = selected.first().topRow();
    mUi->tSyncs->removeRow(index);
    mSyncNames.removeAt(index);

    saveSyncSettings();
}

void SettingsDialog::loadSyncSettings()
{
    mUi->tSyncs->clearContents();
    mSyncNames.clear();

    mUi->tSyncs->horizontalHeader()->setVisible(true);
    int numFolders = mModel->getNumSyncedFolders();
    mUi->tSyncs->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    mUi->tSyncs->setRowCount(numFolders);
    mUi->tSyncs->setColumnCount(6);
    mUi->tSyncs->setColumnWidth(2, 21);
    mUi->tSyncs->setColumnHidden(3, true); //hidden tag
    mUi->tSyncs->setColumnHidden(4, true); //hidden handle
    mUi->tSyncs->setColumnHidden(5, true); //hidden name

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

        QSyncItemWidget *localFolder = new QSyncItemWidget(QSyncItemWidget::LOCAL_FOLDER);
        QString localFolderQString = syncSetting->getLocalFolder();
#ifdef Q_OS_WINDOWS
if (localFolderQString.startsWith(QString::fromAscii("\\\\?\\")))
{
    localFolderQString = localFolderQString.mid(4);
}
#endif
        // Check if current sync is disabled by an error.
        mAreSyncsDisabled = mAreSyncsDisabled || static_cast<bool>(syncSetting->getError());

        // Col 1: Local folder
        localFolder->setPath(localFolderQString, syncSetting->name());
        localFolder->setToolTip(localFolderQString);
        localFolder->setError(syncSetting->getError());
        mUi->tSyncs->setCellWidget(i, 0, localFolder);

        // Col 2: Mega Folder
        QSyncItemWidget *megaFolder = new QSyncItemWidget(QSyncItemWidget::REMOTE_FOLDER);
        assert(syncSetting->getMegaFolder().size() && "remote folder lacks path");
        megaFolder->setPath(syncSetting->getMegaFolder().size()?syncSetting->getMegaFolder():QString::fromUtf8("---"));
        megaFolder->setToolTip(syncSetting->getMegaFolder());
        megaFolder->setSyncSetting(syncSetting);
        megaFolder->mSyncRootHandle = syncSetting->getMegaHandle();
        mUi->tSyncs->setCellWidget(i, 1, megaFolder);

        // Col 3: Enabled/Disabled checkbox
        QCheckBox *c = new QCheckBox();
        c->setChecked(syncSetting->isActive()); //note: isEnabled refers to enable/disabled by the user. It could be temporary disabled or even failed. This should be shown in the UI
        c->setToolTip(tr("Enable / disable"));

        connect(c, SIGNAL(stateChanged(int)), this, SLOT(syncStateChanged(int)),Qt::QueuedConnection);

        mUi->tSyncs->setCellWidget(i, 2, c);

        // Col 4: tag. HIDDEN
        QLabel *lTag = new QLabel();
        lTag->setText(QString::number(syncSetting->backupId()));
        mUi->tSyncs->setCellWidget(i, 3, lTag);

        // Col 5: MegaHandle. HIDDEN
        QLabel *lHandle = new QLabel();
        lHandle->setText(QString::number(syncSetting->getMegaHandle()));
        mUi->tSyncs->setCellWidget(i, 4, lHandle);

        // Col 6: SyncName. HIDDEN
        QLabel *lName = new QLabel();
        lName->setText(syncSetting->name());
        mUi->tSyncs->setCellWidget(i, 5, lName);

        mSyncNames.append(syncSetting->name());
    }

    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
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

void SettingsDialog::on_bSessionHistory_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl,
                      QUrl(QString::fromUtf8("mega://#fm/account/security")));
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
        currentLocalFolders.append(static_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(i, 0))->fullPath());
        currentMegaFoldersPaths.append(static_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(i, 1))->fullPath());
    }

    QPointer<BindFolderDialog> dialog = new BindFolderDialog(mApp, mSyncNames, currentLocalFolders, currentMegaFoldersPaths, this);
    if(megaFolderHandle != mega::INVALID_HANDLE)
    {
        dialog->setMegaFolder(megaFolderHandle);
    }
    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    MegaHandle handle = dialog->getMegaFolder();
    if (!localFolderPath.length() || !dialog->getMegaPath().size())
    {
        delete dialog;
        return;
    }

    QSyncItemWidget *localFolder = new QSyncItemWidget(QSyncItemWidget::LOCAL_FOLDER);
    localFolder->setPath(localFolderPath, dialog->getSyncName());
    QSyncItemWidget *megaFolder = new QSyncItemWidget(QSyncItemWidget::REMOTE_FOLDER);

    //Check if need to setError here or it is enough setting when syncstatechanged
    megaFolder->setPath(dialog->getMegaPath());
    int pos = mUi->tSyncs->rowCount();
    mUi->tSyncs->setRowCount(pos+1);
    localFolder->setToolTip(localFolderPath);
    mUi->tSyncs->setCellWidget(pos, 0, localFolder);
    megaFolder->setToolTip(dialog->getMegaPath());
    mUi->tSyncs->setCellWidget(pos, 1, megaFolder);

    QCheckBox *c = new QCheckBox();
    c->setChecked(true);
    c->setToolTip(tr("Enable / disable"));
    connect(c, SIGNAL(stateChanged(int)), this, SLOT(syncStateChanged(int)),Qt::QueuedConnection);
    mUi->tSyncs->setCellWidget(pos, 2, c);

    // Col 5: MegaHandle. HIDDEN
    QLabel *lHandle = new QLabel();
    lHandle->setText(QString::number(handle));
    mUi->tSyncs->setCellWidget(pos, 4, lHandle);

    // Col 6: SyncName. HIDDEN
    QLabel *lName = new QLabel();
    lName->setText(dialog->getSyncName());
    mUi->tSyncs->setCellWidget(pos, 5, lName);

    mSyncNames.append(dialog->getSyncName());

    delete dialog;

    saveSyncSettings();
}

void SettingsDialog::on_bAdd_clicked()
{
    addSyncFolder(mega::INVALID_HANDLE);
}

void SettingsDialog::on_bLogout_clicked()
{
    QPointer<SettingsDialog> currentDialog = this;
    if (QMegaMessageBox::question(nullptr, tr("Logout"),
                                  tr("Synchronization will stop working.")+ QString::fromUtf8(" ")
                                  + tr("Are you sure?"),
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

void SettingsDialog::on_bRestart_clicked()
{
    QPointer<SettingsDialog> currentDialog = this;
    if (QMegaMessageBox::warning(nullptr, tr("Restart MEGAsync"),
                                 tr("Do you want to restart MEGAsync now?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes)
    {
        if (currentDialog)
        {
            // Restart MEGAsync
#if defined(Q_OS_MACX)
            mApp->rebootApplication(false);
#else
            //we enqueue this call, so as not to close before properly
            // handling the exit of Settings Dialog
            QTimer::singleShot(0, [] () {static_cast<MegaApplication*>(qApp)->rebootApplication(false);});
#endif
        }
    }
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

void SettingsDialog::on_tSyncs_doubleClicked(const QModelIndex &index)
{
    //FIXME: When using custom widget for row items, remove double check or use cellwidget to fix it.
    if (!index.column())
    {
        QString localFolderPath = static_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(index.row(), 0))->fullPath();
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localFolderPath));
    }
    else
    {
        QString megaFolderPath = static_cast<QSyncItemWidget*>(mUi->tSyncs->cellWidget(index.row(), 1))->fullPath();
        MegaNode *node = mMegaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
        if (node)
        {
            const char *handle = node->getBase64Handle();
            QString url = QString::fromUtf8("mega://#fm/") + QString::fromUtf8(handle);
            QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
            delete [] handle;
            delete node;
        }
    }
}

void SettingsDialog::on_bUploadFolder_clicked()
{
    QPointer<NodeSelector> nodeSelector = new NodeSelector(mMegaApi, NodeSelector::UPLOAD_SELECT, this);
    MegaNode* defaultNode = mMegaApi->getNodeByPath(mUi->eUploadFolder->text().toUtf8().constData());
    if (defaultNode)
    {
        nodeSelector->setSelectedFolderHandle(defaultNode->getHandle());
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

    MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode* node = mMegaApi->getNodeByHandle(selectedMegaFolderHandle);
    if (!node)
    {
        delete nodeSelector;
        return;
    }

    const char *nPath = mMegaApi->getNodePath(node);
    if (!nPath)
    {
        delete nodeSelector;
        delete node;
        return;
    }

    QString newPath = QString::fromUtf8(nPath);
    if (newPath.compare(mUi->eUploadFolder->text())
            || mHasDefaultUploadOption != nodeSelector->getDefaultUploadOption())
    {
        mHasDefaultUploadOption = nodeSelector->getDefaultUploadOption();
        mUi->eUploadFolder->setText(newPath);
    }

    delete nodeSelector;
    delete [] nPath;
    delete node;
}

void SettingsDialog::on_bDownloadFolder_clicked()
{
    QPointer<DownloadFromMegaDialog> dialog = new DownloadFromMegaDialog(mPreferences->downloadFolder(), this);
    dialog->setDefaultDownloadOption(mHasDefaultDownloadOption);

    int result = dialog->exec();
    if (!dialog || (result != QDialog::Accepted))
    {
        delete dialog;
        return;
    }

    QString fPath = dialog->getPath();
    if (fPath.size() && (fPath.compare(mUi->eDownloadFolder->text())
                         || (mHasDefaultDownloadOption != dialog->isDefaultDownloadOption())))
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
    }

    delete dialog;
}

void SettingsDialog::on_bAddName_clicked()
{
    QPointer<AddExclusionDialog> add = new AddExclusionDialog(this);
    int result = add->exec();
    if (!add || (result != QDialog::Accepted))
    {
        delete add;
        return;
    }

    QString text = add->textValue();
    delete add;

    if (text.isEmpty())
    {
        return;
    }

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
    mPreferences->setCrashed(true);
    mUi->eUpperThan->setEnabled(enable);
    mUi->cbExcludeUpperUnit->setEnabled(enable);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();
}

void SettingsDialog::on_cExcludeLowerThan_clicked()
{
    if (mLoadingSettings) return;
    bool enable (mUi->cExcludeLowerThan->isChecked());
    mPreferences->setLowerSizeLimit(enable);
    mPreferences->setCrashed(true);
    mUi->eLowerThan->setEnabled(enable);
    mUi->cbExcludeLowerUnit->setEnabled(enable);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();
}

void SettingsDialog::on_eUpperThan_valueChanged(int i)
{
    if (mLoadingSettings) return;
    mPreferences->setUpperSizeLimitValue(i);
    mPreferences->setCrashed(true);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();
}

void SettingsDialog::on_eLowerThan_valueChanged(int i)
{
    if (mLoadingSettings) return;
    mPreferences->setLowerSizeLimitValue(i);
    mPreferences->setCrashed(true);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();
}

void SettingsDialog::on_cbExcludeUpperUnit_currentIndexChanged(int index)
{
    if (mLoadingSettings) return;
    mPreferences->setUpperSizeLimitUnit(index);
    mPreferences->setCrashed(true);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();
}

void SettingsDialog::on_cbExcludeLowerUnit_currentIndexChanged(int index)
{
    if (mLoadingSettings) return;
    mPreferences->setLowerSizeLimitUnit(index);
    mPreferences->setCrashed(true);
    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();
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
        mApp->cleanLocalCaches();
    }
}

void SettingsDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);

#ifdef Q_OS_MACOS
        // FIXME: Do we need to do the same for the other buttons?
        bAccount.get()->setText(tr("Account"));
        //review and check
        ui->cStartOnStartup->setText(tr("Open at login"));
#endif
        onCacheSizeAvailable();

        updateNetworkTab();
    }
    QDialog::changeEvent(event);
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
    mPreferences->setCrashed(true);

    mUi->gExcludedFilesInfo->show();
    mUi->bRestart->show();
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
        mUi->lUploadRateLimit->setText(tr("Don't limit"));
    }

    int downloadLimitKB = mPreferences->downloadLimitKB();
    if (downloadLimitKB > 0)
    {
        mUi->lDownloadRateLimit->setText(QStringLiteral("%1 KB/s").arg(downloadLimitKB));
    }
    else
    {
        mUi->lDownloadRateLimit->setText(tr("Don't limit"));
    }

    switch (mPreferences->proxyType())
    {
        case Preferences::PROXY_TYPE_NONE:
            mUi->lProxySettings->setText(tr("No proxy"));
            break;
        case Preferences::PROXY_TYPE_AUTO:
            mUi->lProxySettings->setText(tr("Auto"));
            break;
        case Preferences::PROXY_TYPE_CUSTOM:
            mUi->lProxySettings->setText(tr("Manual"));
            break;
    }
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
            MegaApplication* megaApp{static_cast<MegaApplication*>(qApp)};
            megaApp->updateUserStats(true, false, false, true, USERSTATS_REMOVEVERSIONS);
        }
    }));
}

void SettingsDialog::savingSyncs(bool completed, QObject* item)
{
    if (!item)
    {
        return;
    }

    for(auto *widget : item->findChildren<QWidget*>())
    {
        widget->setEnabled(completed);
    }

#ifndef Q_OS_MACOS
    mUi->bGeneral->setEnabled(completed);
    mUi->bAccount->setEnabled(completed);
    mUi->bSyncs->setEnabled(completed);
    mUi->bNetwork->setEnabled(completed);
    mUi->bSecurity->setEnabled(completed);
    mUi->bImports->setEnabled(completed);
#else
    toolBar->setEnableToolbarItems(completed);
#endif
}

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
            }
            else
            {
                mUi->sSyncsState->setCurrentWidget(mUi->pNoErrors);
            }
        }
            break;
    }
}

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
        mUi->lBandwidth->setText(tr("Used quota for the last %1 hours:")
                                .arg(mPreferences->bandwidthInterval()));
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
                                        Utilities::getSizeString(usedBandwidth)));
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
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/business.png"));
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

void SettingsDialog::on_bUpdate_clicked()
{
    if (mUi->bUpdate->text() == tr("Check for updates"))
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
    mPreferences->setCrashed(true);
    QPointer<SettingsDialog> currentDialog = this;
    if (QMegaMessageBox::warning(nullptr, tr("Full scan"),
                                 tr("MEGAsync will perform a full scan of your synced folders"
                                    " when it starts.\n\nDo you want to restart MEGAsync now?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            == QMessageBox::Yes)
    {
        if (currentDialog)
        {
            on_bRestart_clicked();
        }
    }
}

void SettingsDialog::setAvatar()
{
    const char* email = mMegaApi->getMyEmail();
    if (email)
    {
        mUi->wAvatar->drawAvatarFromEmail(QString::fromUtf8(email));
        delete [] email;
    }
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

void SettingsDialog::setUpdateAvailable(bool updateAvailable)
{
    if (updateAvailable)
    {
        mUi->bUpdate->setText(tr("Install update"));
    }
    else
    {
        mUi->bUpdate->setText(tr("Check for updates"));
    }
}

void SettingsDialog::openSettingsTab(int tab)
{
    if(mProxyOnly) // do not switch tabs when in guest mode
        return;

    switch (tab)
    {
    case GENERAL_TAB:
        mReloadUIpage = true;
#ifndef Q_OS_MACOS
        mUi->bGeneral->click();
#else
        toolBar->setSelectedItem(bGeneral.get());
        emit bGeneral.get()->activated();
#endif
        break;

    case ACCOUNT_TAB:
        mReloadUIpage = true;
#ifndef Q_OS_MACOS
        mUi->bAccount->click();
#else
        toolBar->setSelectedItem(bAccount.get());
        emit bAccount.get()->activated();
#endif
        break;

    case SYNCS_TAB:
#ifndef Q_OS_MACOS
        mUi->bSyncs->click();
#else
        toolBar->setSelectedItem(bSyncs.get());
        emit bSyncs.get()->activated();
#endif
        break;

    case SECURITY_TAB:
#ifndef Q_OS_MACOS
        mUi->bSecurity->click();
#else
        toolBar->setSelectedItem(bSecurity.get());
        emit bSecurity.get()->activated();
#endif
        break;

    case IMPORTS_TAB:
#ifndef Q_OS_MACOS
        mUi->bImports->click();
#else
        toolBar->setSelectedItem(bImports.get());
        emit bImports.get()->activated();
#endif
        break;

    case NETWORK_TAB:
#ifndef Q_OS_MACOS
        mUi->bNetwork->click();
#else
        toolBar->setSelectedItem(bNetwork.get());
        emit bNetwork.get()->activated();
#endif
        break;

    default:
        break;
    }
}

void SettingsDialog::on_lAccountType_clicked()
{
    mDebugCounter++;
    if (mDebugCounter == 5)
    {
        mApp->toggleLogging();
        mDebugCounter = 0;
    }
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

#ifndef Q_OS_MACOS
void SettingsDialog::on_bHelpIco_clicked()
{
    on_bHelp_clicked();
}
#endif

#ifdef Q_OS_MACOS
void SettingsDialog::onAnimationFinished()
{
    if (ui->wStack->currentWidget() == ui->pGeneral)
    {
        ui->pGeneral->show();
    }
    else if (ui->wStack->currentWidget() == ui->pAccount)
    {
        ui->pAccount->show();
    }
    else if (ui->wStack->currentWidget() == ui->pSyncs)
    {
        ui->pSyncs->show();
    }
    else if (ui->wStack->currentWidget() == ui->pImports)
    {
        ui->pImports->show();
    }
    else if (ui->wStack->currentWidget() == ui->pNetwork)
    {
        ui->pNetwork->show();
    }
    else if (ui->wStack->currentWidget() == ui->pSecurity)
    {
        ui->pSecurity->show();
    }
}

void SettingsDialog::animateSettingPage(int endValue, int duration)
{
    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(minimumHeight());
    maxHeightAnimation->setStartValue(maximumHeight());
    minHeightAnimation->setEndValue(endValue);
    maxHeightAnimation->setEndValue(endValue);
    minHeightAnimation->setDuration(duration);
    maxHeightAnimation->setDuration(duration);
    animationGroup->start();
}
#endif

void SettingsDialog::on_cShowNotifications_toggled(bool checked)
{
    if (mLoadingSettings) return;
    mPreferences->setShowNotifications(checked);
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
        QString currentLanguage = mApp->getCurrentLanguageCode();
        mThreadPool->push([=]()
        {
            mMegaApi->setLanguage(currentLanguage.toUtf8().constData());
            mMegaApi->setLanguagePreference(currentLanguage.toUtf8().constData());
        });
    }
}

void SettingsDialog::on_eUploadFolder_textChanged(const QString &text)
{
    if (mLoadingSettings) return;
    MegaNode* node = mMegaApi->getNodeByPath(text.toUtf8().constData());
    if (node)
    {
        mPreferences->setHasDefaultUploadFolder(mHasDefaultUploadOption);
        mPreferences->setUploadFolder(static_cast<long long>(node->getHandle()));
    }
    else
    {
        mPreferences->setHasDefaultUploadFolder(false);
        mPreferences->setUploadFolder(0);
    }
    delete node;
}

void SettingsDialog::on_eDownloadFolder_textChanged(const QString &text)
{
    if (mLoadingSettings) return;
    QString defaultDownloadPath = Utilities::getDefaultBasePath()
                                  + QString::fromUtf8("/MEGAsync Downloads");
    if (text.compare(QDir::toNativeSeparators(defaultDownloadPath))
        || mPreferences->downloadFolder().size())
    {
        mPreferences->setDownloadFolder(text);
    }

    mPreferences->setHasDefaultDownloadFolder(mHasDefaultDownloadOption);
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
    mMegaApi->setFileVersionsOption(!checked); // This is actually saved to Preferences after the megaApi call succeeds
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

    mApp->setMaxConnections(MegaTransfer::TYPE_UPLOAD, mPreferences->parallelUploadConnections());
    mApp->setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, mPreferences->parallelDownloadConnections());

    mApp->setUseHttpsOnly(mPreferences->usingHttpsOnly());

    updateNetworkTab();
}

#ifdef Q_OS_WINDOWS
void SettingsDialog::on_cFinderIcons_toggled(bool checked)
{
    if (loadingSettings) return;
    if (checked)
    {
        for (int i = 0; i < model->getNumSyncedFolders(); i++)
        {
            auto syncSetting = model->getSyncSetting(i);
            Platform::addSyncToLeftPane(syncSetting->getLocalFolder(),
                                        syncSetting->name(),
                                        syncSetting->getSyncID());
        }
    }
    else
    {
        Platform::removeAllSyncsFromLeftPane();
    }
    preferences->disableLeftPaneIcons(!checked);
}
#endif
