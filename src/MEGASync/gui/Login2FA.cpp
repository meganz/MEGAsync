#include "Login2FA.h"

#include "Preferences.h"
#include "ServiceUrls.h"
#include "ui_Login2FA.h"
#include "Utilities.h"

#include <QDesktopServices>
#include <QRegExp>
#include <QtConcurrent/QtConcurrent>
#include <QUrl>

Login2FA::Login2FA(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login2FA)
{
    ui->setupUi(this);

    ui->lError->setText(ui->lError->text().toUpper());
    ui->lError->hide();

    connect(ui->bNext, &QPushButton::clicked, this, &Login2FA::onNextClicked);
    connect(ui->bCancel, &QPushButton::clicked, this, &Login2FA::onCancelClicked);
    connect(ui->wHelp, &QPushButton::clicked, this, &Login2FA::onHelpClicked);
    connect(ui->leCode, SIGNAL(textChanged(QString)), this, SLOT(inputCodeChanged()));
    ui->bNext->setDefault(true);
    ui->leCode->setFocus();

    auto lostAuthCodeText = tr("[A]Lost your authenticator device?[/A]");
    const auto recoveryUrl = ServiceUrls::instance()->getRecoveryUrl().toString();
    lostAuthCodeText.replace(
        QLatin1String("[A]"),
        QString::fromUtf8("<a href=\"%1\"><span style='color:#666666; text-decoration:none; "
                          "font-size:11px; font-family: \"Lato\"'>")
            .arg(recoveryUrl));
    lostAuthCodeText.replace(QLatin1String("[/A]"), QLatin1String("</span></a>"));
    ui->lLostAuthCode->setText(lostAuthCodeText);
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

void Login2FA::onNextClicked()
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

void Login2FA::onCancelClicked()
{
    reject();
}

void Login2FA::onHelpClicked()
{
    Utilities::openUrl(ServiceUrls::instance()->getRecoveryUrl());
}

void Login2FA::inputCodeChanged()
{
    ui->lError->hide();
}

bool Login2FA::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    return QDialog::event(event);
}
