#ifndef TRANSFERSSTATEINFOWIDGET_H
#define TRANSFERSSTATEINFOWIDGET_H

#include <QWidget>

namespace Ui {
class TransfersStateInfoWidget;
}

class TransfersStateInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransfersStateInfoWidget(QWidget *parent = 0);
    ~TransfersStateInfoWidget();

    void setText(const QString &text);
    void setIcon(const QIcon & icon);
    void setBackgroundStyle(const QString &text);

private:
    Ui::TransfersStateInfoWidget *ui;
};

#endif // TRANSFERSSTATEINFOWIDGET_H
