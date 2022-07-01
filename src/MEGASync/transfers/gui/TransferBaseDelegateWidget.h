#ifndef TRANSFERBASEDELEGATEWIDGET
#define TRANSFERBASEDELEGATEWIDGET

#include "TransferRemainingTime.h"
#include "Preferences.h"
#include "TransferItem.h"

#include <QModelIndex>
#include <QWidget>
#include <QToolButton>
#include <QStyleOptionViewItem>

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

private:
    Preferences* mPreferences;
    QExplicitlySharedDataPointer<TransferData> mData;
    QModelIndex mCurrentIndex;
    QHash<QWidget*, QString> mLastActionTransferIconName;
    bool mStateHasChanged;
};

#endif // TRANSFERBASEDELEGATEWIDGET
