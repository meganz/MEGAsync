#ifndef QSYNCITEMWIDGET_H
#define QSYNCITEMWIDGET_H

#include <QWidget>
#include <QMenu>
#include <megaapi.h>

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

private:
    void elidePathLabel();

protected:
    void resizeEvent(QResizeEvent *event) override;

    bool event(QEvent* event) override;

private:
    Ui::QSyncItemWidget *ui;
    QString mFullPath;
    QString mSyncName;
    int error;
    QString mOriginalPath;
};

#endif // QSYNCITEMWIDGET_H
