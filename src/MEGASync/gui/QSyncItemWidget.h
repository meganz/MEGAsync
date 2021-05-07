#ifndef QSYNCITEMWIDGET_H
#define QSYNCITEMWIDGET_H

#include <QWidget>
#include <QMenu>
#include "gui/MenuItemAction.h"
#include <megaapi.h>

#include "model/Model.h"

namespace Ui {
class QSyncItemWidget;
}

class MegaApplication;

class QSyncItemWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
      LOCAL_FOLDER = 0,
      REMOTE_FOLDER = 1
    };
    explicit QSyncItemWidget(int itemType, QWidget *parent = nullptr);

    void setPath(const QString &path, const QString &name);
    void setPath(const QString &path);

    void setToolTip(const QString &tooltip);
    QString text();
    QString fullPath();
    void setError(int error);

    ~QSyncItemWidget();

    mega::MegaHandle mSyncRootHandle = mega::INVALID_HANDLE;

    void setSyncSetting(const std::shared_ptr<SyncSetting> &value);

private slots:
    void onSyncStateChanged(std::shared_ptr<SyncSetting> syncSettings);
    void nodeChanged(mega::MegaHandle handle);
    void on_bSyncOptions_clicked();
    void on_bReveal_clicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool event(QEvent* event) override;

    bool getIsCacheAvailable() const;
    void setIsCacheAvailable(bool value);

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

    QString mDisplayName;

    int64_t mLastRemotePathCheck = 0;
    bool mNodesUpToDate = true;
    std::shared_ptr<SyncSetting> mSyncSetting;

    void elidePathLabel();
    void configureSyncTypeUI(int type) const;
};

#endif // QSYNCITEMWIDGET_H
