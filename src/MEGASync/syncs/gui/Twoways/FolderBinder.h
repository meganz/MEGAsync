#ifndef FOLDERBINDER_H
#define FOLDERBINDER_H

#include <megaapi.h>

#include <QWidget>
#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

namespace Ui {
class FolderBinder;
}

class MegaApplication;
class FolderBinder : public QWidget
{
    Q_OBJECT

public:
    explicit FolderBinder(QWidget *parent = 0);
    ~FolderBinder();

    mega::MegaHandle selectedMegaFolder();
    void setSelectedMegaFolder(mega::MegaHandle handle, bool disableUi);

    QString selectedLocalFolder();

signals:
    void selectionDone();

private slots:
    void on_bLocalFolder_clicked();
    void on_bMegaFolder_clicked();

protected:
    void changeEvent(QEvent * event);
    void showEvent(QShowEvent *event) override;

private:
    void onLocalFolderSet(const QString& path);
    void checkSelectedSides();

    Ui::FolderBinder *ui;
    MegaApplication *app;
    mega::MegaApi *megaApi;
    mega::MegaHandle selectedMegaFolderHandle;
};

#endif // FOLDERBINDER_H
