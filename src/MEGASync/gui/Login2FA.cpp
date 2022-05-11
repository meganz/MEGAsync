#include "Login2FA.h"
#include "ui_Login2FA.h"
#include "Preferences.h"

#include <QRegExp>
#include <QDesktopServices>
#include <QUrl>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

Login2FA::Login2FA(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login2FA)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lError->setText(ui->lError->text().toUpper());
    ui->lError->hide();

    connect(ui->wHelp, SIGNAL(clicked()), this, SLOT(on_bHelp_clicked()));
    connect(ui->leCode, SIGNAL(textChanged(QString)), this, SLOT(inputCodeChanged()));
    ui->bNext->setDefault(true);
    ui->leCode->setFocus();

    ui->lLostAuthCode->setText(tr("[A]Lost your authenticator device?[/A]")
                               .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"https://mega.nz/recovery\"><span style='color:#666666; text-decoration:none; font-size:11px; font-family: \"Lato\"'>"))
                               .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>")));
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
    QString helpUrl = Preferences::BASE_URL + QString::fromAscii("/recovery");
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
