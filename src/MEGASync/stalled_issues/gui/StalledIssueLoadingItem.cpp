#include "StalledIssueLoadingItem.h"
#include "ui_StalledIssueLoadingItem.h"

StalledIssueLoadingItem::StalledIssueLoadingItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueLoadingItem)
{
    ui->setupUi(this);
}

StalledIssueLoadingItem::~StalledIssueLoadingItem()
{
    delete ui;
}

QSize StalledIssueLoadingItem::widgetSize()
{
    return QSize(500, 64);
}
