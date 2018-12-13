#include "Login2FA.h"
#include "ui_Login2FA.h"
#include <QRegExp>
#include <QtConcurrent/QtConcurrent>
#include <QDesktopServices>

Login2FA::Login2FA(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login2FA)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lError->setText(ui->lError->text().toUpper());
    ui->lError->hide();

    connect(ui->wHelp, SIGNAL(clicked()), this, SLOT(on_bHelp_clicked()));
    connect(ui->leCode, SIGNAL(textChanged(QString)), this, SLOT(inputCodeChanged()));
    ui->bNext->setDefault(true);
    ui->leCode->setFocus();

    ui->lLostAuthCode->setText(QString::fromUtf8("<html><head/><body><p>"
                                                 "<a href=\"https://mega.nz/recovery\">"
                                                 "<span style=\"text-decoration: none; "
                                                 "font-size:11px; color:#666666;"
                                                 "font-family: \"SF UI Text\";"
                                                 ">"
                                                 "Lost your authenticator device?</span></a></p></body></html>"));
}

Login2FA::~Login2FA()
{
    delete ui;
}

QString Login2FA::pinCode()
{
    return ui->leCode->text().trimmed();
}

void Login2FA::invalidCode(bool showWarning)
{
    if (showWarning)
    {
        ui->lError->show();
    }
    else
    {
        ui->lError->hide();
    }
}

void Login2FA::on_bNext_clicked()
{
    QRegExp re(QString::fromUtf8("\\d\\d\\d\\d\\d\\d"));
    QString text = pinCode();
    if (text.isEmpty() || !re.exactMatch(text))
    {
        invalidCode(true);
    }
    else
    {
        accept();
    }
}

void Login2FA::on_bCancel_clicked()
{
    reject();
}

void Login2FA::inputCodeChanged()
{
    ui->lError->hide();
}

void Login2FA::on_bHelp_clicked()
{
    QString helpUrl = QString::fromAscii("https://mega.nz/recovery");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(helpUrl));
}

void Login2FA::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
