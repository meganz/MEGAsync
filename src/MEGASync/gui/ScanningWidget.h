#ifndef SCANNINGWIDGET_H
#define SCANNINGWIDGET_H

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

private slots:
    void onCancelClicked();

private:
    void startAnimation();

    Ui::ScanningWidget *mUi;
    QMovie *mMovie = nullptr;
};

#endif // SCANNINGWIDGET_H
