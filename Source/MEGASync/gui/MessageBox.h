#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QDialog>

namespace Ui {
class MessageBox;
}

class MessageBox : public QDialog
{
    Q_OBJECT

public:
    explicit MessageBox(QWidget *parent = 0);
    ~MessageBox();
    bool dontAskAgain();

protected:
    void changeEvent(QEvent *);

private:
    Ui::MessageBox *ui;
};

#endif // MESSAGEBOX_H
