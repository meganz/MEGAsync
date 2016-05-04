#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QDialog>

namespace Ui {
class TransferManager;
}

class TransferManager : public QDialog
{
    Q_OBJECT

public:
    explicit TransferManager(QWidget *parent = 0);
    ~TransferManager();

private:
    Ui::TransferManager *ui;
};

#endif // TRANSFERMANAGER_H
