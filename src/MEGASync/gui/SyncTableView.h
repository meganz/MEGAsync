#ifndef SYNCTABLEVIEW_H
#define SYNCTABLEVIEW_H

#include "SyncController.h"

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

signals:
    void removeSync(std::shared_ptr<SyncSetting> sync);

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
    ~MenuItemDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

class IconMiddleDelegate : public QStyledItemDelegate
{
public:
    explicit IconMiddleDelegate(QObject *parent = nullptr);
    ~IconMiddleDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    void initStyleOption(QStyleOptionViewItem *option,
                                const QModelIndex &index) const override;
};


#endif // SYNCTABLEVIEW_H
