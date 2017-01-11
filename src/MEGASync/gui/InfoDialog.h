#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QTimer>
#include <QProcess>
#include <QDateTime>
#include <QPainter>
#include "GuestWidget.h"
#include "SettingsDialog.h"

namespace Ui {
class InfoDialog;
}

class MegaApplication;
class InfoDialog : public QDialog
{
    Q_OBJECT

    enum {
        STATE_STARTING,
        STATE_PAUSED,
        STATE_WAITING,
        STATE_INDEXING,
        STATE_UPDATED
    };

public:
    explicit InfoDialog(MegaApplication *app, QWidget *parent = 0);
    ~InfoDialog();

    void setUsage();
    void setTransfer(mega::MegaTransfer *transfer);
    void addRecentFile(QString fileName, long long fileHandle, QString localPath, QString nodeKey);
    void clearRecentFiles();
    void setPaused(bool paused);
    void updateTransfers();
    void transferFinished(int error);
    void updateSyncsButton();
    void setIndexing(bool indexing);
    void setWaiting(bool waiting);
    void increaseUsedStorage(long long bytes, bool isInShare);
    void updateState();
    void showRecentlyUpdated(bool show);
    void closeSyncsMenu();
    void updateRecentFiles();
    void disableGetLink(bool disable);
    void addSync(mega::MegaHandle h);


#ifdef __APPLE__
    void moveArrow(QPoint p);
#endif

    void regenerateLayout();

public slots:
   void addSync();
   void onTransfer1Cancel(int x, int y);
   void onTransfer2Cancel(int x, int y);
   void downloadState();
   void uploadState();
   void cancelAllUploads();
   void cancelAllDownloads();
   void cancelCurrentUpload();
   void cancelCurrentDownload();
   void onAllUploadsFinished();
   void onAllDownloadsFinished();
   void onAllTransfersFinished();
   void onUpdateRecentFiles();

private slots:
    void on_bSettings_clicked();

    void on_bOfficialWeb_clicked();

    void on_bSyncFolder_clicked();

    void openFolder(QString path);

    void on_bPause_clicked();

    void onOverlayClicked();

    void scanningAnimationStep();

    void onUserAction(int action);

#ifdef __APPLE__
    void on_cRecentlyUpdated_stateChanged(int arg1);
    void showRecentList();
    void onAnimationFinished();
#endif

#if defined(_WIN32) || defined(__APPLE__)
    void on_bOfficialWebIcon_clicked();
#endif

private:
    Ui::InfoDialog *ui;
    QPushButton *overlay;
#ifdef __APPLE__
    QPushButton *arrow;
#endif

#ifdef __APPLE__
    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;
#endif

    QMenu *syncsMenu;
    QMenu *transferMenu;

    long long downloadSpeed;
    long long uploadSpeed;
    int currentUpload;
    int currentDownload;
    int totalUploads;
    int totalDownloads;
    int activeDownloadState, activeUploadState;
    long long remainingDownloadBytes, remainingUploadBytes;
    long long meanDownloadSpeed, meanUploadSpeed;
    int remainingUploads, remainingDownloads;
    bool indexing;
    bool waiting;
    GuestWidget *gWidget;
    int state;

protected:
    void changeEvent(QEvent * event);
#ifdef __APPLE__
    void paintEvent( QPaintEvent * e);
    void hideEvent(QHideEvent *event);
#endif

protected:
    QDateTime lastPopupUpdate;
    QTimer scanningTimer;
    QTimer downloadsFinishedTimer;
    QTimer uploadsFinishedTimer;
    QTimer transfersFinishedTimer;
    QTimer recentFilesTimer;
    int scanningAnimationIndex;
    MegaApplication *app;
    Preferences *preferences;
    mega::MegaApi *megaApi;
    mega::MegaTransfer *activeDownload;
    mega::MegaTransfer *activeUpload;
};

#endif // INFODIALOG_H
