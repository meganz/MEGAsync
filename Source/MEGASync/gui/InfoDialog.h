#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QTimer>
#include <QProcess>
#include <QDateTime>

#include "SettingsDialog.h"

namespace Ui {
class InfoDialog;
}

class MegaApplication;
class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(MegaApplication *app, QWidget *parent = 0);
    ~InfoDialog();

    void startAnimation();
	void setUsage(m_off_t totalBytes, m_off_t usedBytes);
    void setTransfer(MegaTransfer *transfer);
	void addRecentFile(QString fileName, long long fileHandle, QString localPath);
	void setTransferSpeeds(long long downloadSpeed, long long uploadSpeed);
	void setTransferredSize(long long totalDownloadedSize, long long totalUploadedSize);
	void setTotalTransferSize(long long totalDownloadSize, long long totalUploadSize);
    void setPaused(bool paused);
	void updateDialog();
    void updateTransfers();

public slots:
   void timerUpdate();
   void addSync();
   void onTransfer1Clicked(int x, int y);
   void onTransfer2Clicked(int x, int y);

private slots:
    void on_bSettings_clicked();

    void on_bOfficialWeb_clicked();

    void on_bSyncFolder_clicked();

	void openFolder(QString path);

    void on_bPause_clicked();

    void onOverlayClicked();
private:
    Ui::InfoDialog *ui;
    QPushButton *overlay;

	long long downloadSpeed;
	long long uploadSpeed;
	int currentUpload;
	int currentDownload;
	int totalUploads;
	int totalDownloads;
	long long totalDownloadedSize, totalUploadedSize;
	long long totalDownloadSize, totalUploadSize;
	int remainingUploads, remainingDownloads;

protected:
	bool eventFilter(QObject *obj, QEvent *ev);
	void showPopup(QPoint globalpos, bool download);
	void updateRecentFiles();

	QDateTime lastPopupUpdate;
    QTimer *timer;
    MegaApplication *app;
    Preferences *preferences;
    bool finishing;
};

#endif // INFODIALOG_H
