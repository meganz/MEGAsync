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
    bool event(QEvent* event) override;

private:
    void startAnimation();

    static QString buildScanDescription(const unsigned int& folderCount, const unsigned int& fileCount);

    static void setRole(QObject* object, const char* name);

    static QString formattedNode(const QString& name);

    QString getScanningFileName() const;

    int mPreviousStage;
    Ui::ScanningWidget *mUi;
    QMovie *mMovie = nullptr;
};

#endif // SCANNINGWIDGET_H
