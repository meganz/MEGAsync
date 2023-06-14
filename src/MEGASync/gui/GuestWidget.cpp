#include "GuestWidget.h"
#include "ui_GuestWidget.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"

#include "platform/Platform.h"
#include "gui/Login2FA.h"
#include "gui/GuiUtilities.h"

#include <QDesktopServices>
#include <QUrl>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

GuestWidget::GuestWidget(QWidget *parent) :
    GuestWidget(((MegaApplication *)qApp)->getMegaApi(), parent)
{
}

GuestWidget::GuestWidget(MegaApi *megaApi, QWidget *parent)
    :mSSLSecureConnectionFailed(false),
    QWidget(parent), ui(new Ui::GuestWidget)
{
    ui->setupUi(this);

    connect(static_cast<MegaApplication *>(qApp), SIGNAL(fetchNodesAfterBlock()), this, SLOT(fetchNodesAfterBlockCallbak()));
}

GuestWidget::~GuestWidget()
{
    delete ui;
}

void GuestWidget::onRequestStart(MegaApi*, MegaRequest* request)
{
    if (request->getType() == MegaRequest::TYPE_LOGIN)
    {
        ui->lProgress->setText(tr("Logging in..."));
//        page_progress();
    }
    if (request->getType() == MegaRequest::TYPE_CREATE_ACCOUNT)
    {
        ui->lProgress->setText(tr("Creating account..."));
//        page_progress();
    }
    else if (request->getType() == MegaRequest::TYPE_LOGOUT && request->getFlag())
    {
//        closing = true;
//        page_logout();
    }
}

void GuestWidget::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *error)
{
    if (request->getType() == MegaRequest::TYPE_LOGOUT)
    {
        if(false/*loggingStarted*/)
        {
//            loggingStarted = false;

            //This message is shown every time the user tries to login and the SSL fails
            if(request->getParamType() == MegaError::API_ESSL)
            {
                mSSLSecureConnectionFailed = true;
                showSSLSecureConnectionErrorMessage(request);
//                megaApi->localLogout();
            }
        }
        return;
    }

    switch (request->getType())
    {
        case MegaRequest::TYPE_LOGIN:
        {
            if (error->getErrorCode() == MegaError::API_EMFAREQUIRED
                || error->getErrorCode() == MegaError::API_EFAILED
                || error->getErrorCode() == MegaError::API_EEXPIRED)
            {
                ui->bCancel->setVisible(false);
            }

            if (error->getErrorCode() == MegaError::API_OK)
            {
                if (loggingStarted)
                {
                    preferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
                    auto email = request->getEmail();
                    static_cast<MegaApplication*>(qApp)->fetchNodes(QString::fromUtf8(email ? email : ""));
                    if (!preferences->hasLoggedIn())
                    {
                        preferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                    }
                }

                //Login successful -> unset the SSL fail flag
                mSSLSecureConnectionFailed = false;

//                page_fetchnodes();
                break;
            }

            if (loggingStarted)
            {
                preferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_FAILED);
                if (error->getErrorCode() == MegaError::API_ENOENT)
                {
                    incorrectCredentialsMessageReceived = true;
                }
                else if (error->getErrorCode() == MegaError::API_EMFAREQUIRED)
                {
                    page_login2FA();
                    return;
                }
                else if (error->getErrorCode() == MegaError::API_EINCOMPLETE)
                {
                    QMegaMessageBox::warning(nullptr, tr("Error"), tr("Please check your e-mail and click the link to confirm your account."), QMessageBox::Ok);
                }
                else if (error->getErrorCode() == MegaError::API_ETOOMANY)
                {
                    QMegaMessageBox::warning(nullptr, tr("Error"),
                                             tr("You have attempted to log in too many times.[BR]Please wait until %1 and try again.")
                                             .replace(QString::fromUtf8("[BR]"), QString::fromUtf8("\n"))
                                             .arg(QTime::currentTime().addSecs(3600).toString(QString::fromUtf8("hh:mm")))
                                             , QMessageBox::Ok);
                }
                else if (error->getErrorCode() == MegaError::API_EBLOCKED)
                {
                    QMegaMessageBox::critical(nullptr, tr("Error"), tr("Your account has been blocked. Please contact support@mega.co.nz"));
                }
                else if (error->getErrorCode() == MegaError::API_EFAILED || error->getErrorCode() == MegaError::API_EEXPIRED)
                {
                    showLogin2FaError();
                    page_login2FA();
                    return;
                }
                //Do not show this error if the SSL Secure connection has failed
                else if (error->getErrorCode() != MegaError::API_ESSL && !mSSLSecureConnectionFailed)
                {
                    QMegaMessageBox::warning(nullptr, tr("Error"), QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
                }

                loggingStarted = false;
            }

            page_login();
            break;
        }
        case MegaRequest::TYPE_RESEND_VERIFICATION_EMAIL:
        {
            int e = error->getErrorCode();
            if (e == MegaError::API_OK)
            {
                ui->lEmailSent->setStyleSheet(QString::fromUtf8("#lEmailSent {font-size: 11px; color: #666666;}"));
                ui->lEmailSent->setText(tr("Email sent"));
            }
            else
            {
                ui->lEmailSent->setStyleSheet(QString::fromUtf8("#lEmailSent {font-size: 11px; color: #F0373A;}"));

                if (e == MegaError::API_ETEMPUNAVAIL)
                {
                    ui->lEmailSent->setText(QString::fromUtf8("Email already sent"));
                }
                else
                {
                    ui->lEmailSent->setText(QString::fromUtf8("%1").arg(QCoreApplication::translate("MegaError", error->getErrorString())));
                }
            }

            ui->lEmailSent->setVisible(true);

            Utilities::animateProperty(ui->lEmailSent, 400, "opacity", ui->lEmailSent->property("opacity"), 1.0);

            int animationTime = 500;
            QTimer::singleShot(10000-animationTime, this, [this, animationTime] () {
                Utilities::animateProperty(ui->lEmailSent, animationTime, "opacity", 1.0, 0.5);
                QTimer::singleShot(animationTime, this, [this] () {
                    ui->bVerifyEmail->setEnabled(true);
                });
            });

            break;
        }
    }
}

void GuestWidget::showSSLSecureConnectionErrorMessage(MegaRequest *request) const
{
    QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                          tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                           + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")));
}

void GuestWidget::on_bSettings_clicked()
{
    QPoint p = ui->bSettings->mapToGlobal(QPoint(ui->bSettings->width() - 2, ui->bSettings->height()));

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

