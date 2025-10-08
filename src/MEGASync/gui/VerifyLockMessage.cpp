#include "VerifyLockMessage.h"

#include "MegaApplication.h"
#include "TextDecorator.h"
#include "ServiceUrls.h"
#include "ui_VerifyLockMessage.h"

#include <QStyle>
#include <QTimer>

using namespace mega;

VerifyLockMessage::VerifyLockMessage(QWidget *parent) :
    QDialog(parent), m_ui(new Ui::VerifyLockMessage)
{
    m_ui->setupUi(this);

    regenerateUI();
    connect(static_cast<MegaApplication *>(qApp), SIGNAL(unblocked()), this, SLOT(close()));
    connect(m_ui->bWhySeenThis,
            &TokenizableToolButton::clicked,
            this,
            &VerifyLockMessage::onHelpButtonClicked);
}

bool VerifyLockMessage::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
        regenerateUI();
    }

    return QDialog::event(event);
}

void VerifyLockMessage::regenerateUI()
{
    QString title = tr("Locked account");
    m_ui->lVerifyEmailTitle->setText(title);
    QString msg = QCoreApplication::translate(
        "GuestStrings",
        "Your account has been locked for your protection after detecting a malicious "
        "login, so we require you to reset your password.[BR]\nCheck your email inbox for "
        "instructions on unlocking your account and tips on how to prevent this from "
        "happening again.");

    Text::NewLine decorator;
    decorator.process(msg, 2);
    m_ui->lVerifyEmailDesc->setText(msg);
}

VerifyLockMessage::~VerifyLockMessage()
{
    delete m_ui;
}

void VerifyLockMessage::on_bLogout_clicked()
{
    close();
    emit logout();
}

void VerifyLockMessage::onHelpButtonClicked()
{
    Utilities::openUrl(ServiceUrls::getCredentialStuffingHelpUrl());
}

