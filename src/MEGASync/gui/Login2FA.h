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
    void onNextClicked();
    void onCancelClicked();
    void onHelpClicked();
    void inputCodeChanged();

protected:
    bool event(QEvent* event) override;

private:
    Ui::Login2FA *ui;
};

#endif // LOGIN2FA_H
