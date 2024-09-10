#ifndef REMOVE_SYNC_CONFIRMATION_DIALOG_H
#define REMOVE_SYNC_CONFIRMATION_DIALOG_H

#include <QDialog>

namespace Ui {
class RemoveSyncConfirmationDialog;
}

class RemoveSyncConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveSyncConfirmationDialog(QWidget* parent = nullptr);
    ~RemoveSyncConfirmationDialog();

private slots:
    void on_bOK_clicked();

private:
    Ui::RemoveSyncConfirmationDialog* ui;
};

#endif // REMOVE_SYNC_CONFIRMATION_DIALOG_H
