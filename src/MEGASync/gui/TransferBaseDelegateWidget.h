#ifndef TRANSFERBASEDELEGATEWIDGET
#define TRANSFERBASEDELEGATEWIDGET

#include "TransferRemainingTime.h"
#include "Preferences.h"
#include "TransferItem.h"

#include <QModelIndex>
#include <QWidget>

class TransferBaseDelegateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransferBaseDelegateWidget(QWidget* parent = nullptr);
    ~TransferBaseDelegateWidget(){}

    void updateUi(const QExplicitlySharedDataPointer<TransferData> data, int);
    virtual bool mouseHoverTransfer(bool, const QPoint&){return false;}

    bool stateHasChanged();

    QExplicitlySharedDataPointer<TransferData> getData();

    QModelIndex getCurrentIndex() const;
    void setCurrentIndex(const QModelIndex &currentIndex);

protected:
    virtual void updateTransferState() = 0;
    virtual void setFileNameAndType() = 0;
    virtual void setType() = 0;

private:
    Preferences* mPreferences;
    QExplicitlySharedDataPointer<TransferData> mData;
    TransferData::TransferState mPreviousState;
    QModelIndex mCurrentIndex;
};

#endif // TRANSFERBASEDELEGATEWIDGET
