#ifndef TRANSFERBASEDELEGATEWIDGET
#define TRANSFERBASEDELEGATEWIDGET

#include "TransferRemainingTime.h"
#include "Preferences.h"
#include "TransferItem.h"

#include <QModelIndex>
#include <QWidget>
#include <QToolButton>
#include <QStyleOptionViewItem>
#include <QSpacerItem>

enum class TRANSFER_STATES
{
    STATE_COMPLETING = 0,
    STATE_RETRYING,
    STATE_STARTING,
    STATE_UPLOADING,
    STATE_DOWNLOADING,
    STATE_SYNCING,
    STATE_COMPLETED,
    STATE_PAUSED,
    STATE_FAILED,
    STATE_INQUEUE_PARENTHESIS,
    STATE_INQUEUE,
    STATE_RETRY,
    STATE_OUT_OF_TRANSFER_QUOTA,
    STATE_OUT_OF_STORAGE_SPACE
};

class TransferBaseDelegateWidget : public QWidget
{
    Q_OBJECT

public:
    enum class ActionHoverType
    {
        NONE = 0,
        HOVER_ENTER,
        HOVER_LEAVE
    };

    explicit TransferBaseDelegateWidget(QWidget* parent = nullptr);
    ~TransferBaseDelegateWidget(){}

    void updateUi(const QExplicitlySharedDataPointer<TransferData> transferData, int);
    virtual ActionHoverType mouseHoverTransfer(bool, const QPoint&){return ActionHoverType::NONE;}

    bool stateHasChanged();

    QExplicitlySharedDataPointer<TransferData> getData();

    QModelIndex getCurrentIndex() const;
    void setCurrentIndex(const QModelIndex &currentIndex);

    virtual void render(const QStyleOptionViewItem &, QPainter *painter, const QRegion &sourceRegion);

signals:
    void retryTransfer();

protected:
    bool setActionTransferIcon(QToolButton* button, const QString& iconName);
    bool isMouseHoverInAction(QToolButton* button, const QPoint &mousePos);
    void onRetryTransfer();

    virtual void updateTransferState() = 0;
    virtual void setFileNameAndType() = 0;
    virtual void setType() = 0;

    void changeEvent(QEvent *) override;

    QString getState(TRANSFER_STATES state);

    int getNameAvailableSize(QWidget* nameContainer, QWidget* syncLabel, QSpacerItem* spacer);

    virtual void reset();

private slots:
    void onTransferRemoved(const QModelIndex &parent, int first, int last);

private:
    Preferences* mPreferences;
    QExplicitlySharedDataPointer<TransferData> mData;
    QModelIndex mCurrentIndex;
    QHash<QWidget*, QString> mLastActionTransferIconName;
    TransferData::TransferState mPreviousState;
};

#endif // TRANSFERBASEDELEGATEWIDGET
