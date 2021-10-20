#include "SyncTreeWidget.h"

#include <QHeaderView>

SyncTreeWidget::SyncTreeWidget(QWidget *parent): QTreeView(parent)
{
    setIconSize(QSize(16, 16));
}
