#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>
#include <QTranslator>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QButtonGroup>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

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
using namespace std;

#ifdef Q_OS_MACOS
//Const values used for macOS Settings dialog resize animation
constexpr auto SETTING_ANIMATION_PAGE_TIMEOUT{150};//ms
constexpr auto SETTING_ANIMATION_GENERAL_TAB_HEIGHT{590};
constexpr auto SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT{295};//px height
constexpr auto SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT_BUSINESS{260};
constexpr auto SETTING_ANIMATION_SYNCS_TAB_HEIGHT{529};
constexpr auto SETTING_ANIMATION_IMPORTS_TAB_HEIGHT{513};
// FIXME: Re-evaluate sizes for Network tab
constexpr auto SETTING_ANIMATION_NETWORK_TAB_HEIGHT{190};
constexpr auto SETTING_ANIMATION_SECURITY_TAB_HEIGHT{400};
#endif

const QString SYNCS_TAB_MENU_LABEL_QSS = QString::fromUtf8("QLabel{"
                                                    "border-image: url(%1);"
                                                    "margin-left: 7px;"
                                                    "margin-top: 5px;"
                                                    "width: 16px;"
                                                    "height: 16px;"
                                                    "min-width: 16px;"
                                                    "min-height: 16px;"
                                                    "max-width: 16px;"
                                                    "max-height: 16px;"
                                                    "}");

long long calculateCacheSize()
{
    Model *model = Model::instance();
    long long cacheSize = 0;
    for (int i = 0; i < model->getNumSyncedFolders(); i++)
    {
        auto syncSetting = model->getSyncSetting(i);
        QString syncPath = syncSetting->getLocalFolder();
        if (!syncPath.isEmpty())
        {
            Utilities::getFolderSize(syncPath + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER), &cacheSize);
        }
    }
    return cacheSize;
}

void deleteCache()
{
    ((MegaApplication *)qApp)->cleanLocalCaches(true);
}

long long calculateRemoteCacheSize(MegaApi *megaApi)
{
    MegaNode *n = megaApi->getNodeByPath("//bin/SyncDebris");
    long long toret = megaApi->getSize(n);
    delete n;
    return toret;
}

void deleteRemoteCache(MegaApi *megaApi)
{
    MegaNode *n = megaApi->getNodeByPath("//bin/SyncDebris");
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

SettingsDialog::SettingsDialog(MegaApplication *app, bool proxyOnly, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    mSelectedSyncRow (-1)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // TODO: get rid of this->, use unique member names when updating with m prefix
    this->app = app;
    this->megaApi = app->getMegaApi();
    this->preferences = Preferences::instance();
    this->controller = Controller::instance();
    this->model = Model::instance();

    mThreadPool = ThreadPoolSingleton::getInstance();

    loadingSettings = 0;
    accountDetailsDialog = NULL;
    cacheSize = -1;
    remoteCacheSize = -1;
    connect(ui->wStack, SIGNAL(currentChanged(int)), ui->wStackFooter, SLOT(setCurrentIndex(int)));
    ui->wStack->setCurrentWidget(ui->pGeneral); // override whatever might be set in .ui

#ifndef Q_OS_MAC
    ui->bGeneral->setChecked(true); // override whatever might be set in .ui
    ui->gCache->setTitle(ui->gCache->title().arg(QString::fromAscii(MEGA_DEBRIS_FOLDER)));
#endif
    setProxyOnly(proxyOnly);

    reloadUIpage = false;
    debugCounter = 0;
    areSyncsDisabled = false;
    isSavingSyncsOnGoing = false;

#ifdef Q_OS_LINUX
    ui->wUpdateSection->hide();
#endif

    ui->gExcludedFilesInfo->hide();

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

    ui->bRestart->hide();

    highDpiResize.init(this);
    ((MegaApplication*)qApp)->attachStorageObserver(*this);
    ((MegaApplication*)qApp)->attachBandwidthObserver(*this);
    ((MegaApplication*)qApp)->attachAccountObserver(*this);

    connect(app, &MegaApplication::avatarReady, this, &SettingsDialog::setAvatar);
    setAvatar();
    connect(app, SIGNAL(storageStateChanged(int)), this, SLOT(storageStateChanged(int)));
    storageStateChanged(app->getAppliedStorageState());

    connect(this->model, SIGNAL(syncStateChanged(std::shared_ptr<SyncSetting>)),
            this, SLOT(onSyncStateChanged(std::shared_ptr<SyncSetting>)));
    connect(this->model, SIGNAL(syncRemoved(std::shared_ptr<SyncSetting>)),
            this, SLOT(onSyncStateChanged(std::shared_ptr<SyncSetting>)));

    connect(ui->tSyncs, &QTableWidget::cellClicked,
            this, &SettingsDialog::onCellClicked);

    connect(ui->tSyncs->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &SettingsDialog::onSyncSelected);
    ui->tSyncs->setMouseTracking(true);
    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);        
}

SettingsDialog::~SettingsDialog()
{
    ((MegaApplication*)qApp)->dettachStorageObserver(*this);
    ((MegaApplication*)qApp)->dettachBandwidthObserver(*this);
    ((MegaApplication*)qApp)->dettachAccountObserver(*this);

    delete ui;
}

void SettingsDialog::setProxyOnly(bool proxyOnly)
{
    this->proxyOnly = proxyOnly;

#ifndef Q_OS_MACOS
    ui->bGeneral->setEnabled(!proxyOnly);
    ui->bAccount->setEnabled(!proxyOnly);
    ui->bSyncs->setEnabled(!proxyOnly);
    ui->bSecurity->setEnabled(!proxyOnly);
    ui->bImports->setEnabled(!proxyOnly);
#endif

    if (proxyOnly)
    {
#ifdef Q_OS_MACOS
        // TODO: Re-evaluate sizes for Network tab
        setMinimumHeight(435);
        setMaximumHeight(435);
        toolBar->setSelectedItem(bNetwork.get());
#else
        ui->bNetwork->setEnabled(true);
        ui->bNetwork->setChecked(true);
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
        ui->wOQError->show();
    }
    else
    {
        ui->wOQError->hide();
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
    QMegaMessageBox::critical(nullptr, tr("Error"), tr("Unexpected error disabling sync %1").arg(syncSetting->name()));
}

void SettingsDialog::showGuestMode()
{
    ui->wStack->setCurrentWidget(ui->pNetwork);
    ui->pNetwork->show();
    ProxySettings *proxySettingsDialog = new ProxySettings(app, this);
    proxySettingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    proxySettingsDialog->setWindowModality(Qt::WindowModal);
    proxySettingsDialog->open();
    connect(proxySettingsDialog, &ProxySettings::finished, this, [this](int result)
    {
        if (result == QDialog::Accepted)
        {
            app->applyProxySettings();
            if (proxyOnly) accept(); // close Settings in guest mode
        }
        else
        {
            if (proxyOnly) reject(); // close Settings in guest mode
        }
        delete mProxySettingsDialog;
    });
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

    for (int i = 0; i < ui->tSyncs->rowCount(); ++i)
    {
        bool select (false);
        QString menuRsc (QString::fromUtf8("://images/Item_options_rest.png"));

        if (i == row)
        {
            menuRsc = QString::fromUtf8("://images/Item_options_press.png");
            select = true;
        }

        // Paths
        auto w (qobject_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(i, SYNC_COL_LFOLDER)));
        // Check if the row has not been deleted
        if (w)
        {
            w->setSelected(select);
            w = qobject_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(i, SYNC_COL_RFOLDER));
            w->setSelected(select);

            // Menu
            auto lMenu (qobject_cast<QLabel*>(ui->tSyncs->cellWidget(i, SYNC_COL_MENU)));
            lMenu->setStyleSheet(SYNCS_TAB_MENU_LABEL_QSS.arg(menuRsc));
        }
    }
}

// errorCode refers to the requestErrorCode.
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
               tr("Your sync \"%1\" can't be enabled. Reason: %2").arg(syncSetting->name())
              .arg(tr(MegaSync::getMegaSyncErrorCode(errorCode))));
            break;
        }
    }

    loadSyncSettings();
}

void SettingsDialog::onLocalCacheSizeAvailable()
{
    cacheSize = cacheSizeWatcher.result();
    onCacheSizeAvailable();
}

void SettingsDialog::onRemoteCacheSizeAvailable()
{
    remoteCacheSize = remoteCacheSizeWatcher.result();
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
    savingSyncs(false, ui->pSyncs);
    isSavingSyncsOnGoing = true;
}

void SettingsDialog::onSavingSettingsCompleted()
{
    auto closeDelay = max(qint64(0), 350 - (QDateTime::currentMSecsSinceEpoch() - ui->wSpinningIndicator->getStartTime()));
    QTimer::singleShot(closeDelay, this, [this] () {
        isSavingSyncsOnGoing = false;
        syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
        savingSyncs(true, ui->pSyncs);
    });
}

void SettingsDialog::storageStateChanged(int newStorageState)
{
     setOverQuotaMode(newStorageState == MegaApi::STORAGE_STATE_RED || newStorageState == MegaApi::STORAGE_STATE_PAYWALL);
}

void SettingsDialog::onCacheSizeAvailable()
{
    if(!preferences->logged())
        return;

    ui->lFileVersionsSize->setText(tr("%1").arg(Utilities::getSizeString(preferences->versionsStorage())));

    if (cacheSize != -1)
        ui->lCacheSize->setText(QString::fromUtf8("%1").arg(Utilities::getSizeString(cacheSize)));
    if (remoteCacheSize != -1)
        ui->lRemoteCacheSize->setText(QString::fromUtf8("%1").arg(Utilities::getSizeString(remoteCacheSize)));

    ui->bClearCache->setEnabled(cacheSize > 0);
    ui->bClearRemoteCache->setEnabled(remoteCacheSize > 0);
    ui->bClearFileVersions->setEnabled(preferences->versionsStorage() > 0);
}

void SettingsDialog::on_bGeneral_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pGeneral && !reloadUIpage)
    {
        return;
    }

    reloadUIpage = false;

    ui->wStack->setCurrentWidget(ui->pGeneral);

#ifdef Q_OS_MACOS
    onCacheSizeAvailable();

    ui->pGeneral->hide();
    animateSettingPage(SETTING_ANIMATION_GENERAL_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bAccount_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pAccount && !reloadUIpage)
    {
        return;
    }

    reloadUIpage = false;

    ui->wStack->setCurrentWidget(ui->pAccount);

#ifdef Q_OS_MACOS
    ui->pAccount->hide();
    if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        animateSettingPage(SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT_BUSINESS, SETTING_ANIMATION_PAGE_TIMEOUT);
    }
    else
    {
        animateSettingPage(SETTING_ANIMATION_ACCOUNT_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
    }
#endif
}

void SettingsDialog::on_bSyncs_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pSyncs)
    {
        return;
    }

    ui->wStack->setCurrentWidget(ui->pSyncs);
    ui->tSyncs->horizontalHeader()->setVisible(true);

#ifdef Q_OS_MACOS
    ui->pSyncs->hide();
    animateSettingPage(SETTING_ANIMATION_SYNCS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bSecurity_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pSecurity)
    {
        return;
    }

    ui->wStack->setCurrentWidget(ui->pSecurity);

#ifdef Q_OS_MACOS
    ui->pSecurity->hide();
    animateSettingPage(SETTING_ANIMATION_SECURITY_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bImports_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pImports)
    {
        return;
    }
    ui->wStack->setCurrentWidget(ui->pImports);

#ifdef Q_OS_MACOS
    ui->pImports->hide();
    animateSettingPage(SETTING_ANIMATION_IMPORTS_TAB_HEIGHT, SETTING_ANIMATION_PAGE_TIMEOUT);
#endif
}

void SettingsDialog::on_bNetwork_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pNetwork)
    {
        return;
    }

    ui->wStack->setCurrentWidget(ui->pNetwork);

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

void SettingsDialog::on_bBuyMoreSpace_clicked()
{
    on_bUpgrade_clicked();
}

void SettingsDialog::on_bMyAccount_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#fm/account")));
}

void SettingsDialog::updateUploadFolder()
{
    MegaNode *node = megaApi->getNodeByHandle(preferences->uploadFolder());
    if (!node)
    {
        hasDefaultUploadOption = false;
        ui->eUploadFolder->setText(QString::fromUtf8("/MEGAsync Uploads"));
    }
    else
    {
        const char *nPath = megaApi->getNodePath(node);
        if (!nPath)
        {
            hasDefaultUploadOption = false;
            ui->eUploadFolder->setText(QString::fromUtf8("/MEGAsync Uploads"));
        }
        else
        {
            hasDefaultUploadOption = preferences->hasDefaultUploadFolder();
            ui->eUploadFolder->setText(QString::fromUtf8(nPath));
            delete [] nPath;
        }
    }
    delete node;
}

void SettingsDialog::updateDownloadFolder()
{
    QString downloadPath = preferences->downloadFolder();
    if (!downloadPath.size())
    {
        downloadPath = Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGAsync Downloads");
    }
    downloadPath = QDir::toNativeSeparators(downloadPath);
    ui->eDownloadFolder->setText(downloadPath);
    hasDefaultDownloadOption = preferences->hasDefaultDownloadFolder();
}

void SettingsDialog::loadSettings()
{
    loadingSettings++;

    if (preferences->logged())
    {
        connect(&cacheSizeWatcher, SIGNAL(finished()), this, SLOT(onLocalCacheSizeAvailable()));
        QFuture<long long> futureCacheSize = QtConcurrent::run(calculateCacheSize);
        cacheSizeWatcher.setFuture(futureCacheSize);

        connect(&remoteCacheSizeWatcher, SIGNAL(finished()), this, SLOT(onRemoteCacheSizeAvailable()));
        QFuture<long long> futureRemoteCacheSize = QtConcurrent::run(calculateRemoteCacheSize,megaApi);
        remoteCacheSizeWatcher.setFuture(futureRemoteCacheSize);
    }

    //General
    ui->cFileVersioning->setChecked(!preferences->fileVersioningDisabled());
    ui->cOverlayIcons->setChecked(!preferences->overlayIconsDisabled());
    ui->cCacheSchedulerEnabled->setChecked(preferences->cleanerDaysLimit());
    ui->sCacheSchedulerDays->setEnabled(preferences->cleanerDaysLimit());
    ui->sCacheSchedulerDays->setValue(preferences->cleanerDaysLimitValue());

    ui->cShowNotifications->setChecked(preferences->showNotifications());

    if (!preferences->canUpdate(MegaApplication::applicationFilePath()))
    {
        ui->bUpdate->setEnabled(false);
        ui->cAutoUpdate->setEnabled(false);
        ui->cAutoUpdate->setChecked(false);
    }
    else
    {
        ui->bUpdate->setEnabled(true);
        ui->cAutoUpdate->setEnabled(true);
        ui->cAutoUpdate->setChecked(preferences->updateAutomatically());
    }

    // if checked: make sure both sources are true
    ui->cStartOnStartup->setChecked(preferences->startOnStartup() && Platform::isStartOnStartupActive());

    //Language
    ui->cLanguage->clear();
    languageCodes.clear();
    QString fullPrefix = Preferences::TRANSLATION_FOLDER+Preferences::TRANSLATION_PREFIX;
    QDirIterator it(Preferences::TRANSLATION_FOLDER);
    QStringList languages;
    int currentIndex = -1;
    QString currentLanguage = preferences->language();
    while (it.hasNext())
    {
        QString file = it.next();
        if (file.startsWith(fullPrefix))
        {
            int extensionIndex = file.lastIndexOf(QString::fromAscii("."));
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
                languageCodes.insert(i, languageCode);
            }
        }
    }

    for (int i = languageCodes.size() - 1; i >= 0; i--)
    {
        if (currentLanguage.startsWith(languageCodes[i]))
        {
            currentIndex = i;
            break;
        }
    }

    if (currentIndex == -1)
    {
        currentIndex = languageCodes.indexOf(QString::fromAscii("en"));
    }

    ui->cLanguage->addItems(languages);
    ui->cLanguage->setCurrentIndex(currentIndex);

    //Account
    ui->lEmail->setText(preferences->email());
    auto fullName {(preferences->firstName() + QStringLiteral(" ")+ preferences->lastName()).trimmed()};
    ui->lName->setText(fullName);

    // account type and details
    updateAccountElements();
    updateStorageElements();
    updateBandwidthElements();

    if (accountDetailsDialog)
    {
        accountDetailsDialog->refresh(preferences);
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
    ui->lExcludedNames->clear();
    QStringList excludedNames = preferences->getExcludedSyncNames();
    for (int i = 0; i < excludedNames.size(); i++)
    {
        ui->lExcludedNames->addItem(excludedNames[i]);
    }

    QStringList excludedPaths = preferences->getExcludedSyncPaths();
    for (int i = 0; i < excludedPaths.size(); i++)
    {
        ui->lExcludedNames->addItem(excludedPaths[i]);
    }

    bool upperSizeLimit (preferences->upperSizeLimit());
    bool lowerSizeLimit (preferences->lowerSizeLimit());

    for (auto cb : {ui->cbExcludeUpperUnit, ui->cbExcludeLowerUnit})
    {
        cb->clear();
        cb->addItem(tr("Bytes"));
        cb->addItem(tr("KB"));
        cb->addItem(tr("MB"));
        cb->addItem(tr("GB"));
    }

    ui->eLowerThan->setMaximum(9999);
    ui->cExcludeUpperThan->setChecked(upperSizeLimit);
    ui->eUpperThan->setEnabled(upperSizeLimit);
    ui->cbExcludeUpperUnit->setEnabled(upperSizeLimit);
    ui->eUpperThan->setValue(preferences->upperSizeLimitValue());
    ui->cbExcludeUpperUnit->setCurrentIndex(preferences->upperSizeLimitUnit());

    ui->eUpperThan->setMaximum(9999);
    ui->cExcludeLowerThan->setChecked(lowerSizeLimit);
    ui->eLowerThan->setEnabled(lowerSizeLimit);
    ui->cbExcludeLowerUnit->setEnabled(lowerSizeLimit);
    ui->eLowerThan->setValue(preferences->lowerSizeLimitValue());
    ui->cbExcludeLowerUnit->setCurrentIndex(preferences->lowerSizeLimitUnit());

    loadingSettings--;
}

void SettingsDialog::saveSyncSettings()
{
    saveSettingsProgress.reset(new ProgressHelper(false, tr("Saving Sync settings")));
    connect(saveSettingsProgress.get(), &ProgressHelper::progress,
            this, &SettingsDialog::onSavingSettingsProgress);
    connect(saveSettingsProgress.get(), &ProgressHelper::completed,
            this, &SettingsDialog::onSavingSettingsCompleted);

    // Uncomment the following to see a progress bar when saving
    // (which being modal will prevent from modifying while changing)
    //Utilities::showProgressDialog(saveSettingsProgress.get(), this);

    ProgressHelperCompletionGuard g(saveSettingsProgress.get());

    if (!proxyOnly)
    {
        onSavingSettingsProgress(0);

        // 1 - loop through the syncs in the model to remove or update
        for (int i = 0; i < model->getNumSyncedFolders(); i++)
        {
            auto syncSetting = model->getSyncSetting(i);
            if (!syncSetting)
            {
                assert("missing setting when looping for saving");
                continue;
            }

            // 1.1 - remove no longer present:
            bool found = false;
            for (int j = 0; j < ui->tSyncs->rowCount(); j++)
            {
                auto tagItem = ui->tSyncs->cellWidget(j, SYNC_COL_TAG);
                if (tagItem
                        && static_cast<QLabel*>(tagItem)->text().toULongLong()
                        == syncSetting->backupId())
                {
                    found = true;
                    break;
                }
            }
            if (!found) //sync no longer found in settings: needs removing
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                             QString::fromUtf8("Removing sync: %1").arg(
                                 syncSetting->name()).toUtf8().constData());
                ActionProgress* removeSyncStep =
                        new ActionProgress(true, QString::fromUtf8("Removing sync: %1 - %2").arg(
                                               syncSetting->getLocalFolder(),
                                               syncSetting->getMegaFolder()));
                saveSettingsProgress->addStep(removeSyncStep);
                controller->removeSync(syncSetting, removeSyncStep);
            }

            // 1.2 - enable/disable changed syncs
            for (int j = 0; j < ui->tSyncs->rowCount(); j++)
            {
                QWidget* cbParent (ui->tSyncs->cellWidget(j, SYNC_COL_ENABLE_CB));
                auto cb (cbParent->findChild<QCheckBox*>());
                bool enabled = cb->isChecked();
                bool disabled = !enabled;

                auto tagItem = qobject_cast<QLabel*>(ui->tSyncs->cellWidget(j, SYNC_COL_TAG));

                if (tagItem && tagItem->text().toULongLong() == syncSetting->backupId())
                {
                    if (disabled && syncSetting->isActive()) //sync disabled
                    {
                        ActionProgress* disableSyncStep
                                = new ActionProgress(true,
                                                     QString::fromUtf8("Disabling sync: %1 - %2")
                                                     .arg(syncSetting->getLocalFolder(),
                                                          syncSetting->getMegaFolder()));
                        saveSettingsProgress->addStep(disableSyncStep);

                        connect(disableSyncStep, &ActionProgress::failedRequest,
                                this, [this, syncSetting](MegaRequest* request, MegaError* error)
                        {
                            Q_UNUSED(request)
                            if (error->getErrorCode())
                            {
                                QObject temporary;
                                QObject::connect(&temporary, &QObject::destroyed,
                                                 this, [this, syncSetting](){
                                    //Note: this might get executed before onSyncStateChanged!
                                    onDisableSyncFailed(syncSetting);
                                    loadSyncSettings(); //to no longer show the failed line
                                }, Qt::QueuedConnection);
                            }
                        }, Qt::DirectConnection); //We need direct connection to use request & error

                        controller->disableSync(syncSetting, disableSyncStep);
                    }
                    else if (enabled && !syncSetting->isActive()) //sync re-enabled!
                    {
                        ActionProgress* enableSyncStep
                                = new ActionProgress(true,
                                                     QString::fromUtf8("Enabling sync: %1 - %2")
                                                     .arg(syncSetting->getLocalFolder(),
                                                          syncSetting->getMegaFolder()));
                        saveSettingsProgress->addStep(enableSyncStep);
                        connect(enableSyncStep, &ActionProgress::failedRequest,
                                this, [this, syncSetting](MegaRequest* request, MegaError* error)
                        {
                            if (error->getErrorCode())
                            {
                                auto syncError = request->getNumDetails();
                                QObject temporary;
                                QObject::connect(&temporary, &QObject::destroyed,
                                                 this, [this, syncError, syncSetting](){
                                    //Note: this might get executed before onSyncStateChanged!
                                    //(syncSettings might have some old values), that's why we
                                    // don't use syncSetting->getError.
                                    onEnableSyncFailed(syncError, syncSetting);
                                    loadSyncSettings(); //to no longer show the failed line
                                }, Qt::QueuedConnection);
                            }
                        }, Qt::DirectConnection); //We need direct connection to use request & error

                        controller->enableSync(syncSetting, enableSyncStep);
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
        for (int j = 0; j < ui->tSyncs->rowCount(); j++)
        {
            auto tagItem = ui->tSyncs->cellWidget(j, SYNC_COL_TAG);
            if (!tagItem) //not found: new sync
            {
                QWidget* cbParent (ui->tSyncs->cellWidget(j, SYNC_COL_ENABLE_CB));
                auto cb (cbParent->findChild<QCheckBox*>());
                bool enabled = cb->isChecked();
                if (enabled)
                {
                    QWidget* w (ui->tSyncs->cellWidget(j,SYNC_COL_LFOLDER));
                    QString localFolderPath = qobject_cast<QSyncItemWidget*>(w)->fullPath();
                    w = ui->tSyncs->cellWidget(j, SYNC_COL_RFOLDER);
                    QString megaFolderPath = qobject_cast<QSyncItemWidget*>(w)->fullPath();
                    w = ui->tSyncs->cellWidget(j, SYNC_COL_HANDLE);
                    MegaHandle nodeHandle = qobject_cast<QLabel*>(w)->text().toULongLong();
                    w = ui->tSyncs->cellWidget(j, SYNC_COL_NAME);
                    QString syncName =qobject_cast<QLabel*>(w)->text();

                    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                                 QString::fromUtf8("Adding sync from Settings: %1 - %2")
                                 .arg(localFolderPath, megaFolderPath).toUtf8().constData());

                    ActionProgress* addSyncStep =
                            new ActionProgress(true, QString::fromUtf8("Adding sync: %1 - %2")
                                               .arg(localFolderPath, megaFolderPath));
                    saveSettingsProgress->addStep(addSyncStep);

                    //Connect failing signals
                    connect(addSyncStep, &ActionProgress::failed,
                            this, [this, localFolderPath, megaFolderPath](int errorCode)
                    {
                        auto app (static_cast<MegaApplication*>(qApp));
                        app->showAddSyncError(errorCode, localFolderPath, megaFolderPath);
                        loadSyncSettings(); //to no longer show the failed line
                    }, Qt::QueuedConnection);

                    connect(addSyncStep, &ActionProgress::failedRequest,
                            this, [this, localFolderPath, megaFolderPath](MegaRequest* request,
                            MegaError* error)
                    {
                        if (error->getErrorCode())
                        {
                            auto reqCopy = request->copy();
                            auto errCopy = error->copy();

                            QObject temporary;
                            QObject::connect(&temporary, &QObject::destroyed,
                                             this, [this, reqCopy, errCopy, localFolderPath,
                                             megaFolderPath](){

                                // we might want to handle this separately
                                // (i.e: indicate errors in SyncSettings engine)
                                auto app (static_cast<MegaApplication*>(qApp));
                                app->showAddSyncError(reqCopy, errCopy, localFolderPath,
                                                      megaFolderPath);
                                loadSyncSettings();  //to no longer show the failed line

                                delete reqCopy;
                                delete errCopy;
                                //(syncSettings might have some old values),
                                // that's why we don't use syncSetting->getError.
                            }, Qt::QueuedConnection);
                        }
                    }, Qt::DirectConnection); //Note, we need direct connection to use request & error

                    controller->addSync(localFolderPath, nodeHandle, syncName, addSyncStep);
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
    if (mSelectedSyncRow == -1)
    {
        return;
    }

    ui->tSyncs->removeRow(mSelectedSyncRow);
    syncNames.removeAt(mSelectedSyncRow);

    saveSyncSettings();
}

void SettingsDialog::loadSyncSettings()
{
    ui->tSyncs->clearContents();
    syncNames.clear();

    ui->tSyncs->horizontalHeader()->setVisible(true);
    ui->tSyncs->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    int numFolders = model->getNumSyncedFolders();
    ui->tSyncs->setRowCount(numFolders);
    ui->tSyncs->setColumnCount(SYNC_COL_NB);
    ui->tSyncs->setColumnHidden(SYNC_COL_TAG, true);    //hidden tag
    ui->tSyncs->setColumnHidden(SYNC_COL_HANDLE, true); //hidden handle
    ui->tSyncs->setColumnHidden(SYNC_COL_NAME, true);   //hidden name
    ui->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_ENABLE_CB, QHeaderView::Fixed);
    ui->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_MENU, QHeaderView::Fixed);
    ui->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_LFOLDER,QHeaderView::Stretch);
    ui->tSyncs->horizontalHeader()->setSectionResizeMode(SYNC_COL_RFOLDER,QHeaderView::Stretch);
    int miniMumSectionSize (ui->tSyncs->horizontalHeader()->minimumSectionSize());
    ui->tSyncs->horizontalHeader()->resizeSection(SYNC_COL_ENABLE_CB, miniMumSectionSize);
    ui->tSyncs->horizontalHeader()->resizeSection(SYNC_COL_MENU, miniMumSectionSize);

    // New check up. Need to reset, syncs state could have changed
    areSyncsDisabled = false;

    for (int i = 0; i < numFolders; i++)
    {
        auto syncSetting = model->getSyncSetting(i);
        if (!syncSetting)
        {
            assert("A sync has been deleting while trying to loop in");
            continue;
        }

        // Check if current sync is disabled by an error.
        areSyncsDisabled = areSyncsDisabled || static_cast<bool>(syncSetting->getError());

        // Col 0: Enabled/Disabled checkbox
        QWidget* w = new QWidget();
        QHBoxLayout* hl = new QHBoxLayout();
        QCheckBox* c = new QCheckBox();
        w->setGeometry(0, 0, miniMumSectionSize, ui->tSyncs->verticalHeader()->minimumHeight());
        hl->setContentsMargins(0, 0, 0, 0);
        hl->addWidget(c);
        hl->setAlignment(c, Qt::AlignCenter);
        w->setLayout(hl);
        // Note: isEnabled refers to enable/disabled by the user. It could be temporary
        //       disabled or even failed. This should be shown in the UI.
        c->setChecked(syncSetting->isActive());
        c->setToolTip(tr("Enable / disable"));
        connect(c, &QCheckBox::stateChanged,
                this, &SettingsDialog::syncStateChanged, Qt::QueuedConnection);

        ui->tSyncs->setCellWidget(i, SYNC_COL_ENABLE_CB, w);

        // Col 1: Local folder
        QSyncItemWidget* localFolder = new QSyncItemWidget(QSyncItemWidget::LOCAL_FOLDER);
        QString localFolderQString = syncSetting->getLocalFolder();
#ifdef Q_OS_WINDOWS
        if (localFolderQString.startsWith(QString::fromUtf8("\\\\?\\")))
        {
            localFolderQString = localFolderQString.mid(4);
        }
#endif
        localFolder->setPath(localFolderQString, syncSetting->name());
        localFolder->setToolTip(localFolderQString);
        localFolder->setError(syncSetting->getError());
        ui->tSyncs->setCellWidget(i, SYNC_COL_LFOLDER, localFolder);

        // Col 2: Mega Folder
        QSyncItemWidget* megaFolder = new QSyncItemWidget(QSyncItemWidget::REMOTE_FOLDER);
        assert(syncSetting->getMegaFolder().size() && "remote folder lacks path");
        megaFolder->setPath(syncSetting->getMegaFolder().size() ?
                                syncSetting->getMegaFolder()
                              : QString::fromUtf8("---"));
        megaFolder->setToolTip(syncSetting->getMegaFolder());
        megaFolder->setSyncSetting(syncSetting);
        megaFolder->mSyncRootHandle = syncSetting->getMegaHandle();
        ui->tSyncs->setCellWidget(i, SYNC_COL_RFOLDER, megaFolder);

        // Col 3: menu
        QLabel* lMenu (new QLabel);
        QString menuRsc (QString::fromUtf8("://images/Item_options_rest.png"));
        lMenu->setStyleSheet(SYNCS_TAB_MENU_LABEL_QSS.arg(menuRsc));
        ui->tSyncs->setCellWidget(i, SYNC_COL_MENU, lMenu);

        // Col 4: tag. HIDDEN
        QLabel* lTag = new QLabel(QString::number(syncSetting->backupId()));
        ui->tSyncs->setCellWidget(i, SYNC_COL_TAG, lTag);

        // Col 5: MegaHandle. HIDDEN
        QLabel* lHandle = new QLabel(QString::number(syncSetting->getMegaHandle()));
        ui->tSyncs->setCellWidget(i, SYNC_COL_HANDLE, lHandle);

        // Col 6: SyncName. HIDDEN
        QLabel* lName = new QLabel(syncSetting->name());
        ui->tSyncs->setCellWidget(i, SYNC_COL_NAME, lName);

        syncNames.append(syncSetting->name());
    }

    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
}

void SettingsDialog::onDeleteSync()
{
    ui->tSyncs->removeRow(mSelectedSyncRow);
    syncNames.removeAt(mSelectedSyncRow);
    saveSyncSettings();
}

void SettingsDialog::showInFolderClicked()
{
    QWidget* w (ui->tSyncs->cellWidget(mSelectedSyncRow, SYNC_COL_LFOLDER));
    QString localFolderPath (qobject_cast<QSyncItemWidget*>(w)->fullPath());
    QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localFolderPath));
}

void SettingsDialog::showInMegaClicked()
{
    QWidget* w (ui->tSyncs->cellWidget(mSelectedSyncRow, SYNC_COL_RFOLDER));
    QString megaFolderPath (qobject_cast<QSyncItemWidget*>(w)->fullPath());
    MegaNode* node (megaApi->getNodeByPath(megaFolderPath.toUtf8().constData()));
    if (node)
    {
        const char* handle = node->getBase64Handle();
        QString url = QString::fromUtf8("mega://#fm/") + QString::fromUtf8(handle);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
        delete [] handle;
        delete node;
    }
}

void SettingsDialog::onCellClicked(int row, int column)
{
    if (column == SYNC_COL_MENU)
    {
        mSelectedSyncRow = row;

// TODO: activate this, but prevent row de-selection.
//        // Make sure the remote path is up to date
//        // queue an update of the sync remote node
//        mThreadPool->push([=]()
//        {//thread pool function
//            auto syncSetting = Model::instance()->getSyncSetting(row);

//            std::unique_ptr<char[]> np (MegaSyncApp->getMegaApi()->getNodePathByNodeHandle(
//                                            syncSetting->getMegaHandle()));
//            Model::instance()->updateMegaFolder(np ? QString::fromUtf8(np.get())
//                                                   : QString(),
//                                                syncSetting);
//        });// end of thread pool function

        QMenu menu(ui->tSyncs);
        // Show in explorer action
#ifdef Q_OS_MACOS
        QString openLocally{tr("Open in Finder")};
#else
        QString openLocally{tr("Open in Explorer")};
#endif
        auto showLocalAction (new MenuItemAction(openLocally,
                 QIcon(QString::fromUtf8("://images/show_in_folder_ico.png"))));
        connect(showLocalAction, &MenuItemAction::triggered,
                this, &SettingsDialog::showInFolderClicked);

        // Show in Mega action
        auto showRemoteAction (new MenuItemAction(tr("Open in MEGA"),
                                                  QIcon(QString::fromUtf8("://images/"
                                                                          "ico_open_MEGA.png"))));
        connect(showRemoteAction, &MenuItemAction::triggered,
                this, &SettingsDialog::showInMegaClicked);
        // Delete Sync action
        auto delAction (new MenuItemAction(tr("Remove Sync"),
                                           QIcon(QString::fromUtf8("://images/"
                                                                   "ico_Delete.png"))));
        connect(delAction, &MenuItemAction::triggered,
                this, &SettingsDialog::onDeleteSync, Qt::QueuedConnection);


        menu.addAction(showLocalAction);
        menu.addAction(showRemoteAction);
        menu.addSeparator();
        menu.addAction(delAction);

        QWidget* w (ui->tSyncs->cellWidget(row, column));
        menu.exec(w->mapToGlobal(w->rect().center()));
    }
}

#ifndef WIN32
void SettingsDialog::on_bPermissions_clicked()
{
    megaApi->setDefaultFolderPermissions(preferences->folderPermissionsValue());
    int folderPermissions = megaApi->getDefaultFolderPermissions();
    megaApi->setDefaultFilePermissions(preferences->filePermissionsValue());
    int filePermissions = megaApi->getDefaultFilePermissions();

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

    if (filePermissions != preferences->filePermissionsValue()
        || folderPermissions != preferences->folderPermissionsValue())
    {
        preferences->setFilePermissionsValue(filePermissions);
        preferences->setFolderPermissionsValue(folderPermissions);
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
    const bool dismissed{app->showSyncOverquotaDialog()};
    if(!dismissed)
    {
        return;
    }

    QStringList currentLocalFolders;
    QStringList currentMegaFoldersPaths;
    for (int i = 0; i < ui->tSyncs->rowCount(); i++)
    {
        //notice: this also takes into account !active ones
        QWidget* w (ui->tSyncs->cellWidget(i, SYNC_COL_LFOLDER));
        currentLocalFolders.append(static_cast<QSyncItemWidget*>(w)->fullPath());
        w = ui->tSyncs->cellWidget(i, SYNC_COL_RFOLDER);
        currentMegaFoldersPaths.append(static_cast<QSyncItemWidget*>(w)->fullPath());
    }

    QPointer<BindFolderDialog> dialog = new BindFolderDialog(app, syncNames, currentLocalFolders,
                                                             currentMegaFoldersPaths, this);
    if (megaFolderHandle > 0)
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
    MegaHandle handle = dialog->getMegaFolder();
    if (!localFolderPath.length() || !dialog->getMegaPath().size())
    {
        delete dialog;
        return;
    }

    int pos = ui->tSyncs->rowCount();
    ui->tSyncs->setRowCount(pos + 1);

    // Col 0: Enabled/Disabled checkbox
    QWidget* w = new QWidget();
    QHBoxLayout* hl = new QHBoxLayout();
    QCheckBox* c = new QCheckBox();
    w->setGeometry(0, 0, ui->tSyncs->horizontalHeader()->minimumWidth(),
                       ui->tSyncs->verticalHeader()->minimumHeight());
    hl->setContentsMargins(0, 0, 0, 0);
    hl->addWidget(c);
    hl->setAlignment(c, Qt::AlignCenter);
    w->setLayout(hl);

    c->setChecked(true);
    c->setToolTip(tr("Enable / disable"));
    connect(c, &QCheckBox::stateChanged,
            this, &SettingsDialog::syncStateChanged, Qt::QueuedConnection);
    ui->tSyncs->setCellWidget(pos, SYNC_COL_ENABLE_CB, w);

    // Col 1: Local folder
    QSyncItemWidget* localFolder = new QSyncItemWidget(QSyncItemWidget::LOCAL_FOLDER);
    localFolder->setPath(localFolderPath, dialog->getSyncName());
    localFolder->setToolTip(localFolderPath);
    ui->tSyncs->setCellWidget(pos, SYNC_COL_LFOLDER, localFolder);

    // Col 2: Mega Folder
    QSyncItemWidget* megaFolder = new QSyncItemWidget(QSyncItemWidget::REMOTE_FOLDER);
    //Check if need to setError here or it is enough setting when syncstatechanged
    megaFolder->setPath(dialog->getMegaPath());
    megaFolder->setToolTip(dialog->getMegaPath());
    ui->tSyncs->setCellWidget(pos, SYNC_COL_RFOLDER, megaFolder);

    // Col 3: menu
    QLabel* lMenu (new QLabel);
    QString menuRsc (QString::fromUtf8("://images/Item_options_rest.png"));
    lMenu->setStyleSheet(SYNCS_TAB_MENU_LABEL_QSS.arg(menuRsc));
    ui->tSyncs->setCellWidget(pos, SYNC_COL_MENU, lMenu);

    // Col 5: MegaHandle. HIDDEN
    QLabel* lHandle = new QLabel(QString::number(handle));
    ui->tSyncs->setCellWidget(pos, SYNC_COL_HANDLE, lHandle);

    // Col 6: SyncName. HIDDEN
    QLabel* lName = new QLabel(dialog->getSyncName());
    ui->tSyncs->setCellWidget(pos, SYNC_COL_NAME, lName);

    syncNames.append(dialog->getSyncName());

    delete dialog;

    saveSyncSettings();
}

void SettingsDialog::on_bAdd_clicked()
{
    const MegaHandle invalidMegaFolderHandle = 0;
    addSyncFolder(invalidMegaFolderHandle);
}

void SettingsDialog::on_bLogout_clicked()
{
    QPointer<SettingsDialog> currentDialog = this;
    if (QMegaMessageBox::question(nullptr, tr("Logout"),
            tr("Synchronization will stop working.") + QString::fromAscii(" ") + tr("Are you sure?"),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
    {
        if (currentDialog)
        {
            close();
            app->unlink();
        }
    }
}

void SettingsDialog::on_bRestart_clicked()
{
    QPointer<SettingsDialog> currentDialog = this;
    if (QMegaMessageBox::warning(nullptr, tr("Restart MEGAsync"), tr("Do you want to restart MEGAsync now?"),
                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        if (currentDialog)
        {
            // Restart MEGAsync
#if defined(Q_OS_MACX) || QT_VERSION < 0x050000
            ((MegaApplication*)qApp)->rebootApplication(false);
#else
            //we enqueue this call, so as not to close before properly handling the exit of Settings Dialog
            QTimer::singleShot(0, [] () {((MegaApplication*)qApp)->rebootApplication(false); });
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
    QString fileName = QFileDialog::getSaveFileName(0,
             tr("Export Master key"), dir.filePath(tr("MEGA-RECOVERYKEY")),
             QString::fromUtf8("Txt file (*.txt)"), NULL, QFileDialog::ShowDirsOnly
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
    out << megaApi->exportMasterKey();

    file.close();

    megaApi->masterKeyExported();

    QMegaMessageBox::information(this, tr("Warning"),
                                 tr("Exporting the master key and keeping it in a secure location enables you to set a new password without data loss.") + QString::fromUtf8("\n")
                                 + tr("Always keep physical control of your master key (e.g. on a client device, external storage, or print)."),
                                 QMessageBox::Ok);
}

void SettingsDialog::on_bChangePassword_clicked()
{
    QPointer<ChangePassword> cPassword = new ChangePassword(this);
    int result = cPassword->exec();
    if (!cPassword || result != QDialog::Accepted)
    {
        delete cPassword;
        return;
    }

    delete cPassword;
}

void SettingsDialog::on_tSyncs_doubleClicked(const QModelIndex& index)
{
    //FIXME: When using custom widget for row items, remove double check or use cellwidget to fix it.
    if (!index.column())
    {
        auto w (static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(index.row(),
                                                                     SYNC_COL_LFOLDER)));
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(w->fullPath()));
    }
    else
    {
        auto w (static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(index.row(),
                                                                     SYNC_COL_RFOLDER)));
        QString megaFolderPath = w->fullPath();
        MegaNode* node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
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
    QPointer<NodeSelector> nodeSelector = new NodeSelector(megaApi, NodeSelector::UPLOAD_SELECT, this);
    MegaNode *defaultNode = megaApi->getNodeByPath(ui->eUploadFolder->text().toUtf8().constData());
    if (defaultNode)
    {
        nodeSelector->setSelectedFolderHandle(defaultNode->getHandle());
        delete defaultNode;
    }

    nodeSelector->setDefaultUploadOption(hasDefaultUploadOption);
    nodeSelector->showDefaultUploadOption();
    int result = nodeSelector->exec();
    if (!nodeSelector || result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    MegaHandle selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *node = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if (!node)
    {
        delete nodeSelector;
        return;
    }

    const char *nPath = megaApi->getNodePath(node);
    if (!nPath)
    {
        delete nodeSelector;
        delete node;
        return;
    }

    QString newPath = QString::fromUtf8(nPath);
    if (newPath.compare(ui->eUploadFolder->text()) || hasDefaultUploadOption != nodeSelector->getDefaultUploadOption())
    {
        hasDefaultUploadOption = nodeSelector->getDefaultUploadOption();
        ui->eUploadFolder->setText(newPath);
    }

    delete nodeSelector;
    delete [] nPath;
    delete node;
}

void SettingsDialog::on_bDownloadFolder_clicked()
{
    QPointer<DownloadFromMegaDialog> dialog = new DownloadFromMegaDialog(preferences->downloadFolder(), this);
    dialog->setDefaultDownloadOption(hasDefaultDownloadOption);

    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    QString fPath = dialog->getPath();
    if (fPath.size() && (fPath.compare(ui->eDownloadFolder->text())
            || (hasDefaultDownloadOption != dialog->isDefaultDownloadOption())))
    {
        QTemporaryFile test(fPath + QDir::separator());
        if (!test.open())
        {
            QMegaMessageBox::critical(nullptr, tr("Error"), tr("You don't have write permissions in this local folder."));
            delete dialog;
            return;
        }

        hasDefaultDownloadOption = dialog->isDefaultDownloadOption();
        ui->eDownloadFolder->setText(fPath);
    }

    delete dialog;
}

void SettingsDialog::on_bAddName_clicked()
{
    QPointer<AddExclusionDialog> add = new AddExclusionDialog(this);
    int result = add->exec();
    if (!add || result != QDialog::Accepted)
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

    for (int i = 0; i < ui->lExcludedNames->count(); i++)
    {
        if (ui->lExcludedNames->item(i)->text() == text)
        {
            return;
        }
        else if (ui->lExcludedNames->item(i)->text().compare(text, Qt::CaseInsensitive) > 0)
        {
            ui->lExcludedNames->insertItem(i, text);
            saveExcludeSyncNames();
            return;
        }
    }

    ui->lExcludedNames->addItem(text);
    saveExcludeSyncNames();
}

void SettingsDialog::on_bDeleteName_clicked()
{
    QList<QListWidgetItem *> selected = ui->lExcludedNames->selectedItems();
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
    if (loadingSettings) return;
    bool enable (ui->cExcludeUpperThan->isChecked());
    preferences->setUpperSizeLimit(enable);
    preferences->setCrashed(true);
    ui->eUpperThan->setEnabled(enable);
    ui->cbExcludeUpperUnit->setEnabled(enable);
    ui->gExcludedFilesInfo->show();
    ui->bRestart->show();
}

void SettingsDialog::on_cExcludeLowerThan_clicked()
{
    if (loadingSettings) return;
    bool enable (ui->cExcludeLowerThan->isChecked());
    preferences->setLowerSizeLimit(enable);
    preferences->setCrashed(true);
    ui->eLowerThan->setEnabled(enable);
    ui->cbExcludeLowerUnit->setEnabled(enable);
    ui->gExcludedFilesInfo->show();
    ui->bRestart->show();
}

void SettingsDialog::on_eUpperThan_valueChanged(int i)
{
    if (loadingSettings) return;
    preferences->setUpperSizeLimitValue(i);
    preferences->setCrashed(true);
    ui->gExcludedFilesInfo->show();
    ui->bRestart->show();
}

void SettingsDialog::on_eLowerThan_valueChanged(int i)
{
    if (loadingSettings) return;
    preferences->setLowerSizeLimitValue(i);
    preferences->setCrashed(true);
    ui->gExcludedFilesInfo->show();
    ui->bRestart->show();
}

void SettingsDialog::on_cbExcludeUpperUnit_currentIndexChanged(int index)
{
    if (loadingSettings) return;
    preferences->setUpperSizeLimitUnit(index);
    preferences->setCrashed(true);
    ui->gExcludedFilesInfo->show();
    ui->bRestart->show();
}

void SettingsDialog::on_cbExcludeLowerUnit_currentIndexChanged(int index)
{
    if (loadingSettings) return;
    preferences->setLowerSizeLimitUnit(index);
    preferences->setCrashed(true);
    ui->gExcludedFilesInfo->show();
    ui->bRestart->show();
}

void SettingsDialog::on_cCacheSchedulerEnabled_toggled()
{
    if (loadingSettings) return;
    bool isEnabled = ui->cCacheSchedulerEnabled->isChecked();
    ui->sCacheSchedulerDays->setEnabled(isEnabled);
    preferences->setCleanerDaysLimit(isEnabled);
    if(isEnabled)
    {
        app->cleanLocalCaches();
    }
}

void SettingsDialog::on_sCacheSchedulerDays_valueChanged(int)
{
    if (loadingSettings) return;
    if(ui->cCacheSchedulerEnabled->isChecked())
    {
        preferences->setCleanerDaysLimitValue(ui->sCacheSchedulerDays->value());
        app->cleanLocalCaches();
    }
}

void SettingsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

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
    for (int i = 0; i < ui->lExcludedNames->count(); i++)
    {
        if (ui->lExcludedNames->item(i)->text().contains(QDir::separator())) // Path exclusion
            excludedPaths.append(ui->lExcludedNames->item(i)->text());
        else
            excludedNames.append(ui->lExcludedNames->item(i)->text()); // File name exclusion
    }

    preferences->setExcludedSyncNames(excludedNames);
    preferences->setExcludedSyncPaths(excludedPaths);
    preferences->setCrashed(true);

    ui->gExcludedFilesInfo->show();
    ui->bRestart->show();
}

void SettingsDialog::updateNetworkTab()
{
    int uploadLimitKB = preferences->uploadLimitKB();
    if (uploadLimitKB < 0)
    {
        ui->lUploadRateLimit->setText(tr("Auto"));
    }
    else if (uploadLimitKB > 0)
    {
        ui->lUploadRateLimit->setText(QStringLiteral("%1 KB/s").arg(uploadLimitKB));
    }
    else
    {
        ui->lUploadRateLimit->setText(tr("Don't limit"));
    }

    int downloadLimitKB = preferences->downloadLimitKB();
    if (downloadLimitKB > 0)
    {
        ui->lDownloadRateLimit->setText(QStringLiteral("%1 KB/s").arg(downloadLimitKB));
    }
    else
    {
        ui->lDownloadRateLimit->setText(tr("Don't limit"));
    }

    switch (preferences->proxyType())
    {
    case Preferences::PROXY_TYPE_NONE:
        ui->lProxySettings->setText(tr("No proxy"));
        break;
    case Preferences::PROXY_TYPE_AUTO:
        ui->lProxySettings->setText(tr("Auto"));
        break;
    case Preferences::PROXY_TYPE_CUSTOM:
        ui->lProxySettings->setText(tr("Manual"));
        break;
    }
}

void SettingsDialog::on_bClearCache_clicked()
{
    QString syncs;
    for (int i = 0; i < model->getNumSyncedFolders(); i++)
    {
        auto syncSetting = model->getSyncSetting(i);
        QFileInfo fi(syncSetting->getLocalFolder() + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
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

    warningDel->setText(tr("Backups of the previous versions of your synced files in your computer will be permanently deleted. "
                           "Please, check your backup folders to see if you need to rescue something before continuing:")
                           + QString::fromUtf8("<br/>") + syncs
                           + QString::fromUtf8("<br/><br/>") + tr("Do you want to delete your local backup now?"));
    warningDel->setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    warningDel->setDefaultButton(QMessageBox::No);
    int result = warningDel->exec();
    if (!warningDel || result != QMessageBox::Yes)
    {
        delete warningDel;
        return;
    }
    delete warningDel;

    QtConcurrent::run(deleteCache);
    cacheSize = 0;
    onCacheSizeAvailable();
}

void SettingsDialog::on_bClearRemoteCache_clicked()
{
    MegaNode *syncDebris = megaApi->getNodeByPath("//bin/SyncDebris");
    if (!syncDebris)
    {
        remoteCacheSize = 0;
        return;
    }

    QPointer<QMessageBox> warningDel = new QMessageBox(this);
    warningDel->setIcon(QMessageBox::Warning);
    warningDel->setWindowTitle(tr("Clear remote backup"));
    warningDel->setTextFormat(Qt::RichText);
    warningDel->setTextInteractionFlags(Qt::NoTextInteraction | Qt::LinksAccessibleByMouse);

    char *base64Handle = syncDebris->getBase64Handle();
    warningDel->setText(tr("Backups of the previous versions of your synced files in MEGA will be permanently deleted. "
                           "Please, check your [A] folder in the Rubbish Bin of your MEGA account to see if you need to rescue something before continuing.")
                           .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"mega://#fm/%1\">SyncDebris</a>").arg(QString::fromUtf8(base64Handle)))
                           + QString::fromUtf8("<br/><br/>") + tr("Do you want to delete your remote backup now?"));
    delete [] base64Handle;

    warningDel->setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    warningDel->setDefaultButton(QMessageBox::No);
    int result = warningDel->exec();
    if (!warningDel || result != QMessageBox::Yes)
    {
        delete warningDel;
        delete syncDebris;
        return;
    }
    delete warningDel;
    delete syncDebris;

    QtConcurrent::run(deleteRemoteCache, megaApi);
    remoteCacheSize = 0;
    onCacheSizeAvailable();
}

void SettingsDialog::on_bClearFileVersions_clicked()
{
    QPointer<SettingsDialog> dialog = QPointer<SettingsDialog>(this);
    if (QMegaMessageBox::warning(nullptr,
                             QString::fromUtf8("MEGAsync"),
                             tr("You are about to permanently remove all file versions. Would you like to proceed?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    megaApi->removeVersions(new MegaListenerFuncExecuter(true, [](MegaApi* api,  MegaRequest *request, MegaError *e)
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

void SettingsDialog::savingSyncs(bool completed, QObject *item)
{
    if (!item)
    {
        return;
    }

    for(auto *widget : item->findChildren<QWidget *>())
    {
        widget->setEnabled(completed);
    }

#ifndef Q_OS_MACOS
    ui->bGeneral->setEnabled(completed);
    ui->bAccount->setEnabled(completed);
    ui->bSyncs->setEnabled(completed);
    ui->bNetwork->setEnabled(completed);
    ui->bSecurity->setEnabled(completed);
    ui->bImports->setEnabled(completed);
#else
    toolBar->setEnableToolbarItems(completed);
#endif
}

void SettingsDialog::syncsStateInformation(int state)
{
    // If saving syncs are still in progress, wait the timeout for setting state widget
    if (isSavingSyncsOnGoing)
    {
        return;
    }

    switch (state)
    {
        case SAVING_SYNCS:
            ui->wSpinningIndicator->start();
            ui->sSyncsState->setCurrentWidget(ui->pSavingSyncs);
            break;
        default:
        {
            ui->wSpinningIndicator->stop();
            // If any sync is disabled, shows warning message
            if (areSyncsDisabled)
            {
                ui->sSyncsState->setCurrentWidget(ui->pSyncsDisabled);

#ifdef Q_OS_MACOS
                QString syncs(QString::fromUtf8("settings-syncs-error"));
                toolBar->customizeIconToolBarItem(bSyncs.get(), syncs);
#else
                ui->bSyncs->setIcon(QIcon(QString::fromUtf8(":/images/settings-syncs-warn.png")));
#endif
            }
            else
            {
                ui->sSyncsState->setCurrentWidget(ui->pNoErrors);

#ifdef Q_OS_MACOS
                QString syncs(QString::fromUtf8("settings-syncs"));
                toolBar->customizeIconToolBarItem(bSyncs.get(), syncs);
#else
                ui->bSyncs->setIcon(QIcon(QString::fromUtf8(":/images/settings-syncs.png")));
#endif
            }
        }
            break;
    }
}

void SettingsDialog::updateStorageElements()
{
    int accountType = preferences->accountType();

    auto totalStorage = static_cast<unsigned long long>(preferences->totalStorage());
    auto usedStorage = static_cast<unsigned long long>(preferences->usedStorage());
    if (totalStorage == 0)
    {
        ui->pStorageQuota->setValue(0);
        ui->lStorage->setText(tr("Data temporarily unavailable"));
        ui->bStorageDetails->setEnabled(false);
    }
    else
    {
        ui->bStorageDetails->setEnabled(true);

        if (accountType == Preferences::ACCOUNT_TYPE_BUSINESS)
        {
            ui->lStorage->setText(tr("%1 used")
                  .arg(Utilities::getSizeString(usedStorage)));
        }
        else
        {
            long double percentage = floor((100.0L*usedStorage)/totalStorage);
            int storagePercentage = static_cast<int>(percentage);
            ui->pStorageQuota->setValue(storagePercentage > ui->pStorageQuota->maximum() ? ui->pStorageQuota->maximum() : storagePercentage);
            ui->lStorage->setText(tr("%1 (%2%) of %3 used")
                  .arg(Utilities::getSizeString(usedStorage))
                  .arg(QString::number(storagePercentage))
                  .arg(Utilities::getSizeString(totalStorage)));
        }
    }
}

void SettingsDialog::updateBandwidthElements()
{
    int accountType = preferences->accountType();
    auto totalBandwidth = static_cast<unsigned long long>(preferences->totalBandwidth());
    auto usedBandwidth = static_cast<unsigned long long>(preferences->usedBandwidth());
    ui->lBandwidthFree->hide();

    if (accountType == Preferences::ACCOUNT_TYPE_FREE)
    {
        ui->lBandwidth->setText(tr("Used quota for the last %1 hours:")
                .arg(preferences->bandwidthInterval()));
        ui->lBandwidthFree->show();
        ui->lBandwidthFree->setText(Utilities::getSizeString(usedBandwidth));
    }
    else if (accountType == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->lBandwidth->setText(tr("%1 used")
              .arg(Utilities::getSizeString(usedBandwidth)));
    }
    else
    {
        if (totalBandwidth == 0)
        {
            ui->pTransferQuota->setValue(0);
            ui->lBandwidth->setText(tr("Data temporarily unavailable"));
        }
        else
        {
            long double percentage = floor((100.0L*usedBandwidth)/totalBandwidth);
            int bandwidthPercentage = static_cast<int>(percentage);
            ui->pTransferQuota->setValue(std::min(bandwidthPercentage, 100));
            ui->lBandwidth->setText(tr("%1 (%2%) of %3 used")
                    .arg(Utilities::getSizeString(usedBandwidth))
                    .arg(QString::number(std::min(bandwidthPercentage, 100)))
                    .arg(Utilities::getSizeString(totalBandwidth)));
        }
    }
}

void SettingsDialog::updateAccountElements()
{
    QIcon icon;
    switch(preferences->accountType())
    {
        case Preferences::ACCOUNT_TYPE_FREE:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Free.png"));
            ui->lAccountType->setText(tr("Free"));
            ui->bUpgrade->show();
            ui->pStorageQuota->show();
            ui->pTransferQuota->hide();
            break;
        case Preferences::ACCOUNT_TYPE_PROI:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_I.png"));
            ui->lAccountType->setText(tr("Pro I"));
            ui->bUpgrade->hide();
            ui->pStorageQuota->show();
            ui->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_PROII:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_II.png"));
            ui->lAccountType->setText(tr("Pro II"));
            ui->bUpgrade->hide();
            ui->pStorageQuota->show();
            ui->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_PROIII:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_III.png"));
            ui->lAccountType->setText(tr("Pro III"));
            ui->bUpgrade->hide();
            ui->pStorageQuota->show();
            ui->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_LITE:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Lite.png"));
            ui->lAccountType->setText(tr("Pro Lite"));
            ui->bUpgrade->hide();
            ui->pStorageQuota->show();
            ui->pTransferQuota->show();
            break;
        case Preferences::ACCOUNT_TYPE_BUSINESS:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/business.png"));
            ui->lAccountType->setText(tr("Business"));
            ui->bUpgrade->hide();
            ui->pStorageQuota->hide();
            ui->pTransferQuota->hide();
            break;
        default:
        // FIXME: is this correct?
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Small_Pro_I.png"));
            ui->lAccountType->setText(QString());
            ui->bUpgrade->hide();
            ui->pStorageQuota->show();
            ui->pTransferQuota->show();
            break;
    }

    ui->lAccountType->setIcon(icon);
}

void SettingsDialog::on_bUpdate_clicked()
{
    if (ui->bUpdate->text() == tr("Check for updates"))
    {
        app->checkForUpdates();
    }
    else
    {
        app->triggerInstallUpdate();
    }
}

void SettingsDialog::on_bFullCheck_clicked()
{
    preferences->setCrashed(true);
    QPointer<SettingsDialog> currentDialog = this;
    if (QMegaMessageBox::warning(nullptr, tr("Full scan"), tr("MEGAsync will perform a full scan of your synced folders when it starts.\n\nDo you want to restart MEGAsync now?"),
                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        if (currentDialog)
        {
            on_bRestart_clicked();
        }
    }
}

void SettingsDialog::setAvatar()
{
    const char *email = megaApi->getMyEmail();
    if (email)
    {
        ui->wAvatar->drawAvatarFromEmail(QString::fromUtf8(email));
        delete [] email;
    }
}

void SettingsDialog::on_bStorageDetails_clicked()
{
    accountDetailsDialog = new AccountDetailsDialog(this);
    app->updateUserStats(true, true, true, true, USERSTATS_STORAGECLICKED);
    QPointer<AccountDetailsDialog> dialog = accountDetailsDialog;
    dialog->exec();
    if (!dialog)
    {
        return;
    }

    delete accountDetailsDialog;
    accountDetailsDialog = NULL;
}

void SettingsDialog::setUpdateAvailable(bool updateAvailable)
{
    if (updateAvailable)
    {
        ui->bUpdate->setText(tr("Install update"));
    }
    else
    {
        ui->bUpdate->setText(tr("Check for updates"));
    }
}

void SettingsDialog::openSettingsTab(int tab)
{
    if(proxyOnly) // do not switch tabs when in guest mode
        return;

    switch (tab)
    {
    case GENERAL_TAB:
        reloadUIpage = true;
#ifndef Q_OS_MACOS
        ui->bGeneral->click();
#else
        toolBar->setSelectedItem(bGeneral.get());
        emit bGeneral.get()->activated();
#endif
        break;

    case ACCOUNT_TAB:
        reloadUIpage = true;
#ifndef Q_OS_MACOS
        ui->bAccount->click();
#else
        toolBar->setSelectedItem(bAccount.get());
        emit bAccount.get()->activated();
#endif
        break;

    case SYNCS_TAB:
#ifndef Q_OS_MACOS
        ui->bSyncs->click();
#else
        toolBar->setSelectedItem(bSyncs.get());
        emit bSyncs.get()->activated();
#endif
        break;

    case SECURITY_TAB:
#ifndef Q_OS_MACOS
        ui->bSecurity->click();
#else
        toolBar->setSelectedItem(bSecurity.get());
        emit bSecurity.get()->activated();
#endif
        break;

    case IMPORTS_TAB:
#ifndef Q_OS_MACOS
        ui->bImports->click();
#else
        toolBar->setSelectedItem(bImports.get());
        emit bImports.get()->activated();
#endif
        break;

    case NETWORK_TAB:
#ifndef Q_OS_MACOS
        ui->bNetwork->click();
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
    debugCounter++;
    if (debugCounter == 5)
    {
        app->toggleLogging();
        debugCounter = 0;
    }
}

void SettingsDialog::on_bSendBug_clicked()
{
    QPointer<BugReportDialog> dialog = new BugReportDialog(this, app->getLogger());
    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
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
    if (loadingSettings) return;
    preferences->setShowNotifications(checked);
}

void SettingsDialog::on_cAutoUpdate_toggled(bool checked)
{
    if (loadingSettings) return;
    if (ui->cAutoUpdate->isEnabled() && (checked != preferences->updateAutomatically()))
    {
        preferences->setUpdateAutomatically(checked);
        if (checked)
        {
            on_bUpdate_clicked();
        }
    }
}

void SettingsDialog::on_cStartOnStartup_toggled(bool checked)
{
    if (loadingSettings) return;
    if (!Platform::startOnStartup(checked))
    {
        // in case of failure - make sure configuration keeps the right value
        //LOG_debug << "Failed to " << (checked ? "enable" : "disable") << " MEGASync on startup.";
        preferences->setStartOnStartup(!checked);
    }
    else
    {
        preferences->setStartOnStartup(checked);
    }
}

void SettingsDialog::on_cLanguage_currentIndexChanged(int index)
{
    if (loadingSettings) return;
    if (index < 0) return; // QComboBox can emit with index -1; do nothing in that case
    QString selectedLanguage = languageCodes[index];
    if (preferences->language() != selectedLanguage)
    {
        preferences->setLanguage(selectedLanguage);
        app->changeLanguage(selectedLanguage);
        QString currentLanguage = app->getCurrentLanguageCode();
        mThreadPool->push([=]()
        {
            megaApi->setLanguage(currentLanguage.toUtf8().constData());
            megaApi->setLanguagePreference(currentLanguage.toUtf8().constData());
        });
    }
}

void SettingsDialog::on_eUploadFolder_textChanged(const QString &text)
{
    if (loadingSettings) return;
    MegaNode *node = megaApi->getNodeByPath(text.toUtf8().constData());
    if (node)
    {
        preferences->setHasDefaultUploadFolder(hasDefaultUploadOption);
        preferences->setUploadFolder(node->getHandle());
    }
    else
    {
        preferences->setHasDefaultUploadFolder(false);
        preferences->setUploadFolder(0);
    }
    delete node;
}

void SettingsDialog::on_eDownloadFolder_textChanged(const QString &text)
{
    if (loadingSettings) return;
    QString defaultDownloadPath = Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGAsync Downloads");
    if (text.compare(QDir::toNativeSeparators(defaultDownloadPath))
        || preferences->downloadFolder().size())
    {
        preferences->setDownloadFolder(text);
    }

    preferences->setHasDefaultDownloadFolder(hasDefaultDownloadOption);
}

void SettingsDialog::on_cFileVersioning_toggled(bool checked)
{
    if (loadingSettings) return;
    if (!checked)
    {
        auto answer = QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                                               tr("Disabling file versioning will prevent the creation and storage of new file versions. Do you want to continue?"),
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (answer == QMessageBox::No)
        {
            ui->cFileVersioning->blockSignals(true);
            ui->cFileVersioning->setChecked(true);
            ui->cFileVersioning->blockSignals(false);
            return;
        }
    }
    megaApi->setFileVersionsOption(!checked); // This is actually saved to Preferences after the megaApi call succeeds
}

void SettingsDialog::on_cOverlayIcons_toggled(bool checked)
{
    if (loadingSettings) return;
    preferences->disableOverlayIcons(!checked);
#ifdef Q_OS_MACOS
    Platform::notifyRestartSyncFolders();
#else
    for (int i = 0; i < model->getNumSyncedFolders(); i++)
    {
        auto syncSetting = model->getSyncSetting(i);
        app->notifyItemChange(syncSetting->getLocalFolder(), MegaApi::STATE_NONE);
    }
#endif
}

void SettingsDialog::on_bOpenProxySettings_clicked()
{
    ProxySettings *proxySettingsDialog = new ProxySettings(app, this);
    if (proxySettingsDialog->exec() == QDialog::Accepted)
    {
        app->applyProxySettings();
        updateNetworkTab();
    }
}

void SettingsDialog::on_bOpenBandwidthSettings_clicked()
{
    BandwidthSettings *bandwidthSettings = new BandwidthSettings(app, this);
    if (bandwidthSettings->exec() == QDialog::Rejected)
    {
        return;
    }

    app->setUploadLimit(std::max(preferences->uploadLimitKB(), 0));

    app->setMaxUploadSpeed(preferences->uploadLimitKB());
    app->setMaxDownloadSpeed(preferences->downloadLimitKB());

    app->setMaxConnections(MegaTransfer::TYPE_UPLOAD, preferences->parallelUploadConnections());
    app->setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());

    app->setUseHttpsOnly(preferences->usingHttpsOnly());

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
