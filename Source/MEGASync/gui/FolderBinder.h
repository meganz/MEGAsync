#ifndef FOLDERBINDER_H
#define FOLDERBINDER_H

#include <QWidget>
#include <QApplication>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

#include "NodeSelector.h"

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

    long long selectedMegaFolder();
    QString selectedLocalFolder();

private slots:
    void on_bLocalFolder_clicked();

    void on_bMegaFolder_clicked();

private:
    Ui::FolderBinder *ui;
    MegaApplication *app;
    MegaApi *megaApi;
    long long selectedMegaFolderHandle;

};

#endif // FOLDERBINDER_H
