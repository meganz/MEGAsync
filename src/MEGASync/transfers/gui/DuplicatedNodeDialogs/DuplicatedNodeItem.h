#ifndef DUPLICATEDNODEITEM_H
#define DUPLICATEDNODEITEM_H

#include <megaapi.h>
#include <DuplicatedNodeDialogs/DuplicatedNodeInfo.h>
#include <control/FolderAttributes.h>

#include <QTMegaRequestListener.h>

#include <memory>

#include <QWidget>
#include <QFutureWatcher>
#include <QDateTime>
#include <QPointer>

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

    virtual void setInfo(std::shared_ptr<DuplicatedNodeInfo> info, NodeItemType type);

    NodeItemType getType(){return mType;}
    void setType(NodeItemType type);

    void setDescription(const QString& description);
    void showLearnMore(const QString& url);

signals:
    void actionClicked();

protected:
    NodeItemType mType;
    std::shared_ptr<DuplicatedNodeInfo> mInfo;
    qint64 mNodeSize;
    QFutureWatcher<qint64> mFolderSizeFuture;

    void fillUi();
    virtual QString getNodeName() = 0;
    virtual void getModifiedTime() = 0;
    virtual void getNodeSize() = 0;
    virtual bool isFile() const = 0;

    void setModifiedTime(QDateTime&& dateTime);
    void setSize(qint64 size);

    bool isValid() const;

    void updateSize();
    void updateModificationTime();

    bool eventFilter(QObject *watched, QEvent *event) override;

protected slots:
    void on_bAction_clicked();
    void onNodeSizeFinished();

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
    ~DuplicatedRemoteItem();

    void setInfo(std::shared_ptr<DuplicatedNodeInfo> info, NodeItemType type) override;
    std::shared_ptr<mega::MegaNode> getNode();

protected:
    QString getNodeName() override;
    void getModifiedTime() override;
    void getNodeSize() override;
    bool isFile() const override;

private:
    mega::QTMegaRequestListener* mListener;
    QPointer<RemoteFolderAttributes> mFolderAttributes;
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
    virtual ~DuplicatedLocalItem();

    void setInfo(std::shared_ptr<DuplicatedNodeInfo> info, NodeItemType type) override;
    const QString& getLocalPath();

protected:
    QString getNodeName() override;

    void getModifiedTime() override;
    void getNodeSize() override;
    bool isFile() const override;

private:
    QString getFullFileName(const QString& path, const QString& fileName);

    QPointer<LocalFolderAttributes> mFolderAttributes;
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
