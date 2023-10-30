#ifndef IGNORESEDITINGDIALOG_H
#define IGNORESEDITINGDIALOG_H

#include "Preferences.h"
#include "syncs/control/MegaIgnoreManager.h"

#include <QDialog>
#include <QFileSystemWatcher>

#include <memory>

namespace Ui {
class IgnoresEditingDialog;
}

class IgnoresEditingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IgnoresEditingDialog(const QString& syncLocalFolder, bool createIfNotExist = false, QWidget *parent = nullptr);
    ~IgnoresEditingDialog();

    void applyChanges();
    void refreshUI();
    void setOutputIgnorePath(const QString& outputPath);


public slots:
	void on_bAddName_clicked();
	void on_bDeleteName_clicked();

    void on_eUpperThan_valueChanged(int i);
    void on_eLowerThan_valueChanged(int i);
    void on_cbExcludeUpperUnit_currentIndexChanged(int i);
    void on_cbExcludeLowerUnit_currentIndexChanged(int i);

    void onlExcludedNamesChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void on_cExcludeUpperThan_clicked();
    void on_cExcludeLowerThan_clicked();
    void on_fileChanged(const QString file);
protected:
signals:
    void signalOpenMegaignore();
private:
    Ui::IgnoresEditingDialog *ui;

private:
    std::shared_ptr<Preferences> mPreferences;
    std::shared_ptr<QFileSystemWatcher> mIgnoresFileWatcher;
    MegaIgnoreManager mManager;
    bool mExtensionsChanged = false;

};

#endif // IGNORESEDITINGDIALOG_H
