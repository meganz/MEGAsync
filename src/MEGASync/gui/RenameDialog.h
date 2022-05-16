#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QTMegaRequestListener.h>

#include <QDialog>
#include <QValidator>

#include <memory>

namespace Ui {
class RenameDialog;
}

class RenameDialog : public QDialog , public mega::MegaRequestListener
{
    Q_OBJECT

public:
    explicit RenameDialog(QWidget *parent = nullptr);
    ~RenameDialog();

    void init(bool isCloud, const QString& originalPath);
    void setEditorValidator(const QValidator *v);

    void renameCloudFile();
    void renameLocalFile();

signals:
    void renameFinished(const QString& newName);

protected slots:
    void onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e);
    void accept() override;

private:
    QString newName();

    Ui::RenameDialog *ui;

    std::unique_ptr<mega::QTMegaRequestListener> mListener;
    mega::MegaHandle mRemoteHandle;
    bool mIsCloud;
    QString mOriginalPath;
};

#endif // RENAMEDIALOG_H
