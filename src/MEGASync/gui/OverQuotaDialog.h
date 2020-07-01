#ifndef STORAGEFULLDIALOG_H
#define STORAGEFULLDIALOG_H

#include <QDialog>
#include <memory>

namespace Ui {
class OverquotaFullDialog;
}

enum class OverQuotaDialogType
{
    STORAGE_UPLOAD = 0, STORAGE_SYNCS,
    BANDWIDTH_DOWNLOAD, BANDWIDTH_IMPORT_LINK, BANDWITH_SYNC, BANDWIDTH_STREAM,
    STORAGE_BANDWIDTH_SYNC
};

class OverQuotaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OverQuotaDialog(OverQuotaDialogType type, QWidget *parent = nullptr);
    ~OverQuotaDialog();
    static std::unique_ptr<OverQuotaDialog> createDialog(OverQuotaDialogType type);

private:
    Ui::OverquotaFullDialog *ui;
    void configureDialog(OverQuotaDialogType type);

private slots:
    void onUpgradeClicked();
};

#endif // STORAGEFULLDIALOG_H
