#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QTimer>
#include <QProcess>

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
    void setTransfer(int type, QString &fileName, long long completedSize, long long totalSize);
	void addRecentFile(QString &fileName, long long fileHandle);
    void setQueuedTransfers(int queuedDownloads, int queuedUploads);
    void updateDialog();

public slots:
   void timerUpdate();

private slots:
    void on_bSettings_clicked();

    void on_bOfficialWeb_clicked();

    void on_bSyncFolder_clicked();

private:
    Ui::InfoDialog *ui;

protected:
    void updateRecentFiles();

    QTimer *timer;
    MegaApplication *app;
};

#endif // INFODIALOG_H
