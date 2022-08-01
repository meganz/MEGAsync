#ifndef DUPLICATEDNODEDIALOG_H
#define DUPLICATEDNODEDIALOG_H

#include "DuplicatedNodeDialogs/DuplicatedNodeItem.h"
#include "DuplicatedNodeDialogs/DuplicatedUploadChecker.h"

#include <QDialog>
#include <QPointer>

namespace Ui {
class DuplicatedNodeDialog;
}

class DuplicatedNodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DuplicatedNodeDialog(QWidget *parent = nullptr);
    ~DuplicatedNodeDialog();

    void checkUpload(const QString& nodePath, std::shared_ptr<mega::MegaNode> parentNode);

    void addNodeItem(DuplicatedNodeItem* item);
    void setHeader(const QString& baseText, const QString &nodeName);

    QList<std::shared_ptr<DuplicatedNodeInfo>> show();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setConflictItems(int count);
    void cleanUi();
    void fillDialog(const QList<std::shared_ptr<DuplicatedNodeInfo>> &conflicts, DuplicatedUploadBase* checker);
    void setDialogTitle(const QString& title);

    Ui::DuplicatedNodeDialog *ui;
    DuplicatedUploadFolder mFolderCheck;
    DuplicatedUploadFile mFileCheck;

    QList<std::shared_ptr<DuplicatedNodeInfo>> mUploads;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFileConflicts;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFolderConflicts;
    bool mApplyToAll;

    QString mHeaderBaseName;
    QString mCurrentNodeName;
};

#endif // DUPLICATEDNODEDIALOG_H
