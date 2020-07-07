#ifndef STORAGEFULLDIALOG_H
#define STORAGEFULLDIALOG_H

#include <QDialog>
#include <QLabel>
#include <memory>


class CustomLabel : public QLabel
{
     Q_OBJECT
public:
    explicit CustomLabel(QWidget * parent = nullptr) : QLabel(parent){};
    ~CustomLabel(){};

signals:
    void labelSizeChange();
protected slots:
    void resizeEvent(QResizeEvent *)
    {
            emit labelSizeChange();
    };

};

namespace Ui {
class OverquotaFullDialog;
}

enum class OverQuotaDialogType
{
    STORAGE_UPLOAD = 0, STORAGE_SYNCS,
    BANDWIDTH_DOWNLOAD, BANDWIDTH_IMPORT_LINK, BANDWITH_SYNC, BANDWIDTH_STREAM
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
    void onTitleLengthChanged();
};

#endif // STORAGEFULLDIALOG_H
