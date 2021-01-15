#ifndef QSYNCITEMWIDGET_H
#define QSYNCITEMWIDGET_H

#include <QWidget>
#include <QMenu>

namespace Ui {
class QSyncItemWidget;
}

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

private:
    Ui::QSyncItemWidget *ui;
    QString mFullPath;
    int error;
};

#endif // QSYNCITEMWIDGET_H
