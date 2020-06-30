#ifndef STORAGEFULLDIALOG_H
#define STORAGEFULLDIALOG_H

#include <QDialog>
#include <memory>

namespace Ui {
class OverquotaFullDialog;
}

enum class OverquotaFullDialogType
{
    storageFullUploads, storageFullSyncs,
    bandwidthFullDownloads, bandwidthFullImportLink, bandwidthFullSync, bandwidthFullStream,
    storageAndBandwidthFullSyncs
};

class OverQuotaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OverQuotaDialog(QWidget *parent = nullptr);
    ~OverQuotaDialog();
    static std::unique_ptr<OverQuotaDialog> createDialog(OverquotaFullDialogType type);

private:
    Ui::OverquotaFullDialog *ui;

private slots:
    void onUpgradeClicked();
};

#endif // STORAGEFULLDIALOG_H
