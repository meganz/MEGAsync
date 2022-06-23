#ifndef FILEFOLDERNAMESETTERDIALOG_H
#define FILEFOLDERNAMESETTERDIALOG_H

#include "QTMegaRequestListener.h"
#include <megaapi.h>

#include <QDialog>
#include <QTimer>

#include <memory>

namespace Ui {
class FileFolderNameSetterDialog;
}

class FileFolderNameSetterDialog : public QDialog, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    FileFolderNameSetterDialog(std::shared_ptr<mega::MegaNode> parentNode, QWidget* parent);

    int show();
    QString getName() const;

protected:
    virtual void onDialogAccepted() = 0;
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) = 0;

    Ui::FileFolderNameSetterDialog* mUi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<mega::MegaNode> mParentNode;

private:
    void showError();

    QTimer mNewFolderErrorTimer;

private slots:
    void dialogAccepted();
    void newFolderErrorTimedOut();
};

class NewFolderDialog : public FileFolderNameSetterDialog
{
    Q_OBJECT

public:
    NewFolderDialog(std::shared_ptr<mega::MegaNode> parentNode, QWidget* parent);

    std::unique_ptr<mega::MegaNode> getNewNode();

protected:
    void onDialogAccepted() override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;

private:
    std::unique_ptr<mega::MegaNode> mNewNode;
};


#endif // FILEFOLDERNAMESETTERDIALOG_H
