#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>

class SwitchButton : public QWidget
{
    Q_OBJECT

public:
    SwitchButton(QWidget* parent);
    void setChecked(bool state);
    bool isChecked();

signals:
    void toggled(bool state);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QWidget* mSwitchButton;

    void onSwitchToggled(bool state);
};

#endif
