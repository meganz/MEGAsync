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

signals:
    void proceed();
    void dismiss();

private slots:
    void on_pDismiss_clicked();
    void on_pProceed_clicked();

private:
    Ui::CancelConfirmWidget *ui;
};

#endif // CANCELCONFIRMWIDGET_H
