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

public:
    explicit InfoDialog(MegaApplication *app, bool guestMode = false, QWidget *parent = 0);
    void init();
    ~InfoDialog();

    void setUsage();
    void setTransfer(mega::MegaTransfer *transfer);
    void addRecentFile(QString fileName, long long fileHandle, QString localPath, QString nodeKey);
	void setTransferSpeeds(long long downloadSpeed, long long uploadSpeed);
	void setTransferredSize(long long totalDownloadedSize, long long totalUploadedSize);
	void setTotalTransferSize(long long totalDownloadSize, long long totalUploadSize);
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

    void setGuestMode(bool mode);


public slots:
   void addSync();
   void onTransfer1Cancel(int x, int y);
   void onTransfer2Cancel(int x, int y);
   void cancelAllUploads();
   void cancelAllDownloads();
   void cancelCurrentUpload();
   void cancelCurrentDownload();
   void onAllUploadsFinished();
   void onAllDownloadsFinished();
   void onAllTransfersFinished();

private slots:
    void on_bSettings_clicked();

    void on_bOfficialWeb_clicked();

    void on_bSyncFolder_clicked();

	void openFolder(QString path);

    void on_bPause_clicked();

    void onOverlayClicked();

    void scanningAnimationStep();

    void regenerateLayout();

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
    QSignalMapper *menuSignalMapper;

    long long downloadSpeed;
    long long uploadSpeed;
    unsigned long long effectiveDownloadSpeed;
    unsigned long long effectiveUploadSpeed;
    unsigned long long uploadStartTime;
    unsigned long long downloadStartTime;
    unsigned long long elapsedDownloadTime;
    unsigned long long elapsedUploadTime;
    unsigned long long lastUpdate;
	int currentUpload;
	int currentDownload;
	int totalUploads;
	int totalDownloads;
	long long totalDownloadedSize, totalUploadedSize;
	long long totalDownloadSize, totalUploadSize;
	int remainingUploads, remainingDownloads;
    bool indexing;
    bool waiting;
    bool guestMode;
    GuestWidget *gWidget;

    int getWidgetIndexByName(QString wName);

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
    int scanningAnimationIndex;
    MegaApplication *app;
    Preferences *preferences;
    mega::MegaApi *megaApi;
    mega::MegaTransfer *transfer1;
    mega::MegaTransfer *transfer2;
};

#endif // INFODIALOG_H
