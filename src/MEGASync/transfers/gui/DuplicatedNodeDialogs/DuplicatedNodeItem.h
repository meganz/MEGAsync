#ifndef DUPLICATEDNODEITEM_H
#define DUPLICATEDNODEITEM_H

#include <megaapi.h>
#include <DuplicatedNodeDialogs/DuplicatedNodeInfo.h>

#include <memory>

#include <QWidget>
#include <QFutureWatcher>
#include <QDateTime>

namespace Ui {
class DuplicatedNodeItem;
}

/*
 * BASE CLASS
*/
class DuplicatedNodeItem : public QWidget
{
    Q_OBJECT

public:
    DuplicatedNodeItem(QWidget *parent = nullptr);
    virtual ~DuplicatedNodeItem();

    void setInfo(std::shared_ptr<DuplicatedNodeInfo> info, NodeItemType type);

    NodeItemType getType(){return mType;}
    void setType(NodeItemType type);

    void setDescription(const QString& description);

signals:
    void actionClicked();

protected:
    NodeItemType mType;
    std::shared_ptr<DuplicatedNodeInfo> mInfo;

    void fillUi();
    virtual QString getNodeName() = 0;
    virtual QDateTime getModifiedTime() = 0;
    virtual bool isModifiedTimeVisible() const = 0;
    virtual qint64 getNodeSize() = 0;
    virtual bool isFile() const = 0;

    bool isValid() const;

    QString getFilesAndFolders(int folders, int files);
    void updateSize();
    void updateModificationTime();

    bool eventFilter(QObject *watched, QEvent *event) override;

protected slots:
    void on_bAction_clicked();

private:
    Ui::DuplicatedNodeItem *ui;

    void setActionAndTitle(const QString& text);
};

/*
 * REMOTE IMPLEMENTATION CLASS
 * USE TO SHOW THE REMOTE NODE INFO
*/
class DuplicatedRemoteItem : public DuplicatedNodeItem
{
    Q_OBJECT
public:
    DuplicatedRemoteItem(QWidget *parent = nullptr);
    ~DuplicatedRemoteItem() = default;

    std::shared_ptr<mega::MegaNode> getNode();

protected:
    QString getNodeName() override;
    QDateTime getModifiedTime() override;
    qint64 getNodeSize() override;
    bool isFile() const override;
    bool isModifiedTimeVisible() const override;
};

/*
 * LOCAL IMPLEMENTATION CLASS
 * USE TO SHOW THE LOCAL NODE INFO
*/
class DuplicatedLocalItem : public DuplicatedNodeItem
{
    Q_OBJECT

public:
    explicit DuplicatedLocalItem(QWidget *parent = nullptr);
    virtual ~DuplicatedLocalItem() = default;

    const QString& getLocalPath();

protected:
    QString getNodeName() override;

    QDateTime getModifiedTime() override;
    bool isModifiedTimeVisible() const override;
    qint64 getNodeSize() override;
    bool isFile() const override;

private slots:
    void onNodeSizeFinished();
    void onNodeModificationTimeFinished();

private:
    void getFolderSize(const QString& path);
    void getFolderModifiedDate(const QString& path);
    QString getFullFileName(const QString& path, const QString& fileName);

    qint64 mNodeSize;
    bool mValidModificationTime;
    QDateTime mModificationTime;

    QFutureWatcher<void> mFolderSizeFuture;
    QFutureWatcher<void> mFolderModificationTimeFuture;
};

/*
 * RENAME IMPLEMENTATION FOR LOCAL NODES
 * USE TO SHOW THE RENAME OPTION
*/
class DuplicatedRenameItem : public DuplicatedLocalItem
{
    Q_OBJECT

public:
    explicit DuplicatedRenameItem(QWidget *parent = nullptr);
    ~DuplicatedRenameItem() = default;

    void setInfo(std::shared_ptr<DuplicatedNodeInfo> conflict);

protected:
    QString getNodeName() override;
};


#endif // DUPLICATEDNODEITEM_H
