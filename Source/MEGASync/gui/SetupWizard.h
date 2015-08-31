#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

#include "NodeSelector.h"
#include "Preferences.h"
#include "megaapi.h"
#include "QTMegaRequestListener.h"

namespace Ui {
class SetupWizard;
}

class MegaApplication;
class SetupWizard : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    enum {
        PAGE_SETUP = 0,
        PAGE_NEW_ACCOUNT = 1,
        PAGE_LOGIN = 2
    };

    explicit SetupWizard(MegaApplication *app, QWidget *parent = 0);
    ~SetupWizard();

    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
    virtual void onRequestUpdate(mega::MegaApi* api, mega::MegaRequest *request);

    void goToStep(int page);

private slots:

    void on_bNext_clicked();

    void on_bBack_clicked();

    void on_bCancel_clicked();

    void on_bSkip_clicked();

    void on_bLocalFolder_clicked();

    void on_bMegaFolder_clicked();

    void wTypicalSetup_clicked();

    void wAdvancedSetup_clicked();

    void lTermsLink_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent * event);

private:
    Ui::SetupWizard *ui;
    MegaApplication *app;
    mega::MegaApi *megaApi;
    Preferences *preferences;
    uint64_t selectedMegaFolderHandle;
    QString sessionKey;
    mega::QTMegaRequestListener *delegateListener;

    void setupPreferences();
};

#endif // SETUPWIZARD_H
