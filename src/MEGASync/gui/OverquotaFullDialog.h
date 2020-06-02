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
    bandwidthFullDownlads, bandwidthFullImportLink, bandwidthFullSync, bandwidthFullStream,
    storageAndBandwidthFullSyncs
};

class OverquotaFullDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OverquotaFullDialog(QWidget *parent = nullptr);
    ~OverquotaFullDialog();
    static std::unique_ptr<OverquotaFullDialog> createDialog(OverquotaFullDialogType type);

private:
    Ui::OverquotaFullDialog *ui;
};

#endif // STORAGEFULLDIALOG_H
