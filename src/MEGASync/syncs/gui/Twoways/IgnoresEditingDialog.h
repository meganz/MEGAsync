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

    void addNameRule(std::shared_ptr<MegaIgnoreRule> rule);


public slots:
	void onAddNameClicked();
	void onDeleteNameClicked();

    void onUpperThanValueChanged(int i);
    void onLowerThanValueChanged(int i);
    void onExcludeUpperUnitCurrentIndexChanged(int i);
    void onExcludeLowerUnitCurrentIndexChanged(int i);

    void onlExcludedNamesChanged(const QModelIndex &topLeft, const QModelIndex &, const QVector<int> &roles = QVector<int>());
    void onExcludeUpperThanClicked();
    void onExcludeLowerThanClicked();
    void onFileChanged(const QString &file);
protected:
signals:
    void signalOpenMegaignore();
private:
    Ui::IgnoresEditingDialog *ui;

private:
    std::shared_ptr<Preferences> mPreferences;
    std::shared_ptr<QFileSystemWatcher> mIgnoresFileWatcher;
    MegaIgnoreManager mManager;
    QString mSyncLocalFolder;
    bool mExtensionsChanged = false;

};

#endif // IGNORESEDITINGDIALOG_H
