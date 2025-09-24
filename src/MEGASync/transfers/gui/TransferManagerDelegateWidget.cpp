#include "TransferManagerDelegateWidget.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "MegaTransferView.h"
#include "ThemeManager.h"
#include "TransferWidgetColumnsManager.h"
#include "ui_TransferManagerDelegateWidget.h"
#include "Utilities.h"
#include <TokenParserWidgetManager.h>

#include <QMouseEvent>
#include <QPainterPath>

constexpr uint PB_PRECISION = 1000;

using namespace mega;

TransferManagerDelegateWidget::TransferManagerDelegateWidget(QWidget *parent) :
    TransferBaseDelegateWidget (parent),
    mUi (new Ui::TransferManagerDelegateWidget)
{
    mUi->setupUi(this);
    mUi->pbTransfer->setMaximum(PB_PRECISION);

    //For elided texts

    mUi->wTransferName->installEventFilter(this);

    mUi->lItemPausedQueued_1->installEventFilter(this);
    mUi->lItemFailed->installEventFilter(this);
    mUi->lItemPaused->installEventFilter(this);
    mUi->lRetryMsg->installEventFilter(this);
    mUi->lItemFailed->installEventFilter(this);
    mUi->tItemRetry->installEventFilter(this);
    mUi->lItemStatus->installEventFilter(this);
    mUi->lDone->installEventFilter(this);
    mUi->lTotal->installEventFilter(this);
    mUi->sStatus->installEventFilter(this);

    setProperty("TOKENIZED", true);
    TokenParserWidgetManager::instance()->polish(this);
}

TransferManagerDelegateWidget::~TransferManagerDelegateWidget()
{
    delete mUi;
}

void TransferManagerDelegateWidget::updateTransferState()
{
    QIcon icon;
    QString speedString;
    QString pauseResumeTooltip;
    QString cancelClearTooltip;
    bool showTPauseResume(true);
    bool showTCancelClear(true);
    QString timeString;
    QString timeTooltip;
    QString statusString;

    auto state = getData()->getState();

    // Set values according to transfer state
    switch (state)
    {
        case TransferData::TRANSFER_ACTIVE:
        {
            if (getData()->mTransferredBytes == 0)
            {
                statusString = getState(TRANSFER_STATES::STATE_STARTING);
            }
            else
            {
                switch (getData()->mType)
                {
                    case TransferData::TRANSFER_DOWNLOAD:
                    case TransferData::TRANSFER_LTCPDOWNLOAD:
                    {
                        statusString = getState(TRANSFER_STATES::STATE_DOWNLOADING);
                        break;
                    }
                    case TransferData::TRANSFER_UPLOAD:
                    {
                        statusString = getState(TRANSFER_STATES::STATE_UPLOADING);
                        break;
                    }
                    default:
                    {
                        statusString = getState(TRANSFER_STATES::STATE_SYNCING);
                        break;
                    }
                }
            }

            if(stateHasChanged())
            {
                mPauseResumeTransferDefaultIconName = QLatin1String(":images/transfer_manager/transfers_actions/lists_pause_ico_default.png");
                pauseResumeTooltip = MegaTransferView::pauseActionText(1); //Use singular form
                cancelClearTooltip = MegaTransferView::cancelActionText(1); //Use singular form
                mUi->wProgressBar->setVisible(true);
                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }

            timeString = getData()->mSpeed == 0 ?
                             timeString
                           : Utilities::getTimeString(getData()->mRemainingTime);

            if(getData()->mTotalSize == getData()->mTransferredBytes)
            {
                speedString = QString::fromUtf8("…");
            }
            else
            {
                speedString = Utilities::getSizeString(getData()->mSpeed)
                        + QLatin1String("/s");
            }

            break;
        }
        case TransferData::TRANSFER_PAUSED:
        {
            if(stateHasChanged())
            {
                mPauseResumeTransferDefaultIconName = QLatin1String(":images/transfer_manager/transfers_actions/lists_pause_ico_selected.png");
                pauseResumeTooltip = MegaTransferView::resumeActionText(1); //Use singular form
                cancelClearTooltip = MegaTransferView::cancelActionText(1); //Use singular form
                mUi->wProgressBar->setVisible(true);

                if(getData()->mTransferredBytes != 0)
                {
                    mUi->lItemPaused->setText(getState(TRANSFER_STATES::STATE_PAUSED));
                    mUi->lItemPaused->setToolTip(getState(TRANSFER_STATES::STATE_PAUSED));
                    mUi->sStatus->setCurrentWidget(mUi->pPaused);
                }
                else
                {
                    mUi->lItemPausedQueued_1->setText(getState(TRANSFER_STATES::STATE_PAUSED));
                    mUi->lItemPausedQueued_2->setText(getState(TRANSFER_STATES::STATE_INQUEUE_PARENTHESIS));
                    mUi->lItemPausedQueued_1->setToolTip(getState(TRANSFER_STATES::STATE_PAUSED));
                    mUi->lItemPausedQueued_2->setToolTip(getState(TRANSFER_STATES::STATE_INQUEUE_PARENTHESIS));
                    mUi->sStatus->setCurrentWidget(mUi->pPausedQueued);
                }
            }

            speedString = QString::fromUtf8("…");

            break;
        }
        case TransferData::TRANSFER_QUEUED:
        {
            if(stateHasChanged())
            {
                mPauseResumeTransferDefaultIconName = QLatin1String(":images/transfer_manager/transfers_actions/lists_pause_ico_default.png");
                pauseResumeTooltip = MegaTransferView::pauseActionText(1); //Use singular form
                cancelClearTooltip = MegaTransferView::cancelActionText(1); //Use singular form
                mUi->wProgressBar->setVisible(true);
                mUi->sStatus->setCurrentWidget(mUi->pQueued);

                if(getData()->mErrorCode == MegaError::API_EOVERQUOTA)
                {
                    QString retryMsg (getData()->mErrorValue ? getState(TRANSFER_STATES::STATE_OUT_OF_TRANSFER_QUOTA)
                                                        : getState(TRANSFER_STATES::STATE_OUT_OF_STORAGE_SPACE));
                    mUi->lRetryMsg->setText(retryMsg);
                    mUi->lRetryMsg->setToolTip(retryMsg);
                    mUi->sStatus->setCurrentWidget(mUi->pRetry);
                }
                else
                {
                    mUi->lItemQueued->setText(getState(TRANSFER_STATES::STATE_INQUEUE));
                    mUi->lItemQueued->setToolTip(getState(TRANSFER_STATES::STATE_INQUEUE));
                    mUi->lRetryMsg->setText(getState(TRANSFER_STATES::STATE_RETRY));
                    mUi->lRetryMsg->setToolTip(getState(TRANSFER_STATES::STATE_RETRY));
                }
            }

            break;
        }
        case TransferData::TRANSFER_CANCELLED:
        {
            //Cancelled transfers are immediately removed from the model
            break;
        }
        case TransferData::TRANSFER_COMPLETING:
        {
            statusString = getState(TRANSFER_STATES::STATE_COMPLETING);

            if(stateHasChanged())
            {
                showTPauseResume = false;
                showTCancelClear = false;
                mUi->wProgressBar->setVisible(true);
                mUi->sStatus->setCurrentWidget(mUi->pActive);
                mPauseResumeTransferDefaultIconName.clear();
            }

            speedString = QString::fromUtf8("…");

            break;
        }
        case TransferData::TRANSFER_FAILED:
        {
            if(stateHasChanged())
            {
                mPauseResumeTransferDefaultIconName.clear();
                mUi->sStatus->setCurrentWidget(mUi->pFailed);
                mUi->tItemRetry->setVisible(getData()->canBeRetried());
                mUi->tItemRetry->setText(getState(TRANSFER_STATES::STATE_RETRY));
                mUi->tItemRetry->setToolTip(getState(TRANSFER_STATES::STATE_RETRY));
                mUi->wProgressBar->setVisible(false);
                cancelClearTooltip = MegaTransferView::cancelActionText(1); //Use singular form
                mUi->lItemFailed->setText(
                    (mColumnManager &&
                     mColumnManager->getCurrentTab() == TransfersWidget::TM_TAB::FAILED_TAB) ?
                        getErrorText() :
                        getState(TRANSFER_STATES::STATE_FAILED));
                mUi->lItemFailed->setToolTip(getErrorText());
                showTPauseResume = false;
            }

            auto dateTime = getData()->getFinishedDateTime();
            timeString = MegaSyncApp->getFormattedDateByCurrentLanguage(dateTime, QLocale::FormatType::ShortFormat);

            timeTooltip = getData()->getFullFormattedFinishedTime();
            speedString = QString::fromUtf8("…");

            break;
        }
        case TransferData::TRANSFER_RETRYING:
        {
            statusString = getState(TRANSFER_STATES::STATE_RETRYING);

            if(stateHasChanged())
            {
                mPauseResumeTransferDefaultIconName = QLatin1String(":images/transfer_manager/transfers_actions/lists_pause_ico_default.png");
                pauseResumeTooltip = MegaTransferView::pauseActionText(1); //Use singular form
                cancelClearTooltip = MegaTransferView::cancelActionText(1); //Use singular form
                mUi->lItemStatus->setToolTip(getErrorText());
                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }

            if(getData()->mErrorCode == MegaError::API_EOVERQUOTA)
            {
                QString retryMsg (getData()->mErrorValue ? getState(TRANSFER_STATES::STATE_OUT_OF_TRANSFER_QUOTA)
                                                    : getState(TRANSFER_STATES::STATE_OUT_OF_STORAGE_SPACE));
                mUi->lRetryMsg->setText(retryMsg);
                mUi->lRetryMsg->setToolTip(retryMsg);
                mUi->sStatus->setCurrentWidget(mUi->pRetry);
            }
            break;
        }
        case TransferData::TRANSFER_COMPLETED:
        {
            statusString = getState(TRANSFER_STATES::STATE_COMPLETED);

            if(stateHasChanged())
            {
                cancelClearTooltip = MegaTransferView::clearActionText(1); //Use singular form
                showTPauseResume = false;
                mUi->wProgressBar->setVisible(false);
                mPauseResumeTransferDefaultIconName.clear();

                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }
            speedString = Utilities::getSizeString(getData()->mSpeed) + QLatin1String("/s");
            auto dateTime = getData()->getFinishedDateTime();
            timeString = MegaSyncApp->getFormattedDateByCurrentLanguage(dateTime, QLocale::FormatType::ShortFormat);

            timeTooltip = getData()->getFullFormattedFinishedTime();
            break;
        }
        default:
        break;
    }

    if(stateHasChanged())
    {
        // Pause/Resume button
        if (showTPauseResume)
        {
            icon = Utilities::getCachedPixmap(mPauseResumeTransferDefaultIconName);
            mUi->tPauseResumeTransfer->setIcon(icon);
            mUi->tPauseResumeTransfer->setToolTip(pauseResumeTooltip);
        }
        mUi->tPauseResumeTransfer->setVisible(showTPauseResume);

        // Cancel/Clear Button
        if ((getData()->mType & TransferData::TRANSFER_SYNC)
                && !(getData()->getState() & TransferData::TRANSFER_COMPLETED))
        {
            showTCancelClear = false;
        }
        if (showTCancelClear)
        {
            mUi->tCancelClearTransfer->setToolTip(cancelClearTooltip);
        }
        mUi->tCancelClearTransfer->setVisible(showTCancelClear);

        mUi->lDone->setVisible(!(getData()->getState() & TransferData::FINISHED_STATES_MASK));

        //Update action icons (for example, when the transfer changes from active to completed)
        mouseHoverTransfer(false, QPoint(0,0));
    }

    //Status
    mUi->lItemStatus->setText(statusString);
    mUi->lItemStatus->setToolTip(statusString);

    // Done label
    auto transferedB (getData()->mTransferredBytes);
    auto totalB (getData()->mTotalSize);

    auto sizes = Utilities::getProgressSizes(transferedB, totalB);

    mUi->lDone->setText(sizes.transferredBytes + QLatin1String(" ") + QLatin1String("/"));
    mUi->lTotal->setText(QLatin1String(" ") + sizes.totalBytes + QLatin1String(" ") + sizes.units);

    // Progress bar
    int permil = getData()->getState() & (TransferData::TRANSFER_COMPLETED | TransferData::TRANSFER_COMPLETING) ?
                     PB_PRECISION
                   : totalB > 0 ? Utilities::partPer(transferedB, totalB, PB_PRECISION)
                                : 0;
    mUi->pbTransfer->setValue(permil);

    // Speed
    mUi->bItemSpeed->setText(speedString);

    // Remaining or finished time
    mUi->lItemTime->setText(timeString);
    mUi->lItemTime->setToolTip(timeTooltip);


    mUi->lSyncIcon->setVisible(getData()->isSyncTransfer());
    if(getData()->isSyncTransfer())
    {
        auto syncIcon = Utilities::getCachedPixmap(QLatin1String(":/images/transfer_manager/transfers_states/synching_ico.png"));
        mUi->lSyncIcon->setPixmap(syncIcon.pixmap(mUi->lSyncIcon->size()));
    }
}

void TransferManagerDelegateWidget::setFileNameAndType()
{
    // Update members
    QIcon icon;
    // File type icon
    icon = Utilities::getCachedPixmap(
        Utilities::getExtensionPixmapName(getData()->mFilename, Utilities::AttributeType::MEDIUM));
    mUi->tFileType->setIcon(icon);

    // File name
    mUi->lTransferName->setToolTip(getData()->mFilename.toHtmlEscaped());
    adjustFileName();
}

void TransferManagerDelegateWidget::setType()
{
    QIcon icon;

    auto transferType = getData()->mType;

    if(transferType & TransferData::TRANSFER_DOWNLOAD || transferType & TransferData::TRANSFER_LTCPDOWNLOAD)
    {
        icon = Utilities::getCachedPixmap(QLatin1String(":/down_arrow"));
    }
    else if(transferType & TransferData::TRANSFER_UPLOAD)
    {
        icon = Utilities::getCachedPixmap(QLatin1String(":/up_arrow"));
    }

    mUi->bItemSpeed->setIcon(icon);
}

void TransferManagerDelegateWidget::adjustFileName()
{
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(getData()->mFilename, Qt::ElideMiddle,
                                           getNameAvailableSize(mUi->wTransferName, mUi->lSyncIcon, mUi->nameSpacer)));
    mUi->lTransferName->adjustSize();
    mUi->lTransferName->parentWidget()->layout()->activate();
}

void TransferManagerDelegateWidget::setColumnManager(
    QPointer<TransferWidgetColumnsManager> columnManager)
{
    mColumnManager = columnManager;

    TransferWidgetColumnsManager::ColumnsWidget info;
    info.insert(TransferWidgetColumnsManager::Columns::NAME, mUi->wName);
    info.insert(TransferWidgetColumnsManager::Columns::SIZE, mUi->wSize);
    info.insert(TransferWidgetColumnsManager::Columns::SPEED, mUi->cSpeedItem);
    info.insert(TransferWidgetColumnsManager::Columns::STATUS, mUi->sStatus);
    info.insert(TransferWidgetColumnsManager::Columns::TIME, mUi->lItemTime);
    info.insert(TransferWidgetColumnsManager::Columns::CLEAR_CANCEL, mUi->wClearCancel);
    info.insert(TransferWidgetColumnsManager::Columns::PAUSE_RESUME, mUi->wPauseResume);

    mColumnManager->addColumnsWidget(this, info);
}

TransferBaseDelegateWidget::ActionHoverType TransferManagerDelegateWidget::mouseHoverTransfer(bool isHover, const QPoint &pos)
{
    bool update(false);
    ActionHoverType hoverType(ActionHoverType::NONE);

    if(!getData())
    {
        return hoverType;
    }

    auto isCompleted(getData()->isFinished() && !getData()->isFailed());

    if (isHover)
    {
        bool inCancelClear = isMouseHoverInAction(mUi->tCancelClearTransfer, pos);

        if(isCompleted)
        {
            update = setActionTransferIcon(mUi->tCancelClearTransfer, inCancelClear ? QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_minus_ico_hover.png")
                                                            : QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_minus_ico_default.png"));
        }
        else
        {
            update = setActionTransferIcon(mUi->tCancelClearTransfer, inCancelClear ? QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_cancel_ico_hover.png")
                                                                : QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_cancel_ico_default.png"));
        }

        bool inPauseResume = isMouseHoverInAction(mUi->tPauseResumeTransfer, pos);
        bool inRetry = isMouseHoverInAction(mUi->tItemRetry, pos);

        if(getData())
        {
            auto hoverPauseResume = getData()->getState() == TransferData::TransferState::TRANSFER_PAUSED ? QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_pause_ico_hover_selected.png") :
                                                                                                    QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_pause_ico_hover.png");
            update |= setActionTransferIcon(mUi->tPauseResumeTransfer, inPauseResume ? hoverPauseResume
                                                                : mPauseResumeTransferDefaultIconName);
        }

        if(inCancelClear || inPauseResume || inRetry)
        {
            hoverType = ActionHoverType::HOVER_ENTER;
        }
        else if(update)
        {
            hoverType = ActionHoverType::HOVER_LEAVE;
        }
    }
    else
    {
        if(isCompleted)
        {
            update = setActionTransferIcon(mUi->tCancelClearTransfer, QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_minus_ico_default.png"));
        }
        else
        {
            update = setActionTransferIcon(mUi->tCancelClearTransfer, QString::fromLatin1("://images/transfer_manager/transfers_actions/lists_cancel_ico_default.png"));
        }
        update |= setActionTransferIcon(mUi->tPauseResumeTransfer, mPauseResumeTransferDefaultIconName);

        if(update)
        {
            hoverType = ActionHoverType::HOVER_LEAVE;
        }
    }

    return hoverType;
}

void TransferManagerDelegateWidget::render(const QStyleOptionViewItem& option,
                                           QPainter* painter,
                                           const QRegion& sourceRegion)
{
    bool isDragging(false);
    static QColor hoverColor =
        TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-1"));
    static QColor selectColor =
        TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-2"));
    static auto theme = ThemeManager::instance()->getSelectedTheme();
    if (theme != ThemeManager::instance()->getSelectedTheme())
    {
        theme = ThemeManager::instance()->getSelectedTheme();
        hoverColor = TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-1"));
        selectColor = TokenParserWidgetManager::instance()->getColor(QLatin1String("surface-2"));
    }
    bool showButtons = false;
    auto view = dynamic_cast<MegaTransferView*>(parent());
    if (view)
    {
        isDragging = view->state() == MegaTransferView::DraggingState;
    }

    if (option.state & (QStyle::State_MouseOver | QStyle::State_Selected))
    {
        QPainterPath path;
        painter->setRenderHint(QPainter::Antialiasing, true);
        path.addRoundedRect(
            QRectF(12.0, 4.0, option.rect.width() - 24.0, option.rect.height() - 8.0),
            4,
            4);

        QPen pen;

        if (option.state & QStyle::State_Selected)
        {
            pen.setColor(selectColor);
            painter->fillPath(path, selectColor);
            showButtons = true;
        }
        else
        {
            if (option.state & QStyle::State_MouseOver)
            {
                pen.setColor(hoverColor);
                painter->fillPath(path, hoverColor);
                showButtons = true;
            }
            else if (option.state & QStyle::State_Selected)
            {
                showButtons = true;
                auto painterWidget = dynamic_cast<QWidget*>(painter->device());

                if (isDragging && painterWidget)
                {
                    painter->setOpacity(0.25);
                }
                else
                {
                    pen.setColor(hoverColor);
                    painter->fillPath(path, selectColor);
                }
            }
        }

        if (pen != QPen())
        {
            painter->setPen(pen);
            painter->drawPath(path);
        }
    }
    mUi->tCancelClearTransfer->setVisible(showButtons);
    mUi->tPauseResumeTransfer->setVisible(showButtons);

    TransferBaseDelegateWidget::render(option, painter, sourceRegion);
}

void TransferManagerDelegateWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit openTransfer();

    TransferBaseDelegateWidget::mouseDoubleClickEvent(event);
}

bool TransferManagerDelegateWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        if(watched == mUi->wTransferName)
        {
           adjustFileName();
        }
        else if(watched == mUi->lItemPausedQueued_1)
        {
            mUi->lItemPausedQueued_1->setText(mUi->lItemPausedQueued_1->fontMetrics().elidedText(mUi->lItemPausedQueued_1->text(), Qt::ElideMiddle,mUi->lItemPausedQueued_1->width()
                                          ));

            mUi->lItemPausedQueued_2->setText(mUi->lItemPausedQueued_2->fontMetrics().elidedText(mUi->lItemPausedQueued_2->text(), Qt::ElideMiddle,mUi->lItemPausedQueued_2->width()
                                          ));
            mUi->lItemPausedQueued_1->parentWidget()->adjustSize();
        }
        else if(watched == mUi->lItemFailed)
        {
            mUi->lItemFailed->setText(
                mUi->lItemFailed->fontMetrics().elidedText(mUi->lItemFailed->text(),
                                                           Qt::ElideMiddle,
                                                           mUi->lItemFailed->width()));
            mUi->lItemFailed->parentWidget()->adjustSize();
        }
        // Adapt manually stack page (just failed as, for the moment, is the only one bigger than
        // its original size)
        else if (watched == mUi->sStatus)
        {
            mUi->pFailed->setFixedWidth(mUi->sStatus->width());
        }
        else if(auto label = dynamic_cast<QWidget*>(watched))
        {
            QString text = label->property("text").toString();

            label->setProperty("text", label->fontMetrics()
                               .elidedText(text, Qt::ElideMiddle,
                                          label->contentsRect().width()));
        }
    }

    return TransferBaseDelegateWidget::eventFilter(watched, event);
}

void TransferManagerDelegateWidget::reset()
{
    mPauseResumeTransferDefaultIconName.clear();
    TransferBaseDelegateWidget::reset();
}

void TransferManagerDelegateWidget::on_tPauseResumeTransfer_clicked()
{
    emit pauseResumeTransfer();
}

void TransferManagerDelegateWidget::on_tCancelClearTransfer_clicked()
{
    emit cancelClearTransfer(getData()->isFinished());
}

void TransferManagerDelegateWidget::on_tItemRetry_clicked()
{
    emit retryTransfer();
}
