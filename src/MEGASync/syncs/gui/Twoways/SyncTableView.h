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

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    virtual void initTable();
    void createStatesContextActions(QMenu* menu, std::shared_ptr<SyncSettings> sync);

signals:
    void signalRemoveSync(std::shared_ptr<SyncSettings> sync);
    void signalRunSync(std::shared_ptr<SyncSettings> sync);
    void signalPauseSync(std::shared_ptr<SyncSettings> sync);
    void signalSuspendSync(std::shared_ptr<SyncSettings> sync);
    void signalDisableSync(std::shared_ptr<SyncSettings> sync);
    void signalOpenMegaignore(std::shared_ptr<SyncSettings> sync);
    void signalRescanQuick(std::shared_ptr<SyncSettings> sync);
    void signalRescanDeep(std::shared_ptr<SyncSettings> sync);

private slots:
    virtual void onCustomContextMenuRequested(const QPoint& pos);
    virtual void onCellClicked(const QModelIndex &index);

private:
    void showContextMenu(const QPoint &pos, const QModelIndex index);

    SyncController mSyncController;
    bool mIsFirstTime;
};

class MenuItemDelegate : public QStyledItemDelegate
{
public:
    explicit MenuItemDelegate(QObject *parent = nullptr);
    ~MenuItemDelegate() = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class IconMiddleDelegate : public QStyledItemDelegate
{
public:
    explicit IconMiddleDelegate(QObject *parent = nullptr);
    ~IconMiddleDelegate() = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    void initStyleOption(QStyleOptionViewItem *option,
                                const QModelIndex &index) const override;
};

class ElideMiddleDelegate : public QStyledItemDelegate
{
public:
    explicit ElideMiddleDelegate(QObject *parent = nullptr);
    ~ElideMiddleDelegate() = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};


#endif // SYNCTABLEVIEW_H
