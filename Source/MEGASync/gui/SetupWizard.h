#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

#include "NodeSelector.h"
#include "Preferences.h"
#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaRequestListener.h"

namespace Ui {
class SetupWizard;
}

class MegaApplication;
class SetupWizard : public QDialog, public MegaRequestListener
{
    Q_OBJECT

public:
    explicit SetupWizard(MegaApplication *app, QWidget *parent = 0);
    ~SetupWizard();

	virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
    virtual void onRequestUpdate(MegaApi* api, MegaRequest *request);

private slots:

    void on_bNext_clicked();

    void on_bBack_clicked();

    void on_bCancel_clicked();

    void on_bLocalFolder_clicked();

    void on_bMegaFolder_clicked();

    void wTypicalSetup_clicked();

    void wAdvancedSetup_clicked();

    void lTermsLink_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::SetupWizard *ui;
    MegaApplication *app;
    MegaApi *megaApi;
    Preferences *preferences;
    mega::handle selectedMegaFolderHandle;
    QString sessionKey;
	QTMegaRequestListener *delegateListener;
};

#endif // SETUPWIZARD_H
