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
    Preferences *preferences = Preferences::instance();
    long long cacheSize = 0;
    for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
    {
        QString syncPath = preferences->getLocalFolder(i);
        if (!syncPath.isEmpty())
        {
            Utilities::getFolderSize(syncPath + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER), &cacheSize);
        }
    }
    return cacheSize;
}

void deleteCache()
{
    Preferences *preferences = Preferences::instance();
    for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
    {
        QString syncPath = preferences->getLocalFolder(i);
        if (!syncPath.isEmpty())
        {
            Utilities::removeRecursively(syncPath + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
        }
    }
}

long long calculateRemoteCacheSize(MegaApi *megaApi)
{
    return megaApi->getSize(megaApi->getNodeByPath("//bin/SyncDebris"));
}
void deleteRemoteCache(MegaApi *megaApi)
{
    megaApi->remove(megaApi->getNodeByPath("//bin/SyncDebris"));
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
    sizeLimitsChanged = false;
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

    hasUpperLimit = false;
    hasLowerLimit = false;
    upperLimit = 0;
    lowerLimit = 0;
    upperLimitUnit = Preferences::MEGA_BYTE_UNIT;
    lowerLimitUnit = Preferences::MEGA_BYTE_UNIT;
    debugCounter = 0;

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

#ifndef WIN32    
    #ifndef __APPLE__
    ui->rProxyAuto->hide();
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

    ui->lCacheSeparator->hide();

#else

    ui->wTabHeader->setStyleSheet(QString::fromUtf8("#wTabHeader { border-image: url(\":/images/menu_header.png\"); }"));
    ui->bAccount->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bBandwidth->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bProxies->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bSyncs->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));
    ui->bAdvanced->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-image: url(\":/images/menu_selected.png\"); }"));

    ui->lCacheSeparator->hide();
#endif

    ui->gCache->setVisible(false);
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
        ui->pProxies->hide();
        setMinimumHeight(485);
        setMaximumHeight(485);
        ui->bApply->hide();
    }
#endif
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::setProxyOnly(bool proxyOnly)
{
    this->proxyOnly = proxyOnly;
    loadSettings();
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
        ui->bSyncs->setEnabled(false);
        ui->bSyncs->setChecked(false);
        ui->bAccount->setChecked(true);
        ui->wStack->setCurrentWidget(ui->pAccount);
        ui->pAccount->show();

#ifdef __APPLE__
        setMinimumHeight(480);
        setMaximumHeight(480);
#endif
    }
    else
    {
        ui->bSyncs->setEnabled(true);
    }
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

void SettingsDialog::syncStateChanged(int state)
{
    if (state)
    {
        QCheckBox *c = ((QCheckBox *)QObject::sender());
        for (int j = 0; j < ui->tSyncs->rowCount(); j++)
        {
            if (ui->tSyncs->cellWidget(j, 2) == c)
            {
                QString newLocalPath = ui->tSyncs->item(j, 0)->text();
                QFileInfo fi(newLocalPath);
                if (!fi.exists() || !fi.isDir())
                {
                    QMessageBox::critical(this, tr("Error"),
                       tr("This sync can't be enabled because the local folder doesn't exist"));
                    c->setCheckState(Qt::Unchecked);
                    return;
                }

                QString newMegaPath = ui->tSyncs->item(j, 1)->text();
                MegaNode *n = megaApi->getNodeByPath(newMegaPath.toUtf8().constData());
                if (!n)
                {
                    QMessageBox::critical(this, tr("Error"),
                       tr("This sync can't be enabled because the remote folder doesn't exist"));
                    c->setCheckState(Qt::Unchecked);
                    return;
                }
                delete n;
                break;
            }
        }
    }

    syncsChanged = true;
    stateChanged();
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


void SettingsDialog::onCacheSizeAvailable()
{
    if (cacheSize != -1 && remoteCacheSize != -1)
    {
        if (!cacheSize && !remoteCacheSize)
        {
            return;
        }

        if (cacheSize)
        {
            ui->lCacheSize->setText(QString::fromUtf8(MEGA_DEBRIS_FOLDER) + QString::fromUtf8(": %1").arg(Utilities::getSizeString(cacheSize)));
        }
        else
        {
            ui->lCacheSize->hide();
            ui->bClearCache->hide();
        }

        if (remoteCacheSize)
        {
            ui->lRemoteCacheSize->setText(QString::fromUtf8("SyncDebris: %1").arg(Utilities::getSizeString(remoteCacheSize)));
        }
        else
        {
            ui->lRemoteCacheSize->hide();
            ui->bClearRemoteCache->hide();
        }

        ui->gCache->setVisible(true);
        ui->lCacheSeparator->show();

    #ifdef __APPLE__
        if (ui->wStack->currentWidget() == ui->pAdvanced)
        {
            minHeightAnimation->setTargetObject(this);
            maxHeightAnimation->setTargetObject(this);
            minHeightAnimation->setPropertyName("minimumHeight");
            maxHeightAnimation->setPropertyName("maximumHeight");
            minHeightAnimation->setStartValue(minimumHeight());
            maxHeightAnimation->setStartValue(maximumHeight());
            minHeightAnimation->setEndValue(540);
            maxHeightAnimation->setEndValue(540);
            minHeightAnimation->setDuration(150);
            maxHeightAnimation->setDuration(150);
            animationGroup->start();
        }
    #endif
    }
}
void SettingsDialog::on_bAccount_clicked()
{
    if (ui->wStack->currentWidget() == ui->pAccount)
    {
        ui->bAccount->setChecked(true);
        return;
    }

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
    minHeightAnimation->setEndValue(485);
    maxHeightAnimation->setEndValue(485);
    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}

void SettingsDialog::on_bSyncs_clicked()
{
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
    bwHeight = 540;

    minHeightAnimation->setTargetObject(this);
    maxHeightAnimation->setTargetObject(this);
    minHeightAnimation->setPropertyName("minimumHeight");
    maxHeightAnimation->setPropertyName("maximumHeight");
    minHeightAnimation->setStartValue(minimumHeight());
    maxHeightAnimation->setStartValue(maximumHeight());
    minHeightAnimation->setEndValue(bwHeight);
    maxHeightAnimation->setEndValue(bwHeight);
    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}

void SettingsDialog::on_bAdvanced_clicked()
{
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
    if (!cacheSize && !remoteCacheSize)
    {
        minHeightAnimation->setEndValue(488);
        maxHeightAnimation->setEndValue(488);
    }
    else
    {
        minHeightAnimation->setEndValue(540);
        maxHeightAnimation->setEndValue(540);
    }
    minHeightAnimation->setDuration(150);
    maxHeightAnimation->setDuration(150);
    animationGroup->start();
#endif
}


void SettingsDialog::on_bProxies_clicked()
{
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
    bool saved = true;
    if (ui->bApply->isEnabled())
    {
        saved = saveSettings();
    }

    if (saved)
    {
        this->close();
    }
    else
    {
        shouldClose = true;
    }
}

void SettingsDialog::on_bHelp_clicked()
{
    QString helpUrl = QString::fromAscii("https://mega.nz/help/client/megasync");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(helpUrl));
}

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
    QString userAgent = QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(megaApi->getUserAgent())));
    QString url = QString::fromUtf8("pro/uao=%1").arg(userAgent);
    megaApi->getSessionTransferURL(url.toUtf8().constData());
}

void SettingsDialog::on_bUpgradeBandwidth_clicked()
{
    QString userAgent = QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(megaApi->getUserAgent())));
    QString url = QString::fromUtf8("pro/uao=%1").arg(userAgent);
    megaApi->getSessionTransferURL(url.toUtf8().constData());
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
        refreshAccountDetails();

        QIcon icon;
        switch(preferences->accountType())
        {
            case Preferences::ACCOUNT_TYPE_FREE:
                icon.addFile(QString::fromUtf8(":/images/Free.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->lAccountType->setText(tr("FREE"));
                break;
            case Preferences::ACCOUNT_TYPE_PROI:
                icon.addFile(QString::fromUtf8(":/images/Pro_I.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->lAccountType->setText(tr("PRO I"));
                break;
            case Preferences::ACCOUNT_TYPE_PROII:
                icon.addFile(QString::fromUtf8(":/images/Pro_II.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->lAccountType->setText(tr("PRO II"));
                break;
            case Preferences::ACCOUNT_TYPE_PROIII:
                icon.addFile(QString::fromUtf8(":/images/Pro_III.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->lAccountType->setText(tr("PRO III"));
                break;
            case Preferences::ACCOUNT_TYPE_LITE:
                icon.addFile(QString::fromUtf8(":/images/Lite.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->lAccountType->setText(tr("PRO Lite"));
                break;
            default:
                icon.addFile(QString::fromUtf8(":/images/Pro_I.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->lAccountType->setText(QString());
                break;
        }

        ui->lAccountImage->setIcon(icon);
        ui->lAccountImage->setIconSize(QSize(32, 32));

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

        QString downloadPath = preferences->downloadFolder();
        if (!downloadPath.size())
        {
            downloadPath = Utilities::getDefaultBasePath() + QString::fromUtf8("/MEGAsync Downloads");
        }
        downloadPath = QDir::toNativeSeparators(downloadPath);
        ui->eDownloadFolder->setText(downloadPath);
        hasDefaultDownloadOption = preferences->hasDefaultDownloadFolder();

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

        if (preferences->accountType() == 0) //Free user
        {
            ui->gBandwidthQuota->show();
            ui->bSeparatorBandwidth->show();
            ui->pUsedBandwidth->setValue(0);
            ui->lBandwidth->setText(tr("Used quota for the last %1 hours: %2")
                    .arg(preferences->bandwidthInterval())
                    .arg(Utilities::getSizeString(preferences->usedBandwidth())));
        }
        else
        {
            double totalBandwidth = preferences->totalBandwidth();
            if (totalBandwidth == 0)
            {
                ui->gBandwidthQuota->hide();
                ui->bSeparatorBandwidth->hide();
                ui->pUsedBandwidth->setValue(0);
                ui->lBandwidth->setText(tr("Data temporarily unavailable"));
            }
            else
            {
                ui->gBandwidthQuota->show();
                ui->bSeparatorBandwidth->show();
                int bandwidthPercentage = ceil(100*((double)preferences->usedBandwidth()/preferences->totalBandwidth()));
                ui->pUsedBandwidth->setValue((bandwidthPercentage < 100) ? bandwidthPercentage : 100);
                ui->lBandwidth->setText(tr("%1 (%2%) of %3 used")
                        .arg(Utilities::getSizeString(preferences->usedBandwidth()))
                        .arg(QString::number(bandwidthPercentage))
                        .arg(Utilities::getSizeString(preferences->totalBandwidth())));
            }
        }

        //Advanced
        ui->lExcludedNames->clear();
        QStringList excludedNames = preferences->getExcludedSyncNames();
        for (int i = 0; i < excludedNames.size(); i++)
        {
            ui->lExcludedNames->addItem(excludedNames[i]);
        }

        loadSizeLimits();
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

void SettingsDialog::refreshAccountDetails()
{
    if (preferences->totalStorage() == 0)
    {
        ui->pStorage->setValue(0);
        ui->lStorage->setText(tr("Data temporarily unavailable"));
        ui->bStorageDetails->setEnabled(false);
    }
    else
    {
        ui->bStorageDetails->setEnabled(true);
        int percentage = ceil(100*((double)preferences->usedStorage()/preferences->totalStorage()));
        ui->pStorage->setValue((percentage < 100) ? percentage : 100);
        ui->lStorage->setText(tr("%1 (%2%) of %3 used")
              .arg(Utilities::getSizeString(preferences->usedStorage()))
              .arg(QString::number(percentage))
              .arg(Utilities::getSizeString(preferences->totalStorage())));
    }

    if (preferences->totalBandwidth() == 0)
    {
        ui->pUsedBandwidth->setValue(0);
        ui->lBandwidth->setText(tr("Data temporarily unavailable"));
    }
    else
    {
        int percentage = ceil(100*((double)preferences->usedBandwidth()/preferences->totalBandwidth()));
        ui->pUsedBandwidth->setValue((percentage < 100) ? percentage : 100);
        ui->lBandwidth->setText(tr("%1 (%2%) of %3 used")
              .arg(Utilities::getSizeString(preferences->usedBandwidth()))
              .arg(QString::number(percentage))
              .arg(Utilities::getSizeString(preferences->totalBandwidth())));
    }

    if (accountDetailsDialog)
    {
        accountDetailsDialog->refresh(preferences);
    }
}

bool SettingsDialog::saveSettings()
{
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
            //Check for removed or disabled folders
            for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
            {
                QString localPath = preferences->getLocalFolder(i);
                QString megaPath = preferences->getMegaFolder(i);
                MegaHandle megaHandle = preferences->getMegaFolderHandle(i);

                int j;
                for (j = 0; j < ui->tSyncs->rowCount(); j++)
                {
                    QString newLocalPath = ui->tSyncs->item(j, 0)->text();
                    QString newMegaPath = ui->tSyncs->item(j, 1)->text();
                    bool enabled = ((QCheckBox *)ui->tSyncs->cellWidget(j, 2))->isChecked();

                    if (!megaPath.compare(newMegaPath) && !localPath.compare(newLocalPath))
                    {
                        if (!enabled && preferences->isFolderActive(i) != enabled)
                        {
                            Platform::syncFolderRemoved(preferences->getLocalFolder(i),
                                                        preferences->getSyncName(i),
                                                        preferences->getSyncID(i));
                            preferences->setSyncState(i, enabled);

                            MegaNode *node = megaApi->getNodeByHandle(megaHandle);
                            megaApi->removeSync(node);
                            delete node;
                        }
                        break;
                    }
                }

                if (j == ui->tSyncs->rowCount())
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Removing sync: %1").arg(preferences->getSyncName(i)).toUtf8().constData());
                    bool active = preferences->isFolderActive(i);
                    MegaNode *node = megaApi->getNodeByHandle(megaHandle);
                    if (active)
                    {
                        Platform::syncFolderRemoved(preferences->getLocalFolder(i),
                                                    preferences->getSyncName(i),
                                                    preferences->getSyncID(i));
                        megaApi->removeSync(node);
                    }
                    Utilities::removeRecursively(preferences->getLocalFolder(i) + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
                    preferences->removeSyncedFolder(i);
                    delete node;
                    i--;
                }
            }

            //Check for new or enabled folders
            for (int i = 0; i < ui->tSyncs->rowCount(); i++)
            {
                QString localFolderPath = ui->tSyncs->item(i, 0)->text();
                QString megaFolderPath = ui->tSyncs->item(i, 1)->text();
                bool enabled = ((QCheckBox *)ui->tSyncs->cellWidget(i, 2))->isChecked();

                QString syncName = syncNames.at(i);
                MegaNode *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
                if (!node)
                {
                    if (enabled)
                    {
                        ((QCheckBox *)ui->tSyncs->cellWidget(i, 2))->setChecked(false);
                    }

                    continue;
                }

                int j;

                QFileInfo localFolderInfo(localFolderPath);
                localFolderPath = QDir::toNativeSeparators(localFolderInfo.canonicalFilePath());
                if (!localFolderPath.size() || !localFolderInfo.isDir())
                {
                    if (enabled)
                    {
                        ((QCheckBox *)ui->tSyncs->cellWidget(i, 2))->setChecked(false);
                    }

                    continue;
                }

                for (j = 0; j < preferences->getNumSyncedFolders(); j++)
                {
                    QString previousLocalPath = preferences->getLocalFolder(j);
                    QString previousMegaPath = preferences->getMegaFolder(j);

                    if (!megaFolderPath.compare(previousMegaPath) && !localFolderPath.compare(previousLocalPath))
                    {
                        if (enabled && preferences->isFolderActive(j) != enabled)
                        {
                            preferences->setMegaFolderHandle(j, node->getHandle());
                            preferences->setSyncState(j, enabled);
                            megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
                        }
                        break;
                    }
                }

                if (j == preferences->getNumSyncedFolders())
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Adding sync: %1 - %2")
                                 .arg(localFolderPath).arg(megaFolderPath).toUtf8().constData());

                    preferences->addSyncedFolder(localFolderPath,
                                                 megaFolderPath,
                                                 node->getHandle(),
                                                 syncName, enabled);

                    if (enabled)
                    {
                        megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
                    }
                }
                delete node;
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
                for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
                {
                    Platform::addSyncToLeftPane(preferences->getLocalFolder(i),
                                                preferences->getSyncName(i),
                                                preferences->getSyncID(i));
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

        //Advanced
        if (excludedNamesChanged)
        {
            QStringList excludedNames;
            for (int i = 0; i < ui->lExcludedNames->count(); i++)
            {
                excludedNames.append(ui->lExcludedNames->item(i)->text());
            }
            preferences->setExcludedSyncNames(excludedNames);

            vector<string> vExclusions;
            for (int i = 0; i < excludedNames.size(); i++)
            {
                vExclusions.push_back(excludedNames[i].toUtf8().constData());
            }
            megaApi->setExcludedNames(&vExclusions);

            QMegaMessageBox::information(this, tr("Warning"), tr("The new excluded file names will be taken into account\n"
                                                                            "when the application starts again"),
                                         Utilities::getDevicePixelRatio(), QMessageBox::Ok);
            excludedNamesChanged = false;
            preferences->setCrashed(true);

            QT_TR_NOOP("Do you want to restart MEGAsync now?");
        }

        if (sizeLimitsChanged)
        {
            preferences->setUpperSizeLimit(hasUpperLimit);
            preferences->setLowerSizeLimit(hasLowerLimit);
            preferences->setUpperSizeLimitValue(upperLimit);
            preferences->setLowerSizeLimitValue(lowerLimit);
            preferences->setUpperSizeLimitUnit(upperLimitUnit);
            preferences->setLowerSizeLimitUnit(lowerLimitUnit);

            if (hasLowerLimit)
            {
                megaApi->setExclusionLowerSizeLimit(preferences->lowerSizeLimitValue() * pow((float)1024, preferences->lowerSizeLimitUnit()));
            }
            else
            {
                megaApi->setExclusionLowerSizeLimit(0);
            }

            if (hasUpperLimit)
            {
                megaApi->setExclusionUpperSizeLimit(preferences->upperSizeLimitValue() * pow((float)1024, preferences->upperSizeLimitUnit()));
            }
            else
            {
                megaApi->setExclusionUpperSizeLimit(0);
            }

            QMegaMessageBox::information(this, tr("Warning"),
                                         tr("The new excluded file sizes will be taken into account when the application starts again."),
                                         Utilities::getDevicePixelRatio(),
                                         QMessageBox::Ok);
            sizeLimitsChanged = false;
        }

        if (ui->cOverlayIcons->isChecked() != preferences->overlayIconsDisabled())
        {
            preferences->disableOverlayIcons(ui->cOverlayIcons->isChecked());
            for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
            {
                Platform::notifyItemChange(preferences->getLocalFolder(i));
            }
        }
    }

    bool proxyChanged = false;
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
        proxyChanged = true;
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

        proxyTestProgressDialog = new MegaProgressDialog(tr("Please wait..."), QString(), 0, 0, this, Qt::CustomizeWindowHint|Qt::WindowTitleHint);
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
    int numFolders = preferences->getNumSyncedFolders();
    ui->tSyncs->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tSyncs->setRowCount(numFolders);
    ui->tSyncs->setColumnCount(3);
    ui->tSyncs->setColumnWidth(2, 21);

    for (int i = 0; i < numFolders; i++)
    {
        QTableWidgetItem *localFolder = new QTableWidgetItem();
        localFolder->setText(preferences->getLocalFolder(i));
        QTableWidgetItem *megaFolder = new QTableWidgetItem();
        megaFolder->setText(preferences->getMegaFolder(i));
        localFolder->setToolTip(preferences->getLocalFolder(i));
        ui->tSyncs->setItem(i, 0, localFolder);
        megaFolder->setToolTip(preferences->getMegaFolder(i));
        ui->tSyncs->setItem(i, 1, megaFolder);
        syncNames.append(preferences->getSyncName(i));
        QCheckBox *c = new QCheckBox();
        c->setChecked(preferences->isFolderActive(i));
        c->setToolTip(tr("Enable / disable"));
        connect(c, SIGNAL(stateChanged(int)), this, SLOT(syncStateChanged(int)));
        ui->tSyncs->setCellWidget(i, 2, c);
    }
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
void SettingsDialog::on_bAdd_clicked()
{
    QStringList currentLocalFolders;
    QList<long long> currentMegaFolders;
    for (int i = 0; i < ui->tSyncs->rowCount(); i++)
    {
        QString localFolder = ui->tSyncs->item(i, 0)->text();
        currentLocalFolders.append(localFolder);

        QString newMegaPath = ui->tSyncs->item(i, 1)->text();
        MegaNode *n = megaApi->getNodeByPath(newMegaPath.toUtf8().constData());
        if (!n)
        {
            continue;
        }

        currentMegaFolders.append(n->getHandle());
        delete n;
    }

    QPointer<BindFolderDialog> dialog = new BindFolderDialog(app, syncNames, currentLocalFolders, currentMegaFolders, this);
    int result = dialog->exec();
    if (!dialog || result != QDialog::Accepted)
    {
        delete dialog;
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    MegaHandle handle = dialog->getMegaFolder();
    MegaNode *node = megaApi->getNodeByHandle(handle);
    if (!localFolderPath.length() || !node)
    {
        delete dialog;
        delete node;
        return;
    }

    QTableWidgetItem *localFolder = new QTableWidgetItem();
    localFolder->setText(localFolderPath);
    QTableWidgetItem *megaFolder = new QTableWidgetItem();
    const char *nPath = megaApi->getNodePath(node);
    if (!nPath)
    {
        delete dialog;
        delete node;
        return;
    }

    megaFolder->setText(QString::fromUtf8(nPath));
    int pos = ui->tSyncs->rowCount();
    ui->tSyncs->setRowCount(pos+1);
    localFolder->setToolTip(localFolderPath);
    ui->tSyncs->setItem(pos, 0, localFolder);
    megaFolder->setToolTip(QString::fromUtf8(nPath));
    ui->tSyncs->setItem(pos, 1, megaFolder);

    QCheckBox *c = new QCheckBox();
    c->setChecked(true);
    c->setToolTip(tr("Enable / disable"));
    connect(c, SIGNAL(stateChanged(int)), this, SLOT(syncStateChanged(int)));
    ui->tSyncs->setCellWidget(pos, 2, c);

    syncNames.append(dialog->getSyncName());
    delete [] nPath;
    delete dialog;
    delete node;

    syncsChanged = true;
    stateChanged();
}

void SettingsDialog::on_bApply_clicked()
{
    saveSettings();
}

void SettingsDialog::on_bUnlink_clicked()
{
    if (QMessageBox::question(this, tr("Logout"),
            tr("Synchronization will stop working.") + QString::fromAscii(" ") + tr("Are you sure?"),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
    {
        this->close();
        app->unlink();
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
             tr("Export Master key"), dir.filePath(QString::fromUtf8("MEGA-RECOVERYKEY")),
             QString::fromUtf8("Txt file (*.txt)"), NULL, QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QFile::Truncate))
    {
        QMegaMessageBox::information(this, tr("Unable to write file"), file.errorString(), Utilities::getDevicePixelRatio());
        return;
    }

    QTextStream out(&file);
    out << megaApi->exportMasterKey();

    file.close();

    QMegaMessageBox::information(this, tr("Warning"),
                                 tr("Exporting the master key and keeping it in a secure location enables you to set a new password without data loss.") + QString::fromUtf8("\n")
                                 + tr("Always keep physical control of your master key (e.g. on a client device, external storage, or print)."),
                                 Utilities::getDevicePixelRatio(),
                                 QMessageBox::Ok);
}

void SettingsDialog::on_tSyncs_doubleClicked(const QModelIndex &index)
{
    if (!index.column())
    {
        QString localFolderPath = ui->tSyncs->item(index.row(), 0)->text();
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localFolderPath));
    }
    else
    {
        QString megaFolderPath = ui->tSyncs->item(index.row(), 1)->text();
        MegaNode *node = megaApi->getNodeByPath(megaFolderPath.toUtf8().constData());
        if (node)
        {
            const char *handle = node->getBase64Handle();
            QString url = QString::fromAscii("https://mega.nz/#fm/") + QString::fromAscii(handle);
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
            QMessageBox::critical(window(), tr("Error"), tr("You don't have write permissions in this local folder."));
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
    QPointer<QInputDialog> id = new QInputDialog(this);
    id->setWindowTitle(tr("Excluded name"));
    id->setLabelText(tr("Enter a name to exclude from synchronization.\n(wildcards * and ? are allowed):"));
    int result = id->exec();

    if (!id || !result)
    {
      delete id;
      return;
    }

    QString text = id->textValue();
    delete id;
    text = text.trimmed();
    if (text.isEmpty())
    {
        return;
    }

    QRegExp regExp(text, Qt::CaseInsensitive, QRegExp::Wildcard);
    if (!regExp.isValid())
    {
        QMessageBox::warning(this, tr("Error"), QString::fromUtf8("You have entered an invalid file name or expression."), QMessageBox::Ok);
        return;
    }

    for (int i = 0; i < ui->lExcludedNames->count(); i++)
    {
        if (ui->lExcludedNames->item(i)->text() == text)
        {
            return;
        }
        else if (ui->lExcludedNames->item(i)->text().compare(text, Qt::CaseInsensitive)>0)
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

void SettingsDialog::changeEvent(QEvent *event)
{
    modifyingSettings++;
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

#ifdef __APPLE__
        setWindowTitle(tr("Preferences - MEGAsync"));
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

void SettingsDialog::on_bClearCache_clicked()
{
    QString syncs;
    int numFolders = preferences->getNumSyncedFolders();
    for (int i = 0; i < numFolders; i++)
    {
        QFileInfo fi(preferences->getLocalFolder(i) + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
        if (fi.exists() && fi.isDir())
        {
            syncs += QString::fromUtf8("<br/><a href=\"local://#%1\">%2</a>").arg(fi.absoluteFilePath()).arg(preferences->getSyncName(i));
        }
    }

    QPointer<QMessageBox> warningDel = new QMessageBox(this);
    warningDel->setIcon(QMessageBox::Warning);
    warningDel->setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-warning.png")
                                                                : QString::fromUtf8(":/images/mbox-warning@2x.png")));

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
    warningDel->setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-warning.png")
                                                                : QString::fromUtf8(":/images/mbox-warning@2x.png")));
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

void SettingsDialog::onClearCache()
{
    if (!cacheSize && !remoteCacheSize)
    {
        ui->gCache->setVisible(false);
        ui->lCacheSeparator->hide();

    #ifdef __APPLE__
        if (!cacheSize && !remoteCacheSize)
        {
            minHeightAnimation->setTargetObject(this);
            maxHeightAnimation->setTargetObject(this);
            minHeightAnimation->setPropertyName("minimumHeight");
            maxHeightAnimation->setPropertyName("maximumHeight");
            minHeightAnimation->setStartValue(minimumHeight());
            maxHeightAnimation->setStartValue(maximumHeight());
            minHeightAnimation->setEndValue(488);
            maxHeightAnimation->setEndValue(488);
            minHeightAnimation->setDuration(150);
            maxHeightAnimation->setDuration(150);
            animationGroup->start();
        }
    #endif
    }

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
        QMessageBox::critical(this, tr("Error"), tr("Your proxy settings are invalid or the proxy doesn't respond"));
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
    else loadSettings();
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
    if (QMessageBox::warning(this, tr("Full scan"), tr("MEGAsync will perform a full scan of your synced folders when it starts.\n\nDo you want to restart MEGAsync now?"),
                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        app->rebootApplication(false);
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

MegaProgressDialog::MegaProgressDialog(const QString &labelText, const QString &cancelButtonText,
                                       int minimum, int maximum, QWidget *parent, Qt::WindowFlags f) :
    QProgressDialog(labelText, cancelButtonText, minimum, maximum, parent, f) {}

void MegaProgressDialog::reject() {}
void MegaProgressDialog::closeEvent(QCloseEvent * event)
{
    event->ignore();
}
