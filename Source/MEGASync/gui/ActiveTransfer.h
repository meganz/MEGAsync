#ifndef ACTIVETRANSFER_H
#define ACTIVETRANSFER_H

#include <QWidget>

namespace Ui {
class ActiveTransfer;
}

class ActiveTransfer : public QWidget
{
    Q_OBJECT

public:
    explicit ActiveTransfer(QWidget *parent = 0);
    ~ActiveTransfer();

    void setFileName(QString fileName);
    void setPercentage(int percentage);
    int getPercentage();
    void setType(int type);

private:
    Ui::ActiveTransfer *ui;

protected:
    QString fileName;
    int percentage;
    int type;
};

#endif // ACTIVETRANSFER_H
