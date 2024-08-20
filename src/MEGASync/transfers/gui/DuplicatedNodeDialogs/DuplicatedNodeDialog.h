#ifndef DUPLICATEDNODEDIALOG_H
#define DUPLICATEDNODEDIALOG_H

#include "DuplicatedNodeItem.h"
#include "DuplicatedUploadChecker.h"

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
    explicit DuplicatedNodeDialog(QWidget* parent = nullptr);
    ~DuplicatedNodeDialog();

    void addNodeItem(DuplicatedNodeItem* item);
    void setHeader(const QString& baseText, const QString &nodeName);

    void show();

    const QList<std::shared_ptr<DuplicatedNodeInfo>>& getResolvedConflicts();
    bool isEmpty() const;

    void setConflicts(std::shared_ptr<ConflictTypes> newConflicts);
    std::shared_ptr<ConflictTypes> conflicts() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void setConflictItems(int count);
    void cleanUi();
    void fillDialog();
    void setDialogTitle(const QString& title);
    void processConflict(std::shared_ptr<DuplicatedNodeInfo> conflict);
    void onConflictProcessed();

    void processFolderConflicts();
    void processFileConflicts();
    void processFileNameConflicts();
    void processFolderNameConflicts();
    void startWithNewCategoryOfConflicts();

    void updateHeader();

    Ui::DuplicatedNodeDialog *ui;

    QList<std::shared_ptr<DuplicatedNodeInfo>> mConflictsBeingProcessed;
    DuplicatedUploadBase* mChecker;
    std::shared_ptr<ConflictTypes>  mConflicts;

    bool mApplyToAll;

    QString mHeaderBaseName;
    QString mCurrentNodeName;

    std::shared_ptr<mega::MegaNode> mNode;

    QTimer mSizeAdjustTimer;
};

#endif // DUPLICATEDNODEDIALOG_H
