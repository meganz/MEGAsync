#ifndef QSYNCITEMWIDGET_H
#define QSYNCITEMWIDGET_H

#include <QWidget>
#include "gui/MenuItemAction.h"
#include <megaapi.h>

#include "model/SyncModel.h"

namespace Ui {
class QSyncItemWidget;
}

class MegaApplication;

class QSyncItemWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
      LOCAL_FOLDER  = 0,
      REMOTE_FOLDER = 1,
    };
    explicit QSyncItemWidget(int itemType, QWidget* parent = nullptr);

    void setPath(const QString& path, const QString &name);
    void setPath(const QString& path);

    void setToolTip(const QString& tooltip);
    QString text();
    QString fullPath();
    void setError(int error);

    ~QSyncItemWidget();

    mega::MegaHandle mSyncRootHandle = mega::INVALID_HANDLE;

    void setSyncSetting(const std::shared_ptr<SyncSetting>& value);
    void setSelected(bool selected = true);

private slots:
    void onSyncStateChanged(std::shared_ptr<SyncSetting> syncSettings);
    void nodeChanged(mega::MegaHandle handle);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void enterEvent(QEvent* event) override;

    bool getIsCacheAvailable() const;
    void setIsCacheAvailable(bool value);

signals:
    void onSyncInfo();
    void onOpenLocalCache();
    void onDeleteSync();

private:
    Ui::QSyncItemWidget* mUi;

    bool mIsCacheAvailable;
    int mItemType;
    QString mFullPath;
    int mError;
    bool mSelected;

    QString mDisplayName;

    int64_t mLastRemotePathCheck;
    bool mNodesUpToDate;
    std::shared_ptr<SyncSetting> mSyncSetting;

    void elidePathLabel();
    void configureSyncTypeUI(int type) const;
};

#endif // QSYNCITEMWIDGET_H
