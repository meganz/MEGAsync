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
    virtual bool event(QEvent *event);

private slots:
    void on_bNext_clicked();

    void on_bBack_clicked();

    void on_bCancel_clicked();

    void on_bLocalFolder_clicked();

    void on_bMegaFolder_clicked();

private:
    Ui::SetupWizard *ui;
    MegaApplication *app;
    MegaApi *megaApi;
    Preferences *preferences;
    MegaRequest *request;
    MegaError *error;
    long long selectedMegaFolderHandle;
};

#endif // SETUPWIZARD_H
