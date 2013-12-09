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
	void setProgress(long long completedSize, long long totalSize);
    void setType(int type);
	void hideTransfer();

private:
    Ui::ActiveTransfer *ui;

protected:
    QString fileName;
    int type;
};

#endif // ACTIVETRANSFER_H
