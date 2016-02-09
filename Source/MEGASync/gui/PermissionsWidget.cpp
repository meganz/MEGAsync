#include "PermissionsWidget.h"
#include "ui_PermissionsWidget.h"

PermissionsWidget::PermissionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PermissionsWidget)
{
    ui->setupUi(this);
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
