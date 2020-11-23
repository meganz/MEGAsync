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
    explicit QSyncItemWidget(QWidget *parent = 0);

    void setText(const QString &path);
    void setToolTip(const QString &tooltip);
    QString text();
    void localCacheAvailable(bool op);

    ~QSyncItemWidget();

private slots:
    void on_bSyncOptions_clicked();

signals:
    void onOpenSync();
    void onOpenLocalCache();
    void onDeleteSync();

private:
    Ui::QSyncItemWidget *ui;

    QMenu *optionsMenu;
    MenuItemAction *openFolder;
    MenuItemAction *openDebris;
    MenuItemAction *deleteSync;
    bool isCacheAvailable;

};

#endif // QSYNCITEMWIDGET_H
