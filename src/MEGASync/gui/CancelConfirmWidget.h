#ifndef CANCELCONFIRMWIDGET_H
#define CANCELCONFIRMWIDGET_H

#include <QMovie>
#include <QWidget>

namespace Ui {
class CancelConfirmWidget;
}

class CancelConfirmWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CancelConfirmWidget(QWidget *parent = nullptr);
    ~CancelConfirmWidget();

    void setStyleSheet(const QString& stylesheet);

    void show();
    void setInCancellingStage();

protected:
    void changeEvent(QEvent* event) override;

signals:
    void proceed();
    void dismiss();

private slots:
    void onDismissClicked();
    void onProceedClicked();

private:
    void setupAnimation();
    void enableButtons(bool value);

    Ui::CancelConfirmWidget *ui;
    QMovie* mAnimation = nullptr;
    unsigned int mDisplayDelayInMs = 800;
};

#endif // CANCELCONFIRMWIDGET_H
