#ifndef SCANNINGWIDGET_H
#define SCANNINGWIDGET_H

#include "FolderTransferEvents.h"
#include <QWidget>

namespace Ui {
class ScanningWidget;
}

class ScanningWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScanningWidget(QWidget *parent = nullptr);
    ~ScanningWidget();

    void show();
    void hide();
    void disableCancelButton();
    void updateAnimation();

signals:
    void cancel();

public slots:
    void onReceiveStatusUpdate(const FolderTransferUpdateEvent& event);

private slots:
    void onCancelClicked();

protected:
    void changeEvent(QEvent* event) override;

private:
    void startAnimation();

    static QString buildScanDescription(const uint32_t folderCount, const uint32_t fileCount);

    static void setRole(QObject* object, const char* name);

    static QString formattedNode(const QString& name);

    int mPreviousStage;
    Ui::ScanningWidget *mUi;
    QMovie *mMovie = nullptr;
};

#endif // SCANNINGWIDGET_H
