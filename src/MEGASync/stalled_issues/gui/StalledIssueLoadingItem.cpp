#include "StalledIssueLoadingItem.h"
#include "ui_StalledIssueLoadingItem.h"

#include <StalledIssueHeader.h>

const int LOADINGITEM_HEIGHT = 50; /*check ui*/

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
    //500 is no taken into account as it changes depending on the stalled issue dialog
    return QSize(500, LOADINGITEM_HEIGHT);
}
