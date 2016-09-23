#include "PermissionsWidget.h"
#include "ui_PermissionsWidget.h"

PermissionsWidget::PermissionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PermissionsWidget)
{
    ui->setupUi(this);
    configurePermissions(ENABLED_READ | ENABLED_WRITE | ENABLED_EXECUTION);
}

void PermissionsWidget::setDefaultPermissions(int permissions)
{
    this->permissionState = permissions;
    updatePermissions();
}

void PermissionsWidget::updatePermissions()
{
    (permissionState & Read) ? ui->cbRead->setChecked(true) : ui->cbRead->setChecked(false);
    (permissionState & Write)  ? ui->cbWrite->setChecked(true) : ui->cbWrite->setChecked(false);
    (permissionState & Execution) ? ui->cbExecution->setChecked(true) : ui->cbExecution->setChecked(false);

    emit onPermissionChanged();
}

void PermissionsWidget::configurePermissions(unsigned int enabledPermissions)
{
    if (enabledPermissions & ENABLED_READ)
    {
        ui->cbRead->setEnabled(true);
    }
    else
    {
        ui->cbRead->setEnabled(false);
    }

    if (enabledPermissions & ENABLED_WRITE)
    {
        ui->cbWrite->setEnabled(true);
    }
    else
    {
        ui->cbWrite->setEnabled(false);
    }

    if (enabledPermissions & ENABLED_EXECUTION)
    {
        ui->cbExecution->setEnabled(true);
    }
    else
    {
        ui->cbExecution->setEnabled(false);
    }
}

int PermissionsWidget::getCurrentPermissions()
{
    return permissionState;
}

PermissionsWidget::~PermissionsWidget()
{
    delete ui;
}

void PermissionsWidget::on_cbRead_stateChanged(int state)
{
    switch(state)
    {
        case Qt::Unchecked:
            permissionState &= ~Read;
        break;
        case Qt::Checked:
            permissionState |=  Read;
        break;
        default:
        break;
    }

    emit onPermissionChanged();
}

void PermissionsWidget::on_cbWrite_stateChanged(int state)
{
    switch(state)
    {
        case Qt::Unchecked:
            permissionState &= ~Write;
        break;
        case Qt::Checked:
            permissionState |=  Write;
        break;
        default:
        break;
    }
    emit onPermissionChanged();

}

void PermissionsWidget::on_cbExecution_stateChanged(int state)
{
    switch(state)
    {
        case Qt::Unchecked:
            permissionState &= ~Execution;
        break;
        case Qt::Checked:
            permissionState |=  Execution;
        break;
        default:
        break;
    }
    emit onPermissionChanged();

}
