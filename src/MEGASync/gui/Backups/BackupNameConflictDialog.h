#ifndef BACKUPNAMECONFLICTDIALOG_H
#define BACKUPNAMECONFLICTDIALOG_H

#include "Backups/BackupRenameWidget.h"
#include "megaapi.h"

#include <QDialog>
#include <QSet>
#include <QMultiMap>

namespace Ui {
class BackupNameConflictDialog;
}

class BackupNameConflictDialog : public QDialog
{
    Q_OBJECT

public:
    explicit  BackupNameConflictDialog(const QStringList& candidatePaths,
                                       const mega::MegaHandle& myBackupsDirHandle,
                                       QWidget *parent = nullptr);
    ~BackupNameConflictDialog();

    QMap<QString, QString> getChanges();
    static bool backupNamesValid(QStringList candidatePaths, mega::MegaHandle myBackupsHandle);

private slots:
    void checkChangedNames();
    void openLink(QString link);

private:
    Ui::BackupNameConflictDialog *ui;
    QMap<QString, QString> mBackupNames;
    mega::MegaHandle mMyBackupsHandle;

    void createWidgets();
    void insertHLine();
    void addRenameWidget(const QString& path, int conflictNumber);
    bool checkBackupNames();
};

#endif // BACKUPNAMECONFLICTDIALOG_H
