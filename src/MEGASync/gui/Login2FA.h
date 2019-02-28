#ifndef LOGIN2FA_H
#define LOGIN2FA_H

#include <QDialog>

namespace Ui {
class Login2FA;
}

class Login2FA : public QDialog
{
    Q_OBJECT

public:
    explicit Login2FA(QWidget *parent = 0);
    ~Login2FA();
    QString pinCode();
    void invalidCode(bool showWarning);

private slots:
    void on_bNext_clicked();
    void on_bCancel_clicked();
    void inputCodeChanged();
    void on_bHelp_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::Login2FA *ui;
};

#endif // LOGIN2FA_H
