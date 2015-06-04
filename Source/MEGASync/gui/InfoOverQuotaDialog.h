#ifndef INFOOVERQUOTADIALOG_H
#define INFOOVERQUOTADIALOG_H

#include <QDialog>
#include "SettingsDialog.h"

namespace Ui {
class InfoOverQuotaDialog;
}

class MegaApplication;
class InfoOverQuotaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoOverQuotaDialog(MegaApplication *app, QWidget *parent = 0);
    ~InfoOverQuotaDialog();

    void setUsage();

private slots:
    void on_bSettings_clicked();
    void on_bUpgrade_clicked();
    void on_bOfficialWeb_clicked();
#ifndef Q_OS_LINUX
    void on_bOfficialWebIcon_clicked();
#endif

private:
    Ui::InfoOverQuotaDialog *ui;

protected:
    void changeEvent(QEvent *event);

protected:
    MegaApplication *app;
    Preferences *preferences;
    mega::MegaApi *megaApi;
};

#endif // INFOOVERQUOTADIALOG_H
