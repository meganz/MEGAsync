#ifndef CANCELCONFIRMWIDGET_H
#define CANCELCONFIRMWIDGET_H

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

    void show();

signals:
    void proceed();
    void dismiss();

private slots:
    void onDismissClicked();
    void onProceedClicked();

private:
    void enableButtons(bool value);

    Ui::CancelConfirmWidget *ui;
};

#endif // CANCELCONFIRMWIDGET_H
