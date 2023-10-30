#include "InfoWizard.h"
#include "ui_InfoWizard.h"
#include "Preferences.h"

InfoWizard::InfoWizard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoWizard)
{
    ui->setupUi(this);
    tweakStrings();
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

void InfoWizard::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        tweakStrings();
    }
    QDialog::changeEvent(event);
}

void InfoWizard::tweakStrings()
{
    ui->lDesckKnowMore->setText(ui->lDesckKnowMore->text()
                                 .replace(QString::fromUtf8("[A]"),
                                          QString::fromUtf8("&nbsp;&nbsp;<a href=\"") + Preferences::BASE_URL + QString::fromUtf8("/sync\"><span style=\"color:#df4843;\">"))
                                 .replace(QString::fromUtf8("[/A]"),
                                          QString::fromUtf8("</span></a>")));
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
    ui->bFirstBullet->setIcon(QIcon(QString::fromLatin1("://images/icon_slice_dot.png")));
    ui->bFirstBullet->setIconSize(QSize(6,6));
    ui->bSecondBullet->setIcon(QIcon(QString::fromLatin1("://images/icon_slice_dot.png")));
    ui->bSecondBullet->setIconSize(QSize(6,6));
    ui->bThirdBullet->setIcon(QIcon(QString::fromLatin1("://images/icon_slice_dot.png")));
    ui->bThirdBullet->setIconSize(QSize(6,6));

    b->setIcon(QIcon(QString::fromLatin1("://images/icon_slice_dot_selected.png")));
    b->setIconSize(QSize(6,6));
}
