#ifndef UPLOADTOMEGADIALOG_H
#define UPLOADTOMEGADIALOG_H

#include <QDialog>
#include "megaapi.h"
#include <memory>

namespace Ui {
class UploadToMegaDialog;
}

class UploadToMegaDialog : public QDialog
{
    Q_OBJECT

public:

    static const char* NODE_PATH_PROPERTY;

    explicit UploadToMegaDialog(mega::MegaApi *megaApi, QWidget *parent = 0);
    ~UploadToMegaDialog();
    mega::MegaHandle getSelectedHandle();
    bool isDefaultFolder();
    void setDefaultFolder(mega::MegaHandle handle);

    void onRequestFinish(mega::MegaRequest *request, mega::MegaError *e);

private slots:
    void on_bChange_clicked();
    void on_bOK_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    std::unique_ptr<mega::MegaNode> getUploadFolder();
    void showNodeSelector();

    static QString getDefaultPath();
    void updatePath(const QString& path = QString());

    bool mUseDefaultPath;
    bool mPathChangedByUser;

    Ui::UploadToMegaDialog *ui;
    mega::MegaApi *megaApi;
    mega::MegaHandle selectedHandle;
};

#endif // UPLOADTOMEGADIALOG_H
