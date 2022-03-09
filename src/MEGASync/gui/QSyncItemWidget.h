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
      NAME,
      RUN_STATE
    };
    explicit QSyncItemWidget(int itemType, QWidget* parent = nullptr);

    QString getLocalPath() const;
    QString getName() const;
    QString getRemotePath() const;
    QString getRunState() const;

    void setError(int error);
    void setLocalPath(const QString& path);
    void setName(const QString& name);
    void setRemotePath(const QString& path);
    void setRunState(const QString& runState);
    void setToolTip(const QString& tooltip);

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
    int mError;
    bool mSelected;

    // Meaningful only for NAME.
    QString mLocalPath;
    QString mName;
    QString mRemotePath;

    // Meaningful only for RUN_STATE.
    QString mRunState;

    int64_t mLastRemotePathCheck;
    bool mNodesUpToDate;
    std::shared_ptr<SyncSetting> mSyncSetting;

    void elideLabel();
};

#endif // QSYNCITEMWIDGET_H
