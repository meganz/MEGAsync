#ifndef TRANSFER_MANAGER_STATUS_HEADER_WIDGET_H
#define TRANSFER_MANAGER_STATUS_HEADER_WIDGET_H

#include <QTimer>
#include <QWidget>

namespace Ui
{
class TransferManagerStatusHeaderWidget;
}

class TransferManagerStatusHeaderWidget: public QWidget
{
    Q_OBJECT

public:
    explicit TransferManagerStatusHeaderWidget(QWidget* parent = nullptr);
    ~TransferManagerStatusHeaderWidget();

    void showAllPausedBanner();
    void hideAllPausedBanner();
    void showTransferBanner(bool show);
    void updateStorageBannerText();
    void setStorageQuotaState(int newStorageQuotaState);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onTransferQuotaExceededUpdate();
    void onUpgradeClicked(const QUrl& link);

private:
    Ui::TransferManagerStatusHeaderWidget* mUi;
    int mStorageQuotaState;
    QTimer mTransferQuotaTimer;

    void showAlmostFullStorageBanner();
    void showFullStorageBanner();
};

#endif // TRANSFER_MANAGER_STATUS_HEADER_WIDGET_H
