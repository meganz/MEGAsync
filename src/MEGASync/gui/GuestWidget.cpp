#include "GuestWidget.h"
#include "ui_GuestWidget.h"
#include "megaapi.h"
#include "MegaApplication.h"

using namespace mega;

GuestWidget::GuestWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GuestWidget)
{
    ui->setupUi(this);

    app = (MegaApplication *)qApp;

    ui->lDescLogin->setText(QString::fromUtf8("<p style=\" line-height: 140%;\"><span style=\"font-size:13px;\">")
                               + ui->lDescLogin->text().replace(QString::fromUtf8("[A]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                                                          .replace(QString::fromUtf8("[/A]"), QString::fromUtf8(" </font>"))
                                                                   + QString::fromUtf8("</span></p>"));

}

GuestWidget::~GuestWidget()
{
    delete ui;
}

void GuestWidget::on_bLogin_clicked()
{
    emit actionButtonClicked(LOGIN_CLICKED);
}

void GuestWidget::on_bCreateAccount_clicked()
{
    emit actionButtonClicked(CREATE_ACCOUNT_CLICKED);
}

void GuestWidget::on_bSettings_clicked()
{
    QPoint p = ui->bSettings->mapToGlobal(QPoint(ui->bSettings->width()-6, ui->bSettings->height()));

#ifdef __APPLE__
    QPointer<GuestWidget> iod = this;
#endif

    app->showTrayMenu(&p);

#ifdef __APPLE__
    if (!iod)
    {
        return;
    }

    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }
#endif
}
