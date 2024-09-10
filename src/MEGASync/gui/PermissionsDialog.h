#ifndef PERMISSIONSDIALOG_H
#define PERMISSIONSDIALOG_H

#include <QDialog>

namespace Ui {
class PermissionsDialog;
}

class PermissionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PermissionsDialog(QWidget *parent = 0);
    ~PermissionsDialog();

    int folderPermissions();
    void setFolderPermissions(int permissions);
    int filePermissions();
    void setFilePermissions(int permissions);

private:
    Ui::PermissionsDialog *ui;

private slots:
    void permissionsChanged();
};

#endif // PERMISSIONSDIALOG_H
