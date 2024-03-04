#ifndef SYNCTABLEVIEW_H
#define SYNCTABLEVIEW_H

#include "syncs/control/SyncController.h"

#include <QObject>
#include <QTableView>
#include <QStyledItemDelegate>

class SyncTableView : public QTableView
{
    Q_OBJECT

public:
    static const int FIXED_COLUMN_WIDTH = 32;
    SyncTableView(QWidget *parent = nullptr);

    mega::MegaSync::SyncType getType() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    virtual void initTable();
    void createStatesContextActions(QMenu* menu, std::shared_ptr<SyncSettings> sync);
    void showContextMenu(const QPoint &pos, const QModelIndex index);

    //Reimplemented methods for context menu
    virtual QString getRemoveActionString();
    virtual void removeActionClicked(std::shared_ptr<SyncSettings> settings);

    const char* mContextMenuName;
    mega::MegaSync::SyncType mType;

signals:
    void signalRemoveSync(std::shared_ptr<SyncSettings> sync);
    void signalRunSync(std::shared_ptr<SyncSettings> sync);
    void signalSuspendSync(std::shared_ptr<SyncSettings> sync);
    void signalOpenMegaignore(std::shared_ptr<SyncSettings> sync);
    void signaladdExclusions(std::shared_ptr<SyncSettings> sync);
    void signalRescanQuick(std::shared_ptr<SyncSettings> sync);
    void signalRescanDeep(std::shared_ptr<SyncSettings> sync);

private slots:
    virtual void onCustomContextMenuRequested(const QPoint& pos);
    virtual void onCellClicked(const QModelIndex &index);

private:
    SyncController mSyncController;
    bool mIsFirstTime;
};

class BackgroundColorDelegate : public QStyledItemDelegate
{
public:
    explicit BackgroundColorDelegate(QObject* parent = nullptr);
    ~BackgroundColorDelegate() = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class MenuItemDelegate : public BackgroundColorDelegate
{
public:
    explicit MenuItemDelegate(QObject *parent = nullptr);
    ~MenuItemDelegate() = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class IconMiddleDelegate : public BackgroundColorDelegate
{
public:
    explicit IconMiddleDelegate(QObject *parent = nullptr);
    ~IconMiddleDelegate() = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    void initStyleOption(QStyleOptionViewItem *option,
                                const QModelIndex &index) const override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);
};

class ElideMiddleDelegate : public BackgroundColorDelegate
{
public:
    explicit ElideMiddleDelegate(QObject *parent = nullptr);
    ~ElideMiddleDelegate() = default;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};


#endif // SYNCTABLEVIEW_H
