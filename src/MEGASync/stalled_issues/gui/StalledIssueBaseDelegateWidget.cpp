#include "StalledIssueBaseDelegateWidget.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "StalledIssueDelegate.h"
#include "StalledIssuesDialog.h"
#include "WordWrapLabel.h"

#include <QFile>
#include <QtConcurrent/QtConcurrent>

StalledIssueBaseDelegateWidget::StalledIssueBaseDelegateWidget(QWidget *parent)
    : QWidget(parent),
      mDelegate(nullptr)
{
    connect(&mResizeNeedTimer, &QTimer::timeout, this, [this](){
        checkForSizeHintChanges();
    });
    mResizeNeedTimer.setInterval(50);
    mResizeNeedTimer.setSingleShot(true);
}

void StalledIssueBaseDelegateWidget::updateIndex()
{
    if(auto view = dynamic_cast<QWidget*>(parent()))
    {
        view->update();
    }
}

void StalledIssueBaseDelegateWidget::render(const QStyleOptionViewItem &,
            QPainter *painter,
            const QRegion &sourceRegion)
{
    QWidget::render(painter,QPoint(0,0),sourceRegion);
}

void StalledIssueBaseDelegateWidget::updateUi(const QModelIndex& index, const StalledIssueVariant & issueData)
{
    MegaSyncApp->getStalledIssuesModel()->UiItemUpdate(mCurrentIndex, index);

    if(mCurrentIndex != index)
    {
        mCurrentIndex = QPersistentModelIndex(index);
        mData = issueData;
    }

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

void StalledIssueBaseDelegateWidget::reset()
{
    mData.reset();
    mCurrentIndex = QModelIndex();
}

QSize StalledIssueBaseDelegateWidget::sizeHint() const
{
    StalledIssue::Type sizeType = isHeader() ? StalledIssue::Header : StalledIssue::Body;

    QSize size;

    if(mData.getDelegateSize(sizeType).isValid())
    {
        size = mData.getDelegateSize(sizeType);
    }

    if(!size.isValid())
    {
        size = QWidget::sizeHint();
        mData.setDelegateSize(size, sizeType);
    }

    return size;
}

bool StalledIssueBaseDelegateWidget::isHeader() const
{
    return mCurrentIndex.isValid() && !mCurrentIndex.parent().isValid();
}

void StalledIssueBaseDelegateWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

bool StalledIssueBaseDelegateWidget::event(QEvent *event)
{
    if(event->type() == WordWrapLabel::HeightAdapted)
    {
        mResizeNeedTimer.start();
    }

    return QWidget::event(event);
}

void StalledIssueBaseDelegateWidget::checkForSizeHintChanges()
{
    if(mDelegate && mData.consultData())
    {
        layout()->activate();
        updateGeometry();

        mData.removeDelegateSize(StalledIssue::Header);
        mData.removeDelegateSize(StalledIssue::Body);

        //Update sizeHint cache
        sizeHint();

        if(auto stalledDelegate = dynamic_cast<StalledIssueDelegate*>(mDelegate))
        {
            stalledDelegate->updateSizeHint();
        }
    }
}

void StalledIssueBaseDelegateWidget::setDelegate(QStyledItemDelegate* newDelegate)
{
    mDelegate = newDelegate;
    if(auto stalledDelegate = dynamic_cast<StalledIssueDelegate*>(mDelegate))
    {
        connect(this,
            &StalledIssueBaseDelegateWidget::needsUpdate,
            stalledDelegate,
            &StalledIssueDelegate::updateView);
    }
}

void StalledIssueBaseDelegateWidget::updateSizeHint()
{
    mResizeNeedTimer.start();
}

bool StalledIssueBaseDelegateWidget::checkForExternalChanges(bool isSingleSelection)
{
    if(isSingleSelection && MegaSyncApp->getStalledIssuesModel()->checkForExternalChanges(getCurrentIndex()))
    {
        updateSizeHint();
        return true;
    }

    return false;
}

bool StalledIssueBaseDelegateWidget::checkSelection(const QList<mega::MegaSyncStall::SyncStallReason>& reasons,
    SelectionInfo& info)
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
    info.selection = dialog->getDialog()->getSelection(reasons);

    if(checkForExternalChanges(info.selection.size() == 1))
    {
        return false;
    }

    info.msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    info.msgInfo.title = MegaSyncApp->getMEGAString();
    info.msgInfo.textFormat = Qt::RichText;
    info.msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;

    info.msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;

    //In order to keep the same context as before
    textsByButton.insert(QMessageBox::No, QCoreApplication::translate("LocalAndRemoteDifferentWidget", "Cancel"));
    textsByButton.insert(QMessageBox::Ok, QCoreApplication::translate("LocalAndRemoteDifferentWidget", "Apply"));

    info.similarSelection = MegaSyncApp->getStalledIssuesModel()->getIssuesByReason(reasons);
    if(info.similarSelection.size() != info.selection.size())
    {
        auto checkBox = new QCheckBox(QCoreApplication::translate("LocalAndRemoteDifferentWidget", "Apply to all"));
        info.msgInfo.checkBox = checkBox;
    }
    info.msgInfo.buttonsText = textsByButton;

    return true;
}
