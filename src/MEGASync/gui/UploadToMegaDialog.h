#ifndef UPLOADTOMEGADIALOG_H
#define UPLOADTOMEGADIALOG_H

#include <QDialog>
#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "HighDpiResize.h"

namespace Ui {
class UploadToMegaDialog;
}

class UploadToMegaDialog : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:

    static const char* NODE_PATH_PROPERTY;
    static const QString DEFAULT_PATH;

    explicit UploadToMegaDialog(mega::MegaApi *megaApi, QWidget *parent = 0);
    ~UploadToMegaDialog();
    mega::MegaHandle getSelectedHandle();
    bool isDefaultFolder();
    void setDefaultFolder(long long handle);

    virtual void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e);

private slots:
    void on_bChange_clicked();
    void on_bOK_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::UploadToMegaDialog *ui;
    mega::MegaApi *megaApi;
    mega::MegaHandle selectedHandle;
    mega::QTMegaRequestListener *delegateListener;
    HighDpiResize highDpiResize;
};

#endif // UPLOADTOMEGADIALOG_H
