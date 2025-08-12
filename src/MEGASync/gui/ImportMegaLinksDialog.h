#ifndef IMPORTMEGALINKSDIALOG_H
#define IMPORTMEGALINKSDIALOG_H

#include "megaapi.h"
#include "Preferences.h"

#include <QDialog>
#include <QStringList>
#include <QVector>

namespace Ui {
class ImportMegaLinksDialog;
}

class UploadNodeSelector;

class ImportMegaLinksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportMegaLinksDialog(const QStringList& linkList, QWidget *parent = 0);
    ~ImportMegaLinksDialog();

    bool shouldImport();
    bool shouldDownload();
    QString getImportPath();
    QString getDownloadPath();

signals:
    void linkSelected(int linkId, bool selected);
    void onChangeEvent();

private slots:
    void on_cDownload_clicked();
    void on_cImport_clicked();
    void on_bLocalFolder_clicked();
    void on_bMegaFolder_clicked();
    void on_bOk_clicked();

public slots:
    void onLinkInfoRequestFinish();
    void onLinkStateChanged(int index, int state);
    void onLinkInfoAvailable(int index,
                             const QString& name,
                             int status,
                             long long size,
                             bool isFolder);

protected:
    bool event(QEvent* event) override;

private:
    Ui::ImportMegaLinksDialog *ui;
    mega::MegaApi *mMegaApi;
    std::shared_ptr<Preferences> mPreferences;
    bool mFinished;

    bool mDownloadPathChangedByUser;

    bool mUseDefaultImportPath;
    bool mImportPathChangedByUser;
    QVector<bool> mSelectedItems;

    void initUiAsLogged();
    void initUiAsUnlogged();
    void initImportFolderControl();
    void setInvalidImportFolder();

    void enableOkButton() const;
    void enableLocalFolder(bool enable);
    void enableMegaFolder(bool enable);
    void checkLinkValidAndSelected();

    void onLocalFolderSet(const QString& path);

    void updateDownloadPath();
    void updateImportPath(const QString& path = QString());

    void setSelectedItem(int index, bool selected);
};

#endif // IMPORTMEGALINKSDIALOG_H
