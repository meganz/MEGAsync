#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QTimer>
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
    void setUsage(int totalGB, int percentage);

public slots:
   void timerUpdate();

private slots:
    void on_bSettings_clicked();

    void on_bOfficialWeb_clicked();

    void on_bSyncFolder_clicked();

private:
    Ui::InfoDialog *ui;

protected:
    SettingsDialog *settingsDialog;
    QTimer *timer;
    MegaApplication *app;
};

#endif // INFODIALOG_H
