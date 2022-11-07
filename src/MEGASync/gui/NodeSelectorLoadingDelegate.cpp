#include "NodeSelectorLoadingDelegate.h"
#include "ui_NodeSelectorLoadingDelegate.h"

NodeSelectorLoadingDelegate::NodeSelectorLoadingDelegate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NodeSelectorLoadingDelegate)
{
    ui->setupUi(this);
}

NodeSelectorLoadingDelegate::~NodeSelectorLoadingDelegate()
{
    delete ui;
}

QSize NodeSelectorLoadingDelegate::widgetSize()
{
    return QSize(100, 26);
}
