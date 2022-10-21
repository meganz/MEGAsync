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
    explicit  BackupNameConflictDialog(const QStringList& candidatePaths, QWidget *parent = nullptr);
    ~BackupNameConflictDialog();

    QMap<QString, QString> getChanges();
    static bool backupNamesValid(QStringList candidatePaths);

private slots:
    void checkChangedNames();
    void openLink(QString link);

private:
    Ui::BackupNameConflictDialog *ui;
    QMap<QString, QString> mBackupNames;

    void createWidgets();
    void insertHLine();
    void addRenameWidget(const QString& path, int conflictNumber);
    bool checkBackupNames();
};

#endif // BACKUPNAMECONFLICTDIALOG_H
