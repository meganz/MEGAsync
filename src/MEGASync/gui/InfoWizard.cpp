#include "InfoWizard.h"
#include "ui_InfoWizard.h"

InfoWizard::InfoWizard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoWizard)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::WindowModal);

    QPalette pal;
    pal.setBrush(this->backgroundRole(), QBrush(QImage( QString::fromUtf8("://images/bg_info_wizard.png" ))));
    this->setPalette(pal);


    ui->lFlexibleDesc->setText(ui->lFlexibleDesc->text().replace(QString::fromUtf8("[S]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                               .replace(QString::fromUtf8("[/S]"), QString::fromUtf8(" </font>"))
                               + QString::fromUtf8("</span></p>"));


    ui->lAdvantagesDesc->setText(ui->lAdvantagesDesc->text().replace(QString::fromUtf8("[A]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                                 .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</font>"))
                                 + QString::fromUtf8("</span></p>"));
    goToPage(FIRST_PAGE);
}

InfoWizard::~InfoWizard()
{
    delete ui;
}

void InfoWizard::on_bLeftArrow_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if (w == ui->page_2)
    {
        goToPage(FIRST_PAGE);
    }
    else if (w == ui->page_3)
    {
        goToPage(SECOND_PAGE);
    }
}

void InfoWizard::on_bRightArrow_clicked()
{
    QWidget *w = ui->sPages->currentWidget();
    if (w == ui->page_1)
    {
        goToPage(SECOND_PAGE);
    }
    else if (w == ui->page_2)
    {
        goToPage(THIRD_PAGE);
    }
}

void InfoWizard::on_bFirstBullet_clicked()
{
    goToPage(FIRST_PAGE);
}

void InfoWizard::on_bSecondBullet_clicked()
{
    goToPage(SECOND_PAGE);
}

void InfoWizard::on_bThirdBullet_clicked()
{
    goToPage(THIRD_PAGE);
}

void InfoWizard::on_bLogin_clicked()
{
    emit actionButtonClicked(LOGIN_CLICKED);
    accept();
}

void InfoWizard::on_bCreateAccount_clicked()
{
    emit actionButtonClicked(CREATE_ACCOUNT_CLICKED);
    accept();
}

void InfoWizard::goToPage(int page)
{
    switch (page)
    {
        case FIRST_PAGE:
            ui->bLeftArrow->setVisible(false);
            ui->bRightArrow->setVisible(true);
            selectedBullet(ui->bFirstBullet);
            ui->sPages->setCurrentWidget(ui->page_1);
            break;

        case SECOND_PAGE:
            ui->bLeftArrow->setVisible(true);
            ui->bRightArrow->setVisible(true);
            selectedBullet(ui->bSecondBullet);
            ui->sPages->setCurrentWidget(ui->page_2);
            break;

        case THIRD_PAGE:
            ui->bLeftArrow->setVisible(true);
            ui->bRightArrow->setVisible(false);
            selectedBullet(ui->bThirdBullet);
            ui->sPages->setCurrentWidget(ui->page_3);
            break;

        default:
            return;
    }

}

void InfoWizard::selectedBullet(QPushButton *b)
{
    ui->bFirstBullet->setIcon(QIcon(QString::fromAscii("://images/empty_dot.png")));
    ui->bFirstBullet->setIconSize(QSize(16,16));
    ui->bSecondBullet->setIcon(QIcon(QString::fromAscii("://images/empty_dot.png")));
    ui->bSecondBullet->setIconSize(QSize(16,16));
    ui->bThirdBullet->setIcon(QIcon(QString::fromAscii("://images/empty_dot.png")));
    ui->bThirdBullet->setIconSize(QSize(16,16));

    b->setIcon(QIcon(QString::fromAscii("://images/filled_dot.png")));
    b->setIconSize(QSize(16,16));

}
