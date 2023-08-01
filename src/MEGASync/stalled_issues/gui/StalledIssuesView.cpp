#include "StalledIssuesView.h"

#include <MegaApplication.h>
#include <StalledIssuesModel.h>

#include <QHeaderView>
#include <QScrollBar>
#include <QApplication>

const int SCROLL_STOP_THRESHOLD = 50;

StalledIssuesView::StalledIssuesView(QWidget *parent)
    :  LoadingSceneView<StalledIssueLoadingItem, QTreeView>(parent)
{
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &StalledIssuesView::onScrollMoved);
    mScrollStop.setSingleShot(true);
    connect(&mScrollStop, &QTimer::timeout, [this](){
        emit scrollStopped();
    });
}

void StalledIssuesView::onScrollMoved()
{
    mScrollStop.start(SCROLL_STOP_THRESHOLD);
}

void StalledIssuesView::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QPersistentModelIndex index = indexAt(pos);

    if(!index.parent().isValid())
    {
        QItemSelectionModel::SelectionFlags command = selectionCommand(index, event);
        selectionModel()->select(index, command);
    }
    else
    {
        QItemSelectionModel::SelectionFlags command = selectionCommand(index.parent(), event);
        selectionModel()->select(index.parent(), command);
    }

    LoadingSceneView<StalledIssueLoadingItem, QTreeView>::mousePressEvent(event);
}

void StalledIssuesView::keyPressEvent(QKeyEvent *event)
{
    Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();

#ifdef Q_OS_MACOS
    if (modifiers.testFlag(Qt::MetaModifier))
    {
#else
    if (modifiers.testFlag(Qt::ControlModifier))
    {
#endif
        if(event->key() == Qt::Key_Minus)
        {
            emit MegaSyncApp->getStalledIssuesModel()->showRawInfo(false);
        }
        else if(event->key() == Qt::Key_Plus)
        {
            emit MegaSyncApp->getStalledIssuesModel()->showRawInfo(true);
        }

        viewport()->update();
    }
    else
    {
        LoadingSceneView<StalledIssueLoadingItem, QTreeView>::keyPressEvent(event);
    }
}
