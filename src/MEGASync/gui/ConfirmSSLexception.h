#ifndef CONFIRMSSLEXCEPTION_H
#define CONFIRMSSLEXCEPTION_H

#include <QDialog>

namespace Ui {
class ConfirmSSLexception;
}

class ConfirmSSLexception : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmSSLexception(QWidget *parent = 0);
    ~ConfirmSSLexception();

    bool dontAskAgain();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::ConfirmSSLexception *ui;
};

#endif // CONFIRMSSLEXCEPTION_H
