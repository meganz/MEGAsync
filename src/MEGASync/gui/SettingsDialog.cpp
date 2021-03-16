#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QRect>
#include <QTranslator>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

#include "MegaApplication.h"
#include "SettingsDialog.h"
#include "QMegaMessageBox.h"
#include "ui_SettingsDialog.h"
#include "control/Utilities.h"
#include "platform/Platform.h"
#include "gui/AddExclusionDialog.h"
#include "gui/BugReportDialog.h"
#include "gui/QSyncItemWidget.h"
#include <assert.h>

#ifdef __APPLE__
    #include "gui/CocoaHelpButton.h"
#endif

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#else
#include "gui/PermissionsDialog.h"
#endif

using namespace mega;
using namespace std;

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
    this->controller = Controller::instance();
    this->model = Model::instance();

    connect(this->model, SIGNAL(syncStateChanged(std::shared_ptr<SyncSetting>)),
            this, SLOT(onSyncStateChanged(std::shared_ptr<SyncSetting>)));
    connect(this->model, SIGNAL(syncRemoved(std::shared_ptr<SyncSetting>)),
            this, SLOT(onSyncDeleted(std::shared_ptr<SyncSetting>)));

    mThreadPool = ThreadPoolSingleton::getInstance();

    syncsChanged = false;
    excludedNamesChanged = false;
    sizeLimitsChanged = false;
    cleanerLimitsChanged = false;
    fileVersioningChanged = false;
#ifndef WIN32
    filePermissions = 0;
    folderPermissions = 0;
    permissionsChanged = false;
#endif
    this->proxyOnly = proxyOnly;
    this->proxyTestProgressDialog = NULL;
    shouldClose = false;
    modifyingSettings = 0;
    accountDetailsDialog = NULL;
    cacheSize = -1;
    remoteCacheSize = -1;
    fileVersionsSize = preferences->logged() ? preferences->versionsStorage() : 0;

    reloadUIpage = false;
    hasUpperLimit = false;
    hasLowerLimit = false;
    upperLimit = 0;
    lowerLimit = 0;
    upperLimitUnit = Preferences::MEGA_BYTE_UNIT;
    lowerLimitUnit = Preferences::MEGA_BYTE_UNIT;
    debugCounter = 0;
    hasDaysLimit = false;
    daysLimit = 0;
    areSyncsDisabled = false;
    isSavingSyncsOnGoing = false;

    ui->eProxyPort->setValidator(new QIntValidator(0, 65535, this));
    ui->eUploadLimit->setValidator(new QIntValidator(0, 1000000000, this));
    ui->eDownloadLimit->setValidator(new QIntValidator(0, 1000000000, this));
    ui->eMaxDownloadConnections->setRange(1, 6);
    ui->eMaxUploadConnections->setRange(1, 6);

    ui->tNoHttp->viewport()->setCursor(Qt::ArrowCursor);
    downloadButtonGroup.addButton(ui->rDownloadLimit);
    downloadButtonGroup.addButton(ui->rDownloadNoLimit);
    uploadButtonGroup.addButton(ui->rUploadLimit);
    uploadButtonGroup.addButton(ui->rUploadNoLimit);
    uploadButtonGroup.addButton(ui->rUploadAutoLimit);

    ui->bAccount->setChecked(true);
    ui->wStack->setCurrentWidget(ui->pAccount);

    connect(ui->rNoProxy, SIGNAL(clicked()), this, SLOT(stateChanged()));
    connect(ui->rProxyAuto, SIGNAL(clicked()), this, SLOT(stateChanged()));
    connect(ui->rProxyManual, SIGNAL(clicked()), this, SLOT(proxyStateChanged()));
    connect(ui->eProxyUsername, SIGNAL(textChanged(QString)), this, SLOT(proxyStateChanged()));
    connect(ui->eProxyPassword, SIGNAL(textChanged(QString)), this, SLOT(proxyStateChanged()));
    connect(ui->eProxyServer, SIGNAL(textChanged(QString)), this, SLOT(proxyStateChanged()));
    connect(ui->eProxyPort, SIGNAL(textChanged(QString)), this, SLOT(proxyStateChanged()));
    connect(ui->cProxyType, SIGNAL(currentIndexChanged(int)), this, SLOT(proxyStateChanged()));
    connect(ui->cProxyRequiresPassword, SIGNAL(clicked()), this, SLOT(proxyStateChanged()));

    connect(ui->cStartOnStartup, SIGNAL(stateChanged(int)), this, SLOT(stateChanged()));
    connect(ui->cShowNotifications, SIGNAL(stateChanged(int)), this, SLOT(stateChanged()));
    connect(ui->cOverlayIcons, SIGNAL(stateChanged(int)), this, SLOT(stateChanged()));
    connect(ui->cAutoUpdate, SIGNAL(stateChanged(int)), this, SLOT(stateChanged()));
    connect(ui->cLanguage, SIGNAL(currentIndexChanged(int)), this, SLOT(stateChanged()));
    connect(ui->cDisableFileVersioning, SIGNAL(clicked(bool)), this, SLOT(fileVersioningStateChanged()));

    connect(ui->rDownloadNoLimit, SIGNAL(clicked()), this, SLOT(stateChanged()));
    connect(ui->rDownloadLimit, SIGNAL(clicked()), this, SLOT(stateChanged()));
    connect(ui->eDownloadLimit, SIGNAL(textChanged(QString)), this, SLOT(stateChanged()));
    connect(ui->rUploadAutoLimit, SIGNAL(clicked()), this, SLOT(stateChanged()));
    connect(ui->rUploadNoLimit, SIGNAL(clicked()), this, SLOT(stateChanged()));
    connect(ui->rUploadLimit, SIGNAL(clicked()), this, SLOT(stateChanged()));
    connect(ui->eUploadLimit, SIGNAL(textChanged(QString)), this, SLOT(stateChanged()));
    connect(ui->eMaxDownloadConnections, SIGNAL(valueChanged(int)), this, SLOT(stateChanged()));
    connect(ui->eMaxUploadConnections, SIGNAL(valueChanged(int)), this, SLOT(stateChanged()));
    connect(ui->cbUseHttps, SIGNAL(clicked()), this, SLOT(stateChanged()));

    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);

#ifndef WIN32
    #ifndef __APPLE__
    ui->rProxyAuto->hide();
    ui->cDisableIcons->hide();
    ui->cAutoUpdate->hide();
    ui->bUpdate->hide();
    #endif
#else
    connect(ui->cDisableIcons, SIGNAL(clicked()), this, SLOT(stateChanged()));
    ui->cDisableIcons->hide();

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
                ui->cDisableIcons->show();
            }
        }
    }
#endif
    ui->cProxyType->addItem(QString::fromUtf8("SOCKS5H"));

#ifdef __APPLE__
    this->setWindowTitle(tr("Preferences - MEGAsync"));
    ui->cStartOnStartup->setText(tr("Open at login"));
    if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_9) //FinderSync API support from 10.10+
    {
        ui->cOverlayIcons->hide();
    }

    CocoaHelpButton *helpButton = new CocoaHelpButton(this);
    ui->layoutBottom->insertWidget(0, helpButton);
    connect(helpButton, SIGNAL(clicked()), this, SLOT(on_bHelp_clicked()));
#endif

    if (!proxyOnly && preferences->logged())
    {
        connect(&cacheSizeWatcher, SIGNAL(finished()), this, SLOT(onLocalCacheSizeAvailable()));
        QFuture<long long> futureCacheSize = QtConcurrent::run(calculateCacheSize);
        cacheSizeWatcher.setFuture(futureCacheSize);

        connect(&remoteCacheSizeWatcher, SIGNAL(finished()), this, SLOT(onRemoteCacheSizeAvailable()));
        QFuture<long long> futureRemoteCacheSize = QtConcurrent::run(calculateRemoteCacheSize,megaApi);
        remoteCacheSizeWatcher.setFuture(futureRemoteCacheSize);
    }

#ifdef __APPLE__
    ui->bOk->hide();
    ui->bCancel->hide();

    const qreal ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
    if (ratio < 2)
    {
        ui->wTabHeader->setStyleSheet(QString::fromUtf8("#wTabHeader { border-image: url(\":/images/menu_header.png\"); }"));

        ui->bAccount->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
        ui->bBandwidth->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
        ui->bProxies->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
        ui->bSyncs->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
        ui->bAdvanced->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    }
    else
    {
        ui->wTabHeader->setStyleSheet(QString::fromUtf8("#wTabHeader { border-image: url(\":/images/menu_header@2x.png\"); }"));

        ui->bAccount->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected@2x.png\"); }"));
        ui->bBandwidth->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected@2x.png\"); }"));
        ui->bProxies->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected@2x.png\"); }"));
        ui->bSyncs->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected@2x.png\"); }"));
        ui->bAdvanced->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected@2x.png\"); }"));
    }

#else

    ui->wTabHeader->setStyleSheet(QString::fromUtf8("#wTabHeader { border-image: url(\":/images/menu_header.png\"); }"));
    ui->bAccount->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bBandwidth->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bProxies->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bSyncs->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bAdvanced->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
#endif

    ui->bLocalCleaner->setText(ui->bLocalCleaner->text().arg(QString::fromAscii(MEGA_DEBRIS_FOLDER)));

    ui->gCache->setVisible(false);
    ui->lFileVersionsSize->setVisible(false);
    ui->bClearFileVersions->setVisible(false);
    setProxyOnly(proxyOnly);
    ui->bOk->setDefault(true);

#ifdef __APPLE__
    minHeightAnimation = new QPropertyAnimation();
    maxHeightAnimation = new QPropertyAnimation();
    animationGroup = new QParallelAnimationGroup();
    animationGroup->addAnimation(minHeightAnimation);
    animationGroup->addAnimation(maxHeightAnimation);
    connect(animationGroup, SIGNAL(finished()), this, SLOT(onAnimationFinished()));

    ui->pAdvanced->hide();
    ui->pBandwidth->hide();
    ui->pSyncs->hide();

    if (!proxyOnly)
    {
        if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
        {
            setMinimumHeight(522);
            setMaximumHeight(522);
            ui->gStorageSpace->setMinimumHeight(83);
        }
        else
        {
            setMinimumHeight(542);
            setMaximumHeight(542);
            ui->gStorageSpace->setMinimumHeight(103);
        }

        ui->pProxies->hide();

        QSizePolicy sp_retain = ui->bApply->sizePolicy();
        sp_retain.setRetainSizeWhenHidden(true);
        ui->bApply->setSizePolicy(sp_retain);
        ui->bApply->hide();

    }
#endif

    highDpiResize.init(this);
    ((MegaApplication*)qApp)->attachStorageObserver(*this);
    ((MegaApplication*)qApp)->attachBandwidthObserver(*this);
    ((MegaApplication*)qApp)->attachAccountObserver(*this);

    connect(app, SIGNAL(storageStateChanged(int)), this, SLOT(storageStateChanged(int)));
    storageStateChanged(app->getAppliedStorageState());

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

    if (proxyOnly)
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
        ui->pProxies->show();

#ifdef __APPLE__
        setMinimumHeight(435);
        setMaximumHeight(435);
        ui->bApply->show();
#endif
    }
    else
    {
        ui->bAccount->setEnabled(true);
        ui->bAdvanced->setEnabled(true);
        ui->bSyncs->setEnabled(true);
        ui->bBandwidth->setEnabled(true);
    }
}

void SettingsDialog::setOverQuotaMode(bool mode)
{
    if (mode)
    {
        QString url = QString::fromUtf8("mega://#pro");
        Utilities::getPROurlWithParameters(url);
        ui->lOQWarning->setText(tr("Your MEGA account is full. All uploads are disabled, which may affect your synced folders. [A]Buy more space[/A]")
                                        .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"%1\"><span style=\"color:#d90007; text-decoration:none;\">").arg(url))
                                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>")));
        ui->wOQError->show();
    }
    else
    {
        ui->lOQWarning->setText(QString::fromUtf8(""));
        ui->wOQError->hide();
    }

    return;
}

void SettingsDialog::stateChanged()
{
    if (modifyingSettings)
    {
        return;
    }

#ifndef __APPLE__
    ui->bApply->setEnabled(true);
#else
    this->on_bApply_clicked();
#endif
}

void SettingsDialog::fileVersioningStateChanged()
{
    QPointer<SettingsDialog> dialog = QPointer<SettingsDialog>(this);
    if (ui->cDisableFileVersioning->isChecked() && (QMegaMessageBox::warning(nullptr,
                             QString::fromUtf8("MEGAsync"),
                             tr("Disabling file versioning will prevent the creation and storage of new file versions. Do you want to continue?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes
            || !dialog))
    {
        if (dialog)
        {
            ui->cDisableFileVersioning->setChecked(false);
        }
        return;
    }

    fileVersioningChanged = true;
    stateChanged();
}

void SettingsDialog::syncStateChanged(int state)
{
    if (state)
    {
        Platform::prepareForSync();
    }

    syncsChanged = true;
    stateChanged();
}

void SettingsDialog::onDisableSyncFailed(std::shared_ptr<SyncSetting> syncSetting)
{
    QMegaMessageBox::critical(nullptr, tr("Error"), tr("Unexpected error disabling sync %1").arg(syncSetting->name()));
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
           tr("Your sync \"%1\" can't be enabled. Reason: %2").arg(syncSetting->name())
          .arg(tr(MegaSync::getMegaSyncErrorCode(errorCode))));
        break;
    }
    }

    loadSettings(); //alt: look for item with syncSetting->tag & update enable/disable checkbox

}
void SettingsDialog::proxyStateChanged()
{
    if (modifyingSettings)
    {
        return;
    }

    ui->bApply->setEnabled(true);
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

void SettingsDialog::onSyncStateChanged(std::shared_ptr<SyncSetting>)
{
    loadSyncSettings();
}

void SettingsDialog::onSyncDeleted(std::shared_ptr<SyncSetting>)
{
    loadSyncSettings();
}

void SettingsDialog::onSavingSettingsProgress(double progress)
{
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

void SettingsDialog::storageChanged()
{
    onCacheSizeAvailable();
}

void SettingsDialog::storageStateChanged(int newStorageState)
{
     setOverQuotaMode(newStorageState == MegaApi::STORAGE_STATE_RED || newStorageState == MegaApi::STORAGE_STATE_PAYWALL);
}

void SettingsDialog::onCacheSizeAvailable()
{
    if (cacheSize != -1 && remoteCacheSize != -1)
    {
        if (!cacheSize && !remoteCacheSize && !fileVersionsSize)
        {
            return;
        }

        if (cacheSize)
        {
            ui->lCacheSize->setText(QString::fromUtf8(MEGA_DEBRIS_FOLDER) + QString::fromUtf8(": %1").arg(Utilities::getSizeString(cacheSize)));
            ui->gCache->setVisible(true);
        }
        else
        {
            //Hide and remove from layout to avoid  uneeded space
            ui->lCacheSize->hide();
            ui->bClearCache->hide();

            // Move remote SyncDebris widget to left side
            ui->gCache->layout()->removeWidget(ui->wLocalCache);
            ui->wRemoteCache->layout()->removeItem(ui->rSpacer);
#ifndef __APPLE__
            ui->lRemoteCacheSize->setMargin(2);
#endif
            ((QBoxLayout *)ui->gCache->layout())->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
        }

        if (remoteCacheSize)
        {
            ui->lRemoteCacheSize->setText(QString::fromUtf8("SyncDebris: %1").arg(Utilities::getSizeString(remoteCacheSize)));
            ui->gCache->setVisible(true);
        }
        else
        {
            //Hide and remove from layout to avoid  uneeded space
            ui->lRemoteCacheSize->hide();
            ui->bClearRemoteCache->hide();
        }

        fileVersionsSize = preferences->logged() ? preferences->versionsStorage() : 0;
        if (fileVersionsSize)
        {
            ui->lFileVersionsSize->setText(tr("File versions: %1").arg(Utilities::getSizeString(fileVersionsSize)));
            ui->lFileVersionsSize->setVisible(true);
            ui->bClearFileVersions->setVisible(true);
        }
        else
        {
            //Hide and remove from layout to avoid  uneeded space
            ui->lFileVersionsSize->hide();
            ui->bClearFileVersions->hide();
        }

    #ifdef __APPLE__
        if (ui->wStack->currentWidget() == ui->pAdvanced)
        {
            minHeightAnimation->setTargetObject(this);
            maxHeightAnimation->setTargetObject(this);
            minHeightAnimation->setPropertyName("minimumHeight");
            maxHeightAnimation->setPropertyName("maximumHeight");
            minHeightAnimation->setStartValue(minimumHeight());
            maxHeightAnimation->setStartValue(maximumHeight());
            minHeightAnimation->setEndValue(572);
            maxHeightAnimation->setEndValue(572);
            minHeightAnimation->setDuration(150);
            maxHeightAnimation->setDuration(150);
            animationGroup->start();
        }
    #endif
    }
}
void SettingsDialog::on_bAccount_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pAccount && !reloadUIpage)
    {
        ui->bAccount->setChecked(true);
        return;
    }

    reloadUIpage = false;

#ifdef __APPLE__
    ui->bApply->hide();
#endif

    ui->bAccount->setChecked(true);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pAccount);
    ui->bOk->setFocus();

#ifdef __APPLE__
    ui->pAccount->hide();
    ui->pAdvanced->hide();
    ui->pBandwidth->hide();
    ui->pProxies->hide();
    ui->pSyncs->hide();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(minimumHeight());
    maxHeightAnimation->setStartValue(maximumHeight());
    if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        minHeightAnimation->setEndValue(522);
        maxHeightAnimation->setEndValue(522);

        ui->gStorageSpace->setMinimumHeight(83);
    }
    else
    {
        minHeightAnimation->setEndValue(542);
        maxHeightAnimation->setEndValue(542);

        ui->gStorageSpace->setMinimumHeight(103);
    }
    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}

void SettingsDialog::on_bSyncs_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pSyncs)
    {
        ui->bSyncs->setChecked(true);
        return;
    }

#ifdef __APPLE__
    ui->bApply->hide();
#endif

    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(true);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pSyncs);
    ui->tSyncs->horizontalHeader()->setVisible( true );
    ui->bOk->setFocus();

#ifdef __APPLE__
    ui->pAccount->hide();
    ui->pAdvanced->hide();
    ui->pBandwidth->hide();
    ui->pProxies->hide();
    ui->pSyncs->hide();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(minimumHeight());
    maxHeightAnimation->setStartValue(maximumHeight());

    minHeightAnimation->setEndValue(420);
    maxHeightAnimation->setEndValue(420);
    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}

void SettingsDialog::on_bBandwidth_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pBandwidth)
    {
        ui->bBandwidth->setChecked(true);
        return;
    }

#ifdef __APPLE__
    ui->bApply->hide();
#endif

    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(true);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pBandwidth);
    ui->bOk->setFocus();

#ifdef __APPLE__
    ui->pAccount->hide();
    ui->pAdvanced->hide();
    ui->pBandwidth->hide();
    ui->pProxies->hide();
    ui->pSyncs->hide();

    int bwHeight;
    ui->gBandwidthQuota->show();
    ui->bSeparatorBandwidth->show();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(minimumHeight());
    maxHeightAnimation->setStartValue(maximumHeight());
    if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        minHeightAnimation->setEndValue(520);
        maxHeightAnimation->setEndValue(520);

        ui->gBandwidthQuota->setMinimumHeight(59);
    }
    else
    {
        minHeightAnimation->setEndValue(540);
        maxHeightAnimation->setEndValue(540);

        ui->gBandwidthQuota->setMinimumHeight(79);
    }
    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}

void SettingsDialog::on_bAdvanced_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pAdvanced)
    {
        ui->bAdvanced->setChecked(true);
        return;
    }

#ifdef __APPLE__
    ui->bApply->hide();
#endif

    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(true);
    ui->bProxies->setChecked(false);
    ui->wStack->setCurrentWidget(ui->pAdvanced);
    ui->bOk->setFocus();

#ifdef __APPLE__
    ui->pAccount->hide();
    ui->pAdvanced->hide();
    ui->pBandwidth->hide();
    ui->pProxies->hide();
    ui->pSyncs->hide();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(minimumHeight());
    maxHeightAnimation->setStartValue(maximumHeight());

    onCacheSizeAvailable();

    if (!cacheSize && !remoteCacheSize)
    {
        minHeightAnimation->setEndValue(595);
        maxHeightAnimation->setEndValue(595);
    }
    else
    {
        minHeightAnimation->setEndValue(640);
        maxHeightAnimation->setEndValue(640);
    }

    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}


void SettingsDialog::on_bProxies_clicked()
{
    emit userActivity();

    if (ui->wStack->currentWidget() == ui->pProxies)
    {
        ui->bProxies->setChecked(true);
        return;
    }

#ifdef __APPLE__
    ui->bApply->show();
#endif

    ui->bAccount->setChecked(false);
    ui->bSyncs->setChecked(false);
    ui->bBandwidth->setChecked(false);
    ui->bAdvanced->setChecked(false);
    ui->bProxies->setChecked(true);
    ui->wStack->setCurrentWidget(ui->pProxies);
    ui->bOk->setFocus();

#ifdef __APPLE__
    ui->pAccount->hide();
    ui->pAdvanced->hide();
    ui->pBandwidth->hide();
    ui->pProxies->hide();
    ui->pSyncs->hide();

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(minimumHeight());
    maxHeightAnimation->setStartValue(maximumHeight());
    minHeightAnimation->setEndValue(435);
    maxHeightAnimation->setEndValue(435);
    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}


void SettingsDialog::on_bCancel_clicked()
{
    this->close();
}

void SettingsDialog::on_bOk_clicked()
{
    bool saved = 1;
    if (ui->bApply->isEnabled())
    {
        saved = saveSettings();
    }

    if (saved == 1)
    {
        this->close();
    }
    else if (!saved)
    {
        shouldClose = true;
    }
}

void SettingsDialog::on_bHelp_clicked()
{
    QString helpUrl = Preferences::BASE_URL + QString::fromAscii("/help/client/megasync");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(helpUrl));
}

#ifndef __APPLE__
void SettingsDialog::on_bHelpIco_clicked()
{
    on_bHelp_clicked();
}
#endif

void SettingsDialog::on_rProxyManual_clicked()
{
    ui->cProxyType->setEnabled(true);
    ui->eProxyServer->setEnabled(true);
    ui->eProxyPort->setEnabled(true);
    ui->cProxyRequiresPassword->setEnabled(true);
    if (ui->cProxyRequiresPassword->isChecked())
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
    QString url = QString::fromUtf8("mega://#pro");
    Utilities::getPROurlWithParameters(url);
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
}

void SettingsDialog::on_bUpgradeBandwidth_clicked()
{
    on_bUpgrade_clicked();
}

void SettingsDialog::on_rUploadAutoLimit_clicked()
{
    ui->eUploadLimit->setEnabled(false);
}

void SettingsDialog::on_rUploadNoLimit_clicked()
{
    ui->eUploadLimit->setEnabled(false);
}

void SettingsDialog::on_rUploadLimit_clicked()
{
    ui->eUploadLimit->setEnabled(true);
}

void SettingsDialog::on_rDownloadNoLimit_clicked()
{
    ui->eDownloadLimit->setEnabled(false);
}

void SettingsDialog::on_rDownloadLimit_clicked()
{
    ui->eDownloadLimit->setEnabled(true);
}

void SettingsDialog::on_cProxyRequiresPassword_clicked()
{
    if (ui->cProxyRequiresPassword->isChecked())
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
    modifyingSettings++;

    if (!proxyOnly)
    {
        //General
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

        int width = ui->bBandwidth->width();
        QFont f = ui->bBandwidth->font();
        QFontMetrics fm = QFontMetrics(f);
        int neededWidth = fm.width(tr("Bandwidth"));
        if (width < neededWidth)
        {
            ui->bBandwidth->setText(tr("Transfers"));
        }

        if (ui->lUploadAutoLimit->text().trimmed().at(0)!=QChar::fromAscii('('))
        {
            ui->lUploadAutoLimit->setText(QString::fromAscii("(%1)").arg(ui->lUploadAutoLimit->text().trimmed()));
        }


        //Account
        ui->lEmail->setText(preferences->email());
        mThreadPool->push([=]()
        {//thread pool function

            char *email = megaApi->getMyEmail();
            if (email)
            {
                Utilities::queueFunctionInAppThread([=]()
                {//queued function

                    ui->lEmail->setText(QString::fromUtf8(email));
                    delete [] email;

                });//end of queued function
            }

        });// end of thread pool function




        // account type and details
        updateAccountElements();

        if (accountDetailsDialog)
        {
            accountDetailsDialog->refresh(preferences);
        }

        updateUploadFolder();

        updateDownloadFolder();

        //Syncs
        loadSyncSettings();
#ifdef _WIN32
        ui->cDisableIcons->setChecked(preferences->leftPaneIconsDisabled());
#endif

        //Bandwidth
        ui->rUploadAutoLimit->setChecked(preferences->uploadLimitKB()<0);
        ui->rUploadLimit->setChecked(preferences->uploadLimitKB()>0);
        ui->rUploadNoLimit->setChecked(preferences->uploadLimitKB()==0);
        ui->eUploadLimit->setText((preferences->uploadLimitKB()<=0)? QString::fromAscii("0") : QString::number(preferences->uploadLimitKB()));
        ui->eUploadLimit->setEnabled(ui->rUploadLimit->isChecked());

        ui->rDownloadLimit->setChecked(preferences->downloadLimitKB()>0);
        ui->rDownloadNoLimit->setChecked(preferences->downloadLimitKB()==0);
        ui->eDownloadLimit->setText((preferences->downloadLimitKB()<=0)? QString::fromAscii("0") : QString::number(preferences->downloadLimitKB()));
        ui->eDownloadLimit->setEnabled(ui->rDownloadLimit->isChecked());

        ui->eMaxDownloadConnections->setValue(preferences->parallelDownloadConnections());
        ui->eMaxUploadConnections->setValue(preferences->parallelUploadConnections());

        ui->cbUseHttps->setChecked(preferences->usingHttpsOnly());

        //Advanced
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

        loadSizeLimits();
        ui->cDisableFileVersioning->setChecked(preferences->fileVersioningDisabled());
        ui->cOverlayIcons->setChecked(preferences->overlayIconsDisabled());
    }

    if (!proxyTestProgressDialog)
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

        if (ui->rProxyManual->isChecked())
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

        if (ui->cProxyRequiresPassword->isChecked())
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
    modifyingSettings--;
}

void SettingsDialog::refreshAccountDetails() //TODO; separate storage from bandwidth
{
    if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->pStorage->hide();
        ui->pUsedBandwidth->hide();
    }
    else
    {
        ui->pStorage->show();
        ui->pUsedBandwidth->show();
    }

    if (preferences->totalStorage() == 0)
    {
        ui->pStorage->setValue(0);
        ui->lStorage->setText(tr("Data temporarily unavailable"));
        ui->bStorageDetails->setEnabled(false);
    }
    else
    {
        ui->bStorageDetails->setEnabled(true);

        if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
        {
            ui->lStorage->setText(tr("%1 used")
                  .arg(Utilities::getSizeString(preferences->usedStorage())));
        }
        else
        {
            int percentage = floor(100*((double)preferences->usedStorage()/preferences->totalStorage()));
            ui->pStorage->setValue(percentage > ui->pStorage->maximum() ? ui->pStorage->maximum() : percentage);
            ui->lStorage->setText(tr("%1 (%2%) of %3 used")
                  .arg(Utilities::getSizeString(preferences->usedStorage()))
                  .arg(QString::number(percentage))
                  .arg(Utilities::getSizeString(preferences->totalStorage())));
        }
    }

    int accType = preferences->accountType();
    if (accType == Preferences::ACCOUNT_TYPE_FREE) //Free user
    {
        ui->gBandwidthQuota->show();
        ui->bSeparatorBandwidth->show();
        ui->pUsedBandwidth->show();
        ui->pUsedBandwidth->setValue(0);
        ui->lBandwidth->setText(tr("Used quota for the last %1 hours: %2")
                .arg(preferences->bandwidthInterval())
                .arg(Utilities::getSizeString(preferences->usedBandwidth())));
    }
    else if (accType == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->gBandwidthQuota->show();
        ui->bSeparatorBandwidth->show();
        ui->pUsedBandwidth->hide();
        ui->lBandwidth->setText(tr("%1 used")
              .arg(Utilities::getSizeString(preferences->usedBandwidth())));
    }
    else
    {
        double totalBandwidth = preferences->totalBandwidth();
        if (totalBandwidth == 0)
        {
            ui->gBandwidthQuota->hide();
            ui->bSeparatorBandwidth->hide();
            ui->pUsedBandwidth->show();
            ui->pUsedBandwidth->setValue(0);
            ui->lBandwidth->setText(tr("Data temporarily unavailable"));
        }
        else
        {
            ui->gBandwidthQuota->show();
            ui->bSeparatorBandwidth->show();
            int bandwidthPercentage = floor(100*((double)preferences->usedBandwidth()/preferences->totalBandwidth()));
            ui->pUsedBandwidth->show();
            ui->pUsedBandwidth->setValue((bandwidthPercentage < 100) ? bandwidthPercentage : 100);
            ui->lBandwidth->setText(tr("%1 (%2%) of %3 used")
                    .arg(Utilities::getSizeString(preferences->usedBandwidth()))
                    .arg(QString::number((bandwidthPercentage < 100) ? bandwidthPercentage : 100))
                    .arg(Utilities::getSizeString(preferences->totalBandwidth())));
        }
    }
}

int SettingsDialog::saveSettings()
{
    saveSettingsProgress.reset(new ProgressHelper(false, tr("Saving settings")));
    connect(saveSettingsProgress.get(), SIGNAL(progress(double)), this, SLOT(onSavingSettingsProgress(double)));
    connect(saveSettingsProgress.get(), SIGNAL(completed()), this, SLOT(onSavingSettingsCompleted()));

    // Uncomment the following to see a progress bar when saving settings (which being modal will prevent from modifying while changing)
    //Utilities::showProgressDialog(saveSettingsProgress.get(), this);

    ProgressHelperCompletionGuard g(saveSettingsProgress.get());

    modifyingSettings++;
    if (!proxyOnly)
    {
        //General
        preferences->setShowNotifications(ui->cShowNotifications->isChecked());

        if (ui->cAutoUpdate->isEnabled())
        {
            bool updateAutomatically = ui->cAutoUpdate->isChecked();
            if (updateAutomatically != preferences->updateAutomatically())
            {
                preferences->setUpdateAutomatically(updateAutomatically);
                if (updateAutomatically)
                {
                    on_bUpdate_clicked();
                }
            }
        }

        bool startOnStartup = ui->cStartOnStartup->isChecked();
        if (!Platform::startOnStartup(startOnStartup))
        {
            // in case of failure - make sure configuration keeps the right value
            //LOG_debug << "Failed to " << (startOnStartup ? "enable" : "disable") << " MEGASync on startup.";
            preferences->setStartOnStartup(!startOnStartup);
        }
        else
        {
            preferences->setStartOnStartup(startOnStartup);
        }

        //Language
        int currentIndex = ui->cLanguage->currentIndex();
        QString selectedLanguageCode = languageCodes[currentIndex];
        if (preferences->language() != selectedLanguageCode)
        {
            preferences->setLanguage(selectedLanguageCode);
            app->changeLanguage(selectedLanguageCode);
            QString currentLanguageCode = app->getCurrentLanguageCode();
            mThreadPool->push([=]()
            {
                megaApi->setLanguage(currentLanguageCode.toUtf8().constData());
                megaApi->setLanguagePreference(currentLanguageCode.toUtf8().constData());
            });

        }

        //Account
        MegaNode *node = megaApi->getNodeByPath(ui->eUploadFolder->text().toUtf8().constData());
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

        QString defaultDownloadPath = Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGAsync Downloads");
        if (ui->eDownloadFolder->text().compare(QDir::toNativeSeparators(defaultDownloadPath))
                || preferences->downloadFolder().size())
        {
            preferences->setDownloadFolder(ui->eDownloadFolder->text());
        }

        preferences->setHasDefaultDownloadFolder(hasDefaultDownloadOption);

        //Syncs
        if (syncsChanged)
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
                    auto tagItem = ui->tSyncs->cellWidget(j,3);
                    if (tagItem && static_cast<QLabel *>(tagItem)->text().toULongLong() == syncSetting->backupId())
                    {
                        found = true;
                        break;
                    }
                }
                if (!found) //sync no longer found in settings: needs removing
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Removing sync: %1").arg(syncSetting->name()).toUtf8().constData());
                    ActionProgress *removeSyncStep = new ActionProgress(true, QString::fromUtf8("Removing sync: %1 - %2")
                                                                        .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()));
                    saveSettingsProgress->addStep(removeSyncStep);
                    controller->removeSync(syncSetting, removeSyncStep);
                }

                // 1.2 - enable/disable changed syncs
                QString localPath = syncSetting->getLocalFolder();
                QString megaPath = syncSetting->getMegaFolder();
                for (int j = 0; j < ui->tSyncs->rowCount(); j++)
                {
                    QString newLocalPath = static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(j, 0))->fullPath();
                    QString newMegaPath = static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(j, 1))->fullPath();
                    bool enabled = ((QCheckBox *)ui->tSyncs->cellWidget(j, 2))->isChecked();
                    bool disabled = !enabled;
#ifdef SYNC_ADVANCED_TEST_MODE
                    enabled = static_cast<QCheckBox*>(ui->tSyncs->cellWidget(j, 2))->checkState() == Qt::Checked;
                    disabled = static_cast<QCheckBox*>(ui->tSyncs->cellWidget(j, 2))->checkState() == Qt::Unchecked;
#endif

                    auto tagItem = ui->tSyncs->cellWidget(j,3);

                    if (tagItem && static_cast<QLabel *>(tagItem)->text().toULongLong() == syncSetting->backupId())
                    {
#ifdef SYNC_ADVANCED_TEST_MODE
                        if (disabled && syncSetting->isEnabled()) //sync disabled
#else
                        if (disabled && syncSetting->isActive()) //sync disabled
#endif
                        {
                            ActionProgress *disableSyncStep = new ActionProgress(true, QString::fromUtf8("Disabling sync: %1 - %2")
                                                                                .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()));
                            saveSettingsProgress->addStep(disableSyncStep);

                            connect(disableSyncStep, &ActionProgress::failedRequest, this, [this, syncSetting](MegaRequest *request, MegaError *error)
                            {
                                if (error->getErrorCode())
                                {
                                    QObject temporary;
                                    QObject::connect(&temporary, &QObject::destroyed, this, [this, syncSetting](){
                                        onDisableSyncFailed(syncSetting); //Note: this might get executed before onSyncStateChanged!
                                        loadSyncSettings(); //to no longer show the failed line
                                    }, Qt::QueuedConnection);
                                }
                            }, Qt::DirectConnection); //Note, we need direct connection to use request & error

                            controller->disableSync(syncSetting, disableSyncStep);
                        }
#ifdef SYNC_ADVANCED_TEST_MODE
                        else if (enabled && !syncSetting->isActive()) //sync re-enabled!
#else
                        else if (enabled && !syncSetting->isActive()) //sync re-enabled!
#endif
                        {
                            ActionProgress *enableSyncStep = new ActionProgress(true, QString::fromUtf8("Enabling sync: %1 - %2")
                                                                                .arg(syncSetting->getLocalFolder()).arg(syncSetting->getMegaFolder()));
                            saveSettingsProgress->addStep(enableSyncStep);
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
                auto tagItem = ui->tSyncs->cellWidget(j, 3);
                if (!tagItem) //not found: new sync
                {
                    bool enabled = ((QCheckBox *)ui->tSyncs->cellWidget(j, 2))->isChecked();
                    if (enabled)
                    {
                        QString localFolderPath = static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(j, 0))->fullPath();
                        QString megaFolderPath = static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(j, 1))->fullPath();
                        MegaHandle nodeHandle = static_cast<QLabel *>(ui->tSyncs->cellWidget(j, 4))->text().toULongLong();
                        QString syncName =static_cast<QLabel *>(ui->tSyncs->cellWidget(j, 5))->text();

                        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Adding sync from Settings: %1 - %2")
                                     .arg(localFolderPath).arg(megaFolderPath).toUtf8().constData());


                        ActionProgress *addSyncStep = new ActionProgress(true, QString::fromUtf8("Adding sync: %1 - %2")
                                                                         .arg(localFolderPath).arg(megaFolderPath));
                        saveSettingsProgress->addStep(addSyncStep);

                        //Connect failing signals
                        connect(addSyncStep, &ActionProgress::failed, this, [this, localFolderPath, megaFolderPath](int errorCode)
                        {
                            static_cast<MegaApplication *>(qApp)->showAddSyncError(errorCode, localFolderPath, megaFolderPath);
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
                                    static_cast<MegaApplication *>(qApp)->showAddSyncError(reqCopy, errCopy, localFolderPath, megaFolderPath);
                                    loadSyncSettings();  //to no longer show the failed line

                                    delete reqCopy;
                                    delete errCopy;
                                    //(syncSettings might have some old values), that's why we don't use syncSetting->getError.
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

            syncsChanged = false;
        }
#ifdef _WIN32
        bool iconsDisabled = ui->cDisableIcons->isChecked();
        if (preferences->leftPaneIconsDisabled() != iconsDisabled)
        {
            if (iconsDisabled)
            {
                Platform::removeAllSyncsFromLeftPane();
            }
            else
            {
                for (int i = 0; i < model->getNumSyncedFolders(); i++)
                {
                    auto syncSetting = model->getSyncSetting(i);
                    Platform::addSyncToLeftPane(syncSetting->getLocalFolder(),
                                                syncSetting->name(),
                                                syncSetting->getSyncID());
                }
            }
            preferences->disableLeftPaneIcons(iconsDisabled);
        }
#endif

#ifndef WIN32
        if (permissionsChanged)
        {
            megaApi->setDefaultFilePermissions(filePermissions);
            filePermissions = megaApi->getDefaultFilePermissions();
            preferences->setFilePermissionsValue(filePermissions);

            megaApi->setDefaultFolderPermissions(folderPermissions);
            folderPermissions = megaApi->getDefaultFolderPermissions();
            preferences->setFolderPermissionsValue(folderPermissions);

            permissionsChanged = false;
            filePermissions   = 0;
            folderPermissions = 0;
        }
#endif

        //Bandwidth
        if (ui->rUploadLimit->isChecked())
        {
            preferences->setUploadLimitKB(ui->eUploadLimit->text().trimmed().toInt());
            app->setUploadLimit(0);
        }
        else
        {
            if (ui->rUploadNoLimit->isChecked())
            {
                preferences->setUploadLimitKB(0);
            }
            else if (ui->rUploadAutoLimit->isChecked())
            {
                preferences->setUploadLimitKB(-1);
            }

            app->setUploadLimit(preferences->uploadLimitKB());
        }
        app->setMaxUploadSpeed(preferences->uploadLimitKB());

        if (ui->rDownloadNoLimit->isChecked())
        {
            preferences->setDownloadLimitKB(0);
        }
        else
        {
            preferences->setDownloadLimitKB(ui->eDownloadLimit->text().trimmed().toInt());
        }

        app->setMaxDownloadSpeed(preferences->downloadLimitKB());

        if (ui->eMaxDownloadConnections->value() != preferences->parallelDownloadConnections())
        {
            preferences->setParallelDownloadConnections(ui->eMaxDownloadConnections->value());
            app->setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());
        }

        if (ui->eMaxUploadConnections->value() != preferences->parallelUploadConnections())
        {
            preferences->setParallelUploadConnections(ui->eMaxUploadConnections->value());
            app->setMaxConnections(MegaTransfer::TYPE_UPLOAD, preferences->parallelUploadConnections());
        }

        preferences->setUseHttpsOnly(ui->cbUseHttps->isChecked());
        app->setUseHttpsOnly(preferences->usingHttpsOnly());

        if (sizeLimitsChanged)
        {
            preferences->setUpperSizeLimit(hasUpperLimit);
            preferences->setLowerSizeLimit(hasLowerLimit);
            preferences->setUpperSizeLimitValue(upperLimit);
            preferences->setLowerSizeLimitValue(lowerLimit);
            preferences->setUpperSizeLimitUnit(upperLimitUnit);
            preferences->setLowerSizeLimitUnit(lowerLimitUnit);
            preferences->setCrashed(true);
            QMegaMessageBox::information(this, tr("Warning"),
                                         tr("The new excluded file sizes will be taken into account when the application starts again."),
                                         QMessageBox::Ok);
            sizeLimitsChanged = false;
        }

        if (cleanerLimitsChanged)
        {
            preferences->setCleanerDaysLimit(hasDaysLimit);
            preferences->setCleanerDaysLimitValue(daysLimit);
            app->cleanLocalCaches();
            cleanerLimitsChanged = false;
        }

        if (fileVersioningChanged && ui->cDisableFileVersioning->isChecked() != preferences->fileVersioningDisabled())
        {
            megaApi->setFileVersionsOption(ui->cDisableFileVersioning->isChecked());
            fileVersioningChanged = false;
        }

        if (ui->cOverlayIcons->isChecked() != preferences->overlayIconsDisabled())
        {
            preferences->disableOverlayIcons(ui->cOverlayIcons->isChecked());
            #ifdef Q_OS_MACX
            Platform::notifyRestartSyncFolders();
            #else
            for (int i = 0; i < model->getNumSyncedFolders(); i++)
            {
                auto syncSetting = model->getSyncSetting(i);
                app->notifyItemChange(syncSetting->getLocalFolder(), MegaApi::STATE_NONE);
            }
            #endif
        }
    }

    int proxyChanged = 0;
    //Proxies
    if (!proxyTestProgressDialog && ((ui->rNoProxy->isChecked() && (preferences->proxyType() != Preferences::PROXY_TYPE_NONE))       ||
        (ui->rProxyAuto->isChecked() &&  (preferences->proxyType() != Preferences::PROXY_TYPE_AUTO))    ||
        (ui->rProxyManual->isChecked() &&  (preferences->proxyType() != Preferences::PROXY_TYPE_CUSTOM))||
        (preferences->proxyProtocol() != ui->cProxyType->currentIndex())                                ||
        (preferences->proxyServer() != ui->eProxyServer->text().trimmed())                              ||
        (preferences->proxyPort() != ui->eProxyPort->text().toInt())                                    ||
        (preferences->proxyRequiresAuth() != ui->cProxyRequiresPassword->isChecked())                   ||
        (preferences->getProxyUsername() != ui->eProxyUsername->text())                                 ||
        (preferences->getProxyPassword() != ui->eProxyPassword->text())))
    {
        proxyChanged = 1;
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::NoProxy);
        if (ui->rProxyManual->isChecked())
        {
            switch(ui->cProxyType->currentIndex())
            {
            case Preferences::PROXY_PROTOCOL_SOCKS5H:
                proxy.setType(QNetworkProxy::Socks5Proxy);
                break;
            default:
                proxy.setType(QNetworkProxy::HttpProxy);
                break;
            }

            proxy.setHostName(ui->eProxyServer->text().trimmed());
            proxy.setPort(ui->eProxyPort->text().trimmed().toInt());
            if (ui->cProxyRequiresPassword->isChecked())
            {
                proxy.setUser(ui->eProxyUsername->text());
                proxy.setPassword(ui->eProxyPassword->text());
            }
        }
        else if (ui->rProxyAuto->isChecked())
        {
            MegaProxy *proxySettings = megaApi->getAutoProxySettings();
            if (proxySettings->getProxyType()==MegaProxy::PROXY_CUSTOM)
            {
                string sProxyURL = proxySettings->getProxyURL();
                QString proxyURL = QString::fromUtf8(sProxyURL.data());

                QStringList parts = proxyURL.split(QString::fromAscii("://"));
                if (parts.size() == 2 && parts[0].startsWith(QString::fromUtf8("socks")))
                {
                    proxy.setType(QNetworkProxy::Socks5Proxy);
                }
                else
                {
                    proxy.setType(QNetworkProxy::HttpProxy);
                }

                QStringList arguments = parts[parts.size()-1].split(QString::fromAscii(":"));
                if (arguments.size() == 2)
                {
                    proxy.setHostName(arguments[0]);
                    proxy.setPort(arguments[1].toInt());
                }
            }
            delete proxySettings;
        }

        proxyTestProgressDialog = new MegaProgressCustomDialog(this, 0, 0);
        proxyTestProgressDialog->setWindowModality(Qt::WindowModal);
        proxyTestProgressDialog->show();

        ConnectivityChecker *connectivityChecker = new ConnectivityChecker(Preferences::PROXY_TEST_URL);
        connectivityChecker->setProxy(proxy);
        connectivityChecker->setTestString(Preferences::PROXY_TEST_SUBSTRING);
        connectivityChecker->setTimeout(Preferences::PROXY_TEST_TIMEOUT_MS);

        connect(connectivityChecker, SIGNAL(testError()), this, SLOT(onProxyTestError()));
        connect(connectivityChecker, SIGNAL(testSuccess()), this, SLOT(onProxyTestSuccess()));
        connect(connectivityChecker, SIGNAL(testFinished()), connectivityChecker, SLOT(deleteLater()));

        connectivityChecker->startCheck();
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Testing proxy settings...");
    }

    //Advanced
    if (excludedNamesChanged)
    {
        QStringList excludedNames;
        QStringList excludedPaths;
        for (int i = 0; i < ui->lExcludedNames->count(); i++)
        {
            if (ui->lExcludedNames->item(i)->text().contains(QDir::separator())) // Path exclusion
            {
                excludedPaths.append(ui->lExcludedNames->item(i)->text());
            }
            else
            {
                excludedNames.append(ui->lExcludedNames->item(i)->text()); // File name exclusion
            }
        }

        preferences->setExcludedSyncNames(excludedNames);
        preferences->setExcludedSyncPaths(excludedPaths);
        preferences->setCrashed(true);
        excludedNamesChanged = false;

        QMessageBox* info = new QMessageBox(QMessageBox::Warning, QString::fromAscii("MEGAsync"),
                                            tr("The new excluded file names will be taken into account\n"
                                               "when the application starts again"));
        info->setStandardButtons(QMessageBox::Ok | QMessageBox::Yes);
        info->setButtonText(QMessageBox::Yes, tr("Restart"));
        info->setDefaultButton(QMessageBox::Ok);

        QPointer<SettingsDialog> currentDialog = this;
        info->exec();
        int result = info->result();
        delete info;
        if (!currentDialog)
        {
            return 2;
        }

        if (result == QMessageBox::Yes)
        {
            // Restart MEGAsync
#if defined(Q_OS_MACX) || QT_VERSION < 0x050000
            ((MegaApplication*)qApp)->rebootApplication(false);
#else
            QTimer::singleShot(0, [] () {((MegaApplication*)qApp)->rebootApplication(false); }); //we enqueue this call, so as not to close before properly handling the exit of Settings Dialog
#endif
            return 2;
        }

        QT_TR_NOOP("Do you want to restart MEGAsync now?");
    }

    ui->bApply->setEnabled(false);
    modifyingSettings--;
    return !proxyChanged;
}

void SettingsDialog::on_bDelete_clicked()
{
    QList<QTableWidgetSelectionRange> selected = ui->tSyncs->selectedRanges();
    if (selected.size() == 0)
    {
        return;
    }

    int index = selected.first().topRow();
    ui->tSyncs->removeRow(index);
    syncNames.removeAt(index);

    syncsChanged = true;
    stateChanged();
}

void SettingsDialog::loadSyncSettings()
{
    ui->tSyncs->clearContents();
    syncNames.clear();

    ui->tSyncs->horizontalHeader()->setVisible(true);
    int numFolders = model->getNumSyncedFolders();
    ui->tSyncs->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tSyncs->setRowCount(numFolders);
    ui->tSyncs->setColumnCount(6);
    ui->tSyncs->setColumnWidth(2, 21);
    ui->tSyncs->setColumnHidden(3, true); //hidden tag
    ui->tSyncs->setColumnHidden(4, true); //hidden handle
    ui->tSyncs->setColumnHidden(5, true); //hidden name

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

        QSyncItemWidget *localFolder = new QSyncItemWidget();
        QString localFolderQString = syncSetting->getLocalFolder();
#ifdef WIN32
if (localFolderQString.startsWith(QString::fromAscii("\\\\?\\")))
{
    localFolderQString = localFolderQString.mid(4);
}
#endif
        // Check if current sync is disabled by an error.
        areSyncsDisabled = areSyncsDisabled || static_cast<bool>(syncSetting->getError());

        // Col 1: Local folder
        localFolder->setPathAndName(localFolderQString, syncSetting->name());
        localFolder->setToolTip(localFolderQString);
        localFolder->setError(syncSetting->getError());
        ui->tSyncs->setCellWidget(i, 0, localFolder);

        // Col 2: Mega Folder
        QSyncItemWidget *megaFolder = new QSyncItemWidget();
        assert(syncSetting->getMegaFolder().size() && "remote folder lacks path");
        megaFolder->setPathAndGuessName(syncSetting->getMegaFolder().size()?syncSetting->getMegaFolder():QString::fromUtf8("---"));
        megaFolder->setToolTip(syncSetting->getMegaFolder());
        megaFolder->setSyncSetting(syncSetting);
        megaFolder->mSyncRootHandle = syncSetting->getMegaHandle();
        ui->tSyncs->setCellWidget(i, 1, megaFolder);

        // Col 3: Enabled/Disabled checkbox
        QCheckBox *c = new QCheckBox();
        c->setChecked(syncSetting->isActive()); //note: isEnabled refers to enable/disabled by the user. It could be temporary disabled or even failed. This should be shown in the UI
        c->setToolTip(tr("Enable / disable"));

#ifdef SYNC_ADVANCED_TEST_MODE
        if (syncSetting->isEnabled() && !syncSetting->isActive())
        {
            c->setCheckState(Qt::PartiallyChecked);
        }

        if (syncSetting->isActive())
        {
            localFolder->setTextColor(QColor::fromRgb(0, 255,0));
        }
        else
        {
            localFolder->setTextColor(QColor::fromRgb(255, 0,0));
        }

        if (syncSetting->isTemporaryDisabled())
        {
            megaFolder->setTextColor(QColor::fromRgb(125, 125, 125));
        }

        if (syncSetting->getState() == MegaSync::SYNC_FAILED)
        {
            megaFolder->setTextColor(QColor::fromRgb(255, 0, 0));
        }
#endif
        connect(c, SIGNAL(stateChanged(int)), this, SLOT(syncStateChanged(int)),Qt::QueuedConnection);

        ui->tSyncs->setCellWidget(i, 2, c);

        // Col 4: tag. HIDDEN
        QLabel *lTag = new QLabel();
        lTag->setText(QString::number(syncSetting->backupId()));
        ui->tSyncs->setCellWidget(i, 3, lTag);

        // Col 5: MegaHandle. HIDDEN
        QLabel *lHandle = new QLabel();
        lHandle->setText(QString::number(syncSetting->getMegaHandle()));
        ui->tSyncs->setCellWidget(i, 4, lHandle);

        // Col 6: SyncName. HIDDEN
        QLabel *lName = new QLabel();
        lName->setText(syncSetting->name());
        ui->tSyncs->setCellWidget(i, 5, lName);

        syncNames.append(syncSetting->name());
    }

    syncsStateInformation(SyncStateInformation::NO_SAVING_SYNCS);
}

void SettingsDialog::loadSizeLimits()
{
    hasUpperLimit = preferences->upperSizeLimit();
    hasLowerLimit = preferences->lowerSizeLimit();
    upperLimit = preferences->upperSizeLimitValue();
    lowerLimit = preferences->lowerSizeLimitValue();
    upperLimitUnit = preferences->upperSizeLimitUnit();
    lowerLimitUnit = preferences->lowerSizeLimitUnit();
    ui->lLimitsInfo->setText(getFormatString());
    hasDaysLimit = preferences->cleanerDaysLimit();
    daysLimit = preferences->cleanerDaysLimitValue();
    ui->lLocalCleanerState->setText(getFormatLimitDays());
}
#ifndef WIN32
void SettingsDialog::on_bPermissions_clicked()
{
    QPointer<PermissionsDialog> dialog = new PermissionsDialog(this);
    dialog->setFolderPermissions(folderPermissions ? folderPermissions : megaApi->getDefaultFolderPermissions());
    dialog->setFilePermissions(filePermissions ? filePermissions : megaApi->getDefaultFilePermissions());

    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    filePermissions = dialog->filePermissions();
    folderPermissions = dialog->folderPermissions();
    delete dialog;

    if (filePermissions != preferences->filePermissionsValue() ||
       folderPermissions != preferences->folderPermissionsValue())
    {
        permissionsChanged = true;
        stateChanged();
    }
}
#endif

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
        currentLocalFolders.append(static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(i, 0))->fullPath());
        currentMegaFoldersPaths.append(static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(i, 1))->fullPath());
    }

    QPointer<BindFolderDialog> dialog = new BindFolderDialog(app, syncNames, currentLocalFolders, currentMegaFoldersPaths, this);
    if(megaFolderHandle > 0)
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

    QSyncItemWidget *localFolder = new QSyncItemWidget();
    localFolder->setPathAndName(localFolderPath, dialog->getSyncName());
    QSyncItemWidget *megaFolder = new QSyncItemWidget();

    //Check if need to setError here or it is enough setting when syncstatechanged
    megaFolder->setPathAndGuessName(dialog->getMegaPath());
    int pos = ui->tSyncs->rowCount();
    ui->tSyncs->setRowCount(pos+1);
    localFolder->setToolTip(localFolderPath);
    ui->tSyncs->setCellWidget(pos, 0, localFolder);
    megaFolder->setToolTip(dialog->getMegaPath());
    ui->tSyncs->setCellWidget(pos, 1, megaFolder);

    QCheckBox *c = new QCheckBox();
    c->setChecked(true);
    c->setToolTip(tr("Enable / disable"));
    connect(c, SIGNAL(stateChanged(int)), this, SLOT(syncStateChanged(int)),Qt::QueuedConnection);
    ui->tSyncs->setCellWidget(pos, 2, c);

    // Col 5: MegaHandle. HIDDEN
    QLabel *lHandle = new QLabel();
    lHandle->setText(QString::number(handle));
    ui->tSyncs->setCellWidget(pos, 4, lHandle);

    // Col 6: SyncName. HIDDEN
    QLabel *lName = new QLabel();
    lName->setText(dialog->getSyncName());
    ui->tSyncs->setCellWidget(pos, 5, lName);

    syncNames.append(dialog->getSyncName());

    delete dialog;

    syncsChanged = true;
    stateChanged();
}

void SettingsDialog::on_bAdd_clicked()
{
    const auto invalidMegaFolderHandle = 0;
    addSyncFolder(invalidMegaFolderHandle);
}

void SettingsDialog::on_bApply_clicked()
{
    saveSettings();
}

void SettingsDialog::on_bUnlink_clicked()
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

void SettingsDialog::on_tSyncs_doubleClicked(const QModelIndex &index)
{
    if (!index.column())
    {
        QString localFolderPath = static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(index.row(), 0))->fullPath();
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localFolderPath));
    }
    else
    {
        QString megaFolderPath = static_cast<QSyncItemWidget*>(ui->tSyncs->cellWidget(index.row(), 1))->fullPath();
        MegaNode *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
        if (node)
        {
            const char *handle = node->getBase64Handle();
            QString url = Preferences::BASE_URL + QString::fromAscii("/fm/") + QString::fromAscii(handle);
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
        stateChanged();
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
        stateChanged();
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
    if (selected.size() == 0)
    {
        return;
    }

    for (int i = 0; i < selected.size(); i++)
    {
        delete selected[i];
    }

    excludedNamesChanged = true;
    stateChanged();
}

void SettingsDialog::on_bExcludeSize_clicked()
{
    QPointer<SizeLimitDialog> dialog = new SizeLimitDialog(this);
    dialog->setUpperSizeLimit(hasUpperLimit);
    dialog->setLowerSizeLimit(hasLowerLimit);
    dialog->setUpperSizeLimitValue(upperLimit);
    dialog->setLowerSizeLimitValue(lowerLimit);
    dialog->setUpperSizeLimitUnit(upperLimitUnit);
    dialog->setLowerSizeLimitUnit(lowerLimitUnit);

    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    hasUpperLimit = dialog->upperSizeLimit();
    hasLowerLimit = dialog->lowerSizeLimit();
    upperLimit = dialog->upperSizeLimitValue();
    lowerLimit = dialog->lowerSizeLimitValue();
    upperLimitUnit = dialog->upperSizeLimitUnit();
    lowerLimitUnit = dialog->lowerSizeLimitUnit();
    delete dialog;

    ui->lLimitsInfo->setText(getFormatString());
    if (hasUpperLimit != preferences->upperSizeLimit() ||
       hasLowerLimit != preferences->lowerSizeLimit() ||
       upperLimit != preferences->upperSizeLimitValue() ||
       lowerLimit != preferences->lowerSizeLimitValue() ||
       upperLimitUnit != preferences->upperSizeLimitUnit() ||
       lowerLimitUnit != preferences->lowerSizeLimitUnit())
    {
        sizeLimitsChanged = true;
        stateChanged();
    }
}

void SettingsDialog::on_bLocalCleaner_clicked()
{
    QPointer<LocalCleanScheduler> dialog = new LocalCleanScheduler(this);
    dialog->setDaysLimit(hasDaysLimit);
    dialog->setDaysLimitValue(daysLimit);

    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    hasDaysLimit = dialog->daysLimit();
    daysLimit = dialog->daysLimitValue();
    delete dialog;

    ui->lLocalCleanerState->setText(getFormatLimitDays());
    if (hasDaysLimit != preferences->cleanerDaysLimit() ||
       daysLimit != preferences->cleanerDaysLimitValue())
    {
        cleanerLimitsChanged = true;
        stateChanged();
    }
}

void SettingsDialog::changeEvent(QEvent *event)
{
    modifyingSettings++;
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        ui->bLocalCleaner->setText(ui->bLocalCleaner->text().arg(QString::fromAscii(MEGA_DEBRIS_FOLDER)));
        ui->lFileVersionsSize->setText(tr("File versions: %1").arg(Utilities::getSizeString(fileVersionsSize)));

#ifdef __APPLE__
        ui->cStartOnStartup->setText(tr("Open at login"));
#endif
        ui->cProxyType->addItem(QString::fromUtf8("SOCKS5H"));

        loadSettings();
        onCacheSizeAvailable();
    }
    QDialog::changeEvent(event);
    modifyingSettings--;
}

QString SettingsDialog::getFormatString()
{
    QString format;
    if (hasLowerLimit || hasUpperLimit)
    {
        format += QString::fromUtf8("(");

        if (hasLowerLimit)
        {
            format  += QString::fromUtf8("<") + Utilities::getSizeString(lowerLimit * pow((float)1024, lowerLimitUnit));
        }

        if (hasLowerLimit && hasUpperLimit)
        {
            format  += QString::fromUtf8(", ");
        }

        if (hasUpperLimit)
        {
            format  += QString::fromUtf8(">") + Utilities::getSizeString(upperLimit * pow((float)1024, upperLimitUnit));
        }

        format += QString::fromUtf8(")");
    }
    else
    {
        format = tr("Disabled");
    }
    return format;
}

QString SettingsDialog::getFormatLimitDays()
{
    QString format;
    if (hasDaysLimit)
    {
        if (daysLimit > 1)
        {
            format += tr("Remove files older than %1 days").arg(QString::number(daysLimit));
        }
        else
        {
            format += tr("Remove files older than 1 day");
        }
    }
    else
    {
        format = tr("Disabled");
    }
    return format;
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

#if QT_VERSION > 0x050100
    warningDel->setTextInteractionFlags(Qt::NoTextInteraction | Qt::LinksAccessibleByMouse);
#endif

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

    ui->bClearCache->hide();
    ui->lCacheSize->hide();

    // Move remote SyncDebris widget to left side
    ui->gCache->layout()->removeWidget(ui->wLocalCache);
    ui->wRemoteCache->layout()->removeItem(ui->rSpacer);
#ifndef __APPLE__
    ui->lRemoteCacheSize->setMargin(2);
#endif
    ((QBoxLayout *)ui->gCache->layout())->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

    onClearCache();
}

void SettingsDialog::on_bClearRemoteCache_clicked()
{
    MegaNode *syncDebris = megaApi->getNodeByPath("//bin/SyncDebris");
    if (!syncDebris)
    {
        remoteCacheSize = 0;
        ui->bClearRemoteCache->hide();
        ui->lRemoteCacheSize->hide();
        onClearCache();
        return;
    }

    QPointer<QMessageBox> warningDel = new QMessageBox(this);
    warningDel->setIcon(QMessageBox::Warning);
    warningDel->setWindowTitle(tr("Clear remote backup"));
    warningDel->setTextFormat(Qt::RichText);

#if QT_VERSION > 0x050100
    warningDel->setTextInteractionFlags(Qt::NoTextInteraction | Qt::LinksAccessibleByMouse);
#endif

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

    ui->bClearRemoteCache->hide();
    ui->lRemoteCacheSize->hide();

    onClearCache();
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

    megaApi->removeVersions();
    // Reset file version size to adjust UI
    fileVersionsSize = 0;

    ui->lFileVersionsSize->hide();
    ui->bClearFileVersions->hide();
}

void SettingsDialog::onClearCache()
{
    if (!cacheSize && !remoteCacheSize)
    {
        ui->gCache->setVisible(false);

    #ifdef __APPLE__
        if (!cacheSize && !remoteCacheSize)
        {
            minHeightAnimation->setTargetObject(this);
            maxHeightAnimation->setTargetObject(this);
            minHeightAnimation->setPropertyName("minimumHeight");
            maxHeightAnimation->setPropertyName("maximumHeight");
            minHeightAnimation->setStartValue(minimumHeight());
            maxHeightAnimation->setStartValue(maximumHeight());
            minHeightAnimation->setEndValue(595);
            maxHeightAnimation->setEndValue(595);
            minHeightAnimation->setDuration(150);
            maxHeightAnimation->setDuration(150);
            animationGroup->start();
        }
    #endif
    }
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

    ui->bAccount->setEnabled(completed);
    ui->bSyncs->setEnabled(completed);
    ui->bAdvanced->setEnabled(completed);
    ui->bBandwidth->setEnabled(completed);
    ui->bProxies->setEnabled(completed);
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
            }
            else
            {
                ui->sSyncsState->setCurrentWidget(ui->pNoErrors);
            }
        }
            break;
    }
}

void SettingsDialog::updateStorageElements()
{
    refreshAccountDetails();
}

void SettingsDialog::updateBandwidthElements()
{
    refreshAccountDetails();
}

void SettingsDialog::updateAccountElements()
{
    refreshAccountDetails(); //all those are affected by account type

    // Disable Upgrade buttons for business accounts
    if (preferences->accountType() == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->bUpgrade->hide();
        ui->bUpgradeBandwidth->hide();
    }
    else
    {
        ui->bUpgrade->show();
        ui->bUpgradeBandwidth->show();
    }

    QIcon icon;
    int accType = preferences->accountType();
    switch(accType)
    {
        case Preferences::ACCOUNT_TYPE_FREE:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Free.png"));
            ui->lAccountType->setText(tr("FREE"));
            break;
        case Preferences::ACCOUNT_TYPE_PROI:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Pro_I.png"));
            ui->lAccountType->setText(tr("PRO I"));
            break;
        case Preferences::ACCOUNT_TYPE_PROII:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Pro_II.png"));
            ui->lAccountType->setText(tr("PRO II"));
            break;
        case Preferences::ACCOUNT_TYPE_PROIII:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Pro_III.png"));
            ui->lAccountType->setText(tr("PRO III"));
            break;
        case Preferences::ACCOUNT_TYPE_LITE:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Lite.png"));
            ui->lAccountType->setText(tr("PRO Lite"));
            break;
        case Preferences::ACCOUNT_TYPE_BUSINESS:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/business.png"));
            ui->lAccountType->setText(QString::fromUtf8("BUSINESS"));
            break;
        default:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/Pro_I.png"));
            ui->lAccountType->setText(QString());
            break;
    }

    ui->lAccountImage->setIcon(icon);
    ui->lAccountImage->setIconSize(QSize(32, 32));
}

void SettingsDialog::onProxyTestError()
{
    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Proxy test failed");
    if (proxyTestProgressDialog)
    {
        proxyTestProgressDialog->hide();
        delete proxyTestProgressDialog;
        proxyTestProgressDialog = NULL;
        ui->bApply->setEnabled(true);
        QMegaMessageBox::critical(nullptr, tr("Error"), tr("Your proxy settings are invalid or the proxy doesn't respond"));
    }

    shouldClose = false;
}

void SettingsDialog::onProxyTestSuccess()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Proxy test OK");
    if (ui->rNoProxy->isChecked())
    {
        preferences->setProxyType(Preferences::PROXY_TYPE_NONE);
    }
    else if (ui->rProxyAuto->isChecked())
    {
        preferences->setProxyType(Preferences::PROXY_TYPE_AUTO);
    }
    else if (ui->rProxyManual->isChecked())
    {
        preferences->setProxyType(Preferences::PROXY_TYPE_CUSTOM);
    }

    preferences->setProxyProtocol(ui->cProxyType->currentIndex());
    preferences->setProxyServer(ui->eProxyServer->text().trimmed());
    preferences->setProxyPort(ui->eProxyPort->text().toInt());
    preferences->setProxyRequiresAuth(ui->cProxyRequiresPassword->isChecked());
    preferences->setProxyUsername(ui->eProxyUsername->text());
    preferences->setProxyPassword(ui->eProxyPassword->text());

    app->applyProxySettings();

    if (proxyTestProgressDialog)
    {
        proxyTestProgressDialog->hide();
        delete proxyTestProgressDialog;
        proxyTestProgressDialog = NULL;
    }

    if (shouldClose)
    {
        shouldClose = false;
        this->close();
    }
    else
    {
        loadSettings();
    }
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
            app->rebootApplication(false);
        }
    }
}

void SettingsDialog::onAnimationFinished()
{
    if (ui->wStack->currentWidget() == ui->pAccount)
    {
        ui->pAccount->show();
    }
    else if (ui->wStack->currentWidget() == ui->pSyncs)
    {
        ui->pSyncs->show();
    }
    else if (ui->wStack->currentWidget() == ui->pBandwidth)
    {
        ui->pBandwidth->show();
    }
    else if (ui->wStack->currentWidget() == ui->pProxies)
    {
        ui->pProxies->show();
    }
    else if (ui->wStack->currentWidget() == ui->pAdvanced)
    {
        ui->pAdvanced->show();
    }
}

void SettingsDialog::on_bStorageDetails_clicked()
{
    accountDetailsDialog = new AccountDetailsDialog(megaApi, this);
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
    switch (tab)
    {
    case ACCOUNT_TAB:
        reloadUIpage = true;
        on_bAccount_clicked();
        break;

    case SYNCS_TAB:
        on_bSyncs_clicked();
        break;

    case BANDWIDTH_TAB:
        on_bBandwidth_clicked();
        break;

    case PROXY_TAB:
        on_bProxies_clicked();
        break;

    case ADVANCED_TAB:
        on_bAdvanced_clicked();
        break;

    default:
        break;
    }
}

void SettingsDialog::on_lAccountImage_clicked()
{
    debugCounter++;
    if (debugCounter == 5)
    {
        app->toggleLogging();
        debugCounter = 0;
    }
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
