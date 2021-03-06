#ifndef INFOWIZARD_H
#define INFOWIZARD_H

#include <QDialog>

namespace Ui {
class InfoWizard;
}

class InfoWizard : public QDialog
{
    Q_OBJECT

public:
    enum {
        FIRST_PAGE = 0,
        SECOND_PAGE = 1,
        THIRD_PAGE = 2
    };

    enum {
        CREATE_ACCOUNT_CLICKED = 1,
        LOGIN_CLICKED
    };

    explicit InfoWizard(QWidget *parent = 0);
    ~InfoWizard();

private slots:
    void on_bLeftArrow_clicked();
    void on_bRightArrow_clicked();

    void on_bFirstBullet_clicked();
    void on_bSecondBullet_clicked();
    void on_bThirdBullet_clicked();

    void on_bLogin_clicked();
    void on_bCreateAccount_clicked();

signals:
    void actionButtonClicked(int button);

protected:
    void changeEvent(QEvent* event);
    void tweakStrings();
    void goToPage(int page);
    void selectedBullet(QPushButton *b);

private:
    Ui::InfoWizard *ui;
};

#endif // INFOWIZARD_H
