#ifndef QSYNCITEMWIDGET_H
#define QSYNCITEMWIDGET_H

#include <QWidget>
#include <QMenu>
#include "gui/MenuItemAction.h"

namespace Ui {
class QSyncItemWidget;
}

class QSyncItemWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
      LOCAL_FOLDER = 0,
      REMOTE_FOLDER = 1
    };
    explicit QSyncItemWidget(int itemType, QWidget *parent = nullptr);

    void setText(const QString &path);
    void setToolTip(const QString &tooltip);
    QString text();
    void localCacheAvailable(bool op);

    ~QSyncItemWidget();

private slots:
    void on_bSyncOptions_clicked();
    void on_bReveal_clicked();

signals:
    void onSyncInfo();
    void onOpenLocalCache();
    void onDeleteSync();

private:
    Ui::QSyncItemWidget *ui;

    QMenu *optionsMenu;
    MenuItemAction *syncInfo;
    MenuItemAction *openDebris;
    MenuItemAction *deleteSync;
    bool isCacheAvailable;
    int itemType;
    QString fullPath;

    void configureSyncTypeUI(int type) const;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // QSYNCITEMWIDGET_H
