#ifndef BINDFOLDERDIALOG_H
#define BINDFOLDERDIALOG_H

#include <megaapi.h>

#include <QDialog>


class MegaApplication;

namespace Ui {
class BindFolderDialog;
}

class BindFolderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BindFolderDialog(MegaApplication* _app, QWidget* parent = nullptr);
    ~BindFolderDialog();

    mega::MegaHandle getMegaFolder();
    void setMegaFolder(mega::MegaHandle handle, bool disableUi);
    void setLocalFolder(const QString& localPath, bool disableUi);

    QString getLocalFolder();
    QString getSyncName();

    QString getMegaPath() const;

private slots:
    void on_bOK_clicked();
    void allSelectionsDone();

protected:
    bool focusNextPrevChild(bool next) override;
    void changeEvent(QEvent * event);

private:
    Ui::BindFolderDialog *ui;
    MegaApplication *mApp;
    QString mSyncName;
    QString mMegaPath;
};

#endif // BINDFOLDERDIALOG_H
