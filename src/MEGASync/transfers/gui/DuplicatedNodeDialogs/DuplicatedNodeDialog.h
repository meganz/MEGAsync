#ifndef DUPLICATEDNODEDIALOG_H
#define DUPLICATEDNODEDIALOG_H

#include "DuplicatedNodeDialogs/DuplicatedNodeItem.h"
#include "DuplicatedNodeDialogs/DuplicatedUploadChecker.h"

#include <QDialog>
#include <QPointer>
#include <QTimer>

namespace Ui {
class DuplicatedNodeDialog;
}

class DuplicatedNodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DuplicatedNodeDialog(std::shared_ptr<mega::MegaNode> node);
    ~DuplicatedNodeDialog();

    void checkUpload(const QString& nodePath, std::shared_ptr<mega::MegaNode> parentNode);

    void addNodeItem(DuplicatedNodeItem* item);
    void setHeader(const QString& baseText, const QString &nodeName);

    void show();

    const std::shared_ptr<mega::MegaNode>& getNode() const;

    const QList<std::shared_ptr<DuplicatedNodeInfo>>& getResolvedConflicts();
    bool isEmpty() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;

private:
    void setConflictItems(int count);
    void cleanUi();
    void fillDialog();
    void setDialogTitle(const QString& title);
    void processConflict(std::shared_ptr<DuplicatedNodeInfo> conflict);
    void onConflictProcessed();

    void processFolderConflicts();
    void processFileConflicts();
    void startWithNewCategoryOfConflicts();

    void updateHeader();

    Ui::DuplicatedNodeDialog *ui;
    DuplicatedUploadFolder mFolderCheck;
    DuplicatedUploadFile mFileCheck;

    QList<std::shared_ptr<DuplicatedNodeInfo>> mConflictsBeingProcessed;
    DuplicatedUploadBase* mChecker;

    QList<std::shared_ptr<DuplicatedNodeInfo>> mResolvedUploads;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFileConflicts;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFolderConflicts;
    bool mApplyToAll;

    QString mHeaderBaseName;
    QString mCurrentNodeName;

    std::shared_ptr<mega::MegaNode> mNode;

    QTimer mSizeAdjustTimer;
};

#endif // DUPLICATEDNODEDIALOG_H
