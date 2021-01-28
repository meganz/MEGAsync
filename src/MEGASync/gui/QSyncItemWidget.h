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
    void setPathAndName(const QString &path, const QString &name);
    void setPathAndGuessName(const QString &path);
    void setToolTip(const QString &tooltip);
    QString text();
    QString fullPath();
    void setError(int error);

    ~QSyncItemWidget();

    bool getIsCacheAvailable() const;
    void setIsCacheAvailable(bool value);

private slots:
    void on_bSyncOptions_clicked();
    void on_bReveal_clicked();

signals:
    void onSyncInfo();
    void onOpenLocalCache();
    void onDeleteSync();

private:
    Ui::QSyncItemWidget *ui;

    QMenu *mOptionsMenu;
    MenuItemAction *mSyncInfo;
    MenuItemAction *mOpenDebris;
    MenuItemAction *mDeleteSync;
    bool mIsCacheAvailable;
    int mItemType;
    QString mFullPath;
    int mError;

    void configureSyncTypeUI(int type) const;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // QSYNCITEMWIDGET_H
