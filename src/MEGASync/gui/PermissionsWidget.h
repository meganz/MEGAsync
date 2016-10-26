#ifndef PERMISSIONSWIDGET_H
#define PERMISSIONSWIDGET_H

#include <QWidget>

namespace Ui {
class PermissionsWidget;
}

class PermissionsWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
        None = 0,
        Execution = 1,
        Write = 2,
        Read = 4
    };

    enum {
      ENABLED_READ = 0x01,
      ENABLED_WRITE = 0x02,
      ENABLED_EXECUTION = 0x04
    };

    explicit PermissionsWidget(QWidget *parent = 0);
    void setDefaultPermissions(int permissions);
    int getCurrentPermissions();
    void configurePermissions(unsigned int enabledPermissions);
    ~PermissionsWidget();

signals:
    void onPermissionChanged();

private:
    Ui::PermissionsWidget *ui;
    int permissionState;
    void updatePermissions();

private slots:
    void on_cbRead_stateChanged(int state);
    void on_cbWrite_stateChanged(int state);
    void on_cbExecution_stateChanged(int state);
};

#endif // PERMISSIONSWIDGET_H
