#include "StalledIssueBaseDelegateWidget.h"

#include "MegaApplication.h"

#include "mega/types.h"

#include <QtConcurrent/QtConcurrent>
#include <QFile>

StalledIssueBaseDelegateWidget::StalledIssueBaseDelegateWidget(QWidget *parent)
    : QWidget(parent),
      mIsSolved(false),
      mKeepEditor(false)
{
}

void StalledIssueBaseDelegateWidget::render(const QStyleOptionViewItem &,
            QPainter *painter,
            const QRegion &sourceRegion)
{
    QWidget::render(painter,QPoint(0,0),sourceRegion);
}
void StalledIssueBaseDelegateWidget::updateUi(const QModelIndex& index, const StalledIssueVariant &data)
{
    mCurrentIndex = QPersistentModelIndex(index);
    mData = data;

    refreshUi();
}

QModelIndex StalledIssueBaseDelegateWidget::getCurrentIndex() const
{
    return mCurrentIndex;
}

const StalledIssueVariant& StalledIssueBaseDelegateWidget::getData() const
{
    return mData;
}

bool StalledIssueBaseDelegateWidget::keepEditor() const
{
    return mKeepEditor;
}

void StalledIssueBaseDelegateWidget::setKeepEditor(bool newKeepEditor)
{
    if(mKeepEditor != newKeepEditor)
    {
        mKeepEditor = newKeepEditor;
        emit editorKeepStateChanged(mKeepEditor);
    }
}
