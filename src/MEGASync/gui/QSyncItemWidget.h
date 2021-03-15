#ifndef QSYNCITEMWIDGET_H
#define QSYNCITEMWIDGET_H

#include <QWidget>
#include <QMenu>
#include <megaapi.h>

#include "Model.h"

namespace Ui {
class QSyncItemWidget;
}

class MegaApplication;

class QSyncItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QSyncItemWidget(QWidget *parent = nullptr);

    void setPathAndName(const QString &path, const QString &name);
    void setPathAndGuessName(const QString &path);

    void setToolTip(const QString &tooltip);
    void setError(int error);
    QString fullPath();

    ~QSyncItemWidget();

    mega::MegaHandle mSyncRootHandle = mega::INVALID_HANDLE;
    MegaApplication* app;

    void setSyncSetting(const std::shared_ptr<SyncSetting> &value);

private:
    void elidePathLabel();


private slots:
    void onSyncStateChanged(std::shared_ptr<SyncSetting> syncSettings);
    void nodeChanged(mega::MegaHandle handle);

protected:
    void resizeEvent(QResizeEvent *event) override;

    bool event(QEvent* event) override;

private:
    Ui::QSyncItemWidget *ui;
    QString mFullPath;
    QString mSyncName;
    int error;
    QString mOriginalPath;

    int64_t mLastRemotePathCheck = 0;
    bool mNodesUpToDate = true;
    std::shared_ptr<SyncSetting> mSyncSetting;
};

#endif // QSYNCITEMWIDGET_H
