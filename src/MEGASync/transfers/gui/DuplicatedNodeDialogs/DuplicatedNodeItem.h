#ifndef DUPLICATEDNODEITEM_H
#define DUPLICATEDNODEITEM_H

#include <megaapi.h>
#include <DuplicatedNodeDialogs/DuplicatedNodeInfo.h>

#include <QTMegaRequestListener.h>

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
    virtual QDateTime getModifiedTime() = 0;
    virtual qint64 getNodeSize() = 0;
    virtual bool isFile() const = 0;

    bool isModifiedTimeVisible();
    void setModifiedTimeVisible(bool state);

    bool isValid() const;

    void updateSize();
    void updateModificationTime();

    bool eventFilter(QObject *watched, QEvent *event) override;

protected slots:
    void on_bAction_clicked();
    void onNodeSizeFinished();

private:
    Ui::DuplicatedNodeItem *ui;
    bool mModifiedTimeVisible;

    void setActionAndTitle(const QString& text);
};

/*
 * REMOTE IMPLEMENTATION CLASS
 * USE TO SHOW THE REMOTE NODE INFO
*/
class DuplicatedRemoteItem : public DuplicatedNodeItem, public mega::MegaRequestListener
{
    Q_OBJECT
public:
    DuplicatedRemoteItem(QWidget *parent = nullptr);
    ~DuplicatedRemoteItem();

    std::shared_ptr<mega::MegaNode> getNode();

protected:
    QString getNodeName() override;
    QDateTime getModifiedTime() override;
    qint64 getNodeSize() override;
    bool isFile() const override;
    void onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e) override;

private:
    mega::QTMegaRequestListener* mListener;
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
    qint64 getNodeSize() override;
    bool isFile() const override;

private slots:
    void onNodeModificationTimeFinished();

private:
    qint64 getFolderSize(const QString& path, qint64 size);

    QDateTime getFolderModifiedDate(const QString& path, const QDateTime& date);
    QString getFullFileName(const QString& path, const QString& fileName);

    QDateTime mModificationTime;
    QFutureWatcher<QDateTime> mFolderModificationTimeFuture;
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
