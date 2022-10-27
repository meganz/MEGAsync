#include "GuestWidget.h"
#include "ui_GuestWidget.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

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

#ifdef _WIN32
    if(getenv("QT_SCREEN_SCALE_FACTORS") || getenv("QT_SCALE_FACTOR"))
    {
        //do not use WA_TranslucentBackground when using custom scale factors in windows
        setStyleSheet(styleSheet().append(QString::fromUtf8("#wGuestWidgetIn{border-radius: 0px;}" ) ));
    }
    else
#endif
    {
        setAttribute(Qt::WA_TranslucentBackground);
    }
    ui->lEmail->setStyleSheet(QString::fromAscii("QLineEdit {color: black;}"));
    ui->lPassword->setStyleSheet(QString::fromAscii("QLineEdit {color: black;}"));
    ui->leCode->setStyleSheet(QString::fromAscii("QLineEdit {color: black;}"));

    reset_UI_props();

    app = (MegaApplication *)qApp;
    this->megaApi = megaApi;
    preferences = Preferences::instance();
    closing = false;
    loggingStarted = false;

    delegateListener = new QTMegaRequestListener(megaApi, this);
    megaApi->addRequestListener(delegateListener);

    ui->sLoginTitle->setCurrentWidget(ui->pLoginTitle);
    ui->sPages->setCurrentWidget(ui->pLogin);
    state = GuestWidgetState::LOGIN;

    ui->lLogin2FAError->hide();

    connect(ui->leCode, &QLineEdit::textChanged, this, &GuestWidget::hide2FaLoginError);
    connect(ui->lEmail, &QLineEdit::textChanged, this, &GuestWidget::hideLoginError);
    connect(ui->lPassword, &QLineEdit::textChanged, this, &GuestWidget::hideLoginError);

    connect(static_cast<MegaApplication *>(qApp), SIGNAL(fetchNodesAfterBlock()), this, SLOT(fetchNodesAfterBlockCallbak()));
    connect(static_cast<MegaApplication *>(qApp), SIGNAL(setupWizardCreated()), this, SLOT(connectToSetupWizard()));

    resetFocus();
}

GuestWidget::~GuestWidget()
{
    delete delegateListener;
    delete ui;
}

void GuestWidget::setTexts(const QString& s1, const QString& s2)
{
    ui->lEmail->setText(s1);
    ui->lPassword->setText(s2);
}

std::pair<QString, QString> GuestWidget::getTexts()
{
    return std::make_pair(ui->lEmail->text(), ui->lPassword->text());
}

void GuestWidget::onRequestStart(MegaApi*, MegaRequest* request)
{
    if (request->getType() == MegaRequest::TYPE_LOGIN)
    {
        ui->lProgress->setText(tr("Logging in..."));
        page_progress();
    }
    if (request->getType() == MegaRequest::TYPE_CREATE_ACCOUNT)
    {
        ui->lProgress->setText(tr("Creating account..."));
        page_progress();
    }
    else if (request->getType() == MegaRequest::TYPE_LOGOUT && request->getFlag())
    {
        closing = true;
        page_logout();
    }
}

void GuestWidget::page_fetchnodes()
{
    ui->lProgress->setText(tr("Fetching file list..."));
    ui->progressBar->setValue(-1);
    page_progress();
}

void GuestWidget::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *error)
{
    if (request->getType() == MegaRequest::TYPE_LOGOUT)
    {
        if(loggingStarted)
        {
            loggingStarted = false;

            //This message is shown every time the user tries to login and the SSL fails
            if(request->getParamType() == MegaError::API_ESSL)
            {
                mSSLSecureConnectionFailed = true;
                showSSLSecureConnectionErrorMessage(request);
                megaApi->localLogout();
            }
        }

        whyAmISeeingThisDialog.reset(nullptr);
        reset_UI_props();
        closing = false;
        page_login();

        return;
    }
    if (closing)
    {
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

                page_fetchnodes();
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
        case MegaRequest::TYPE_FETCH_NODES:
        {
            if (error->getErrorCode() != MegaError::API_OK)
            {
                loggingStarted = false;

                if (error->getErrorCode() != MegaError::API_EBLOCKED)
                {
                    page_login();
                }
                break;
            }

            if (loggingStarted)
            {
                if (!megaApi->isFilesystemAvailable())
                {
                    page_login();
                    return;
                }

            }
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


void GuestWidget::onRequestUpdate(MegaApi*, MegaRequest *request)
{
    GuiUtilities::updateDataRequestProgressBar(ui->progressBar, request);
}

void GuestWidget::resetFocus()
{
    if(state == GuestWidgetState::LOGIN)
    {
        if (ui->lEmail->text().isEmpty())
        {
            ui->lEmail->setFocus();
        }
        ui->bLogin->setDefault(true);
    }

    if(state == GuestWidgetState::LOGIN2FA)
    {
        ui->leCode->setFocus();
        ui->bLogin2FaNext->setDefault(true);
    }
}

void GuestWidget::disableListener()
{
    if (!delegateListener)
    {
        return;
    }

    delete delegateListener;
    delegateListener = NULL;
}

void GuestWidget::enableListener()
{
    if (delegateListener)
    {
        return;
    }

    delegateListener = new QTMegaRequestListener(megaApi, this);
    megaApi->addRequestListener(delegateListener);
}

void GuestWidget::initialize()
{
    whyAmISeeingThisDialog.reset(nullptr);
    reset_UI_props();

    closing = false;
    loggingStarted = false;
    ui->lEmail->clear();
    resetFocus();
    page_login();
}

void GuestWidget::resetPageAfterBlock()
{
    switch(state)
    {
    case GuestWidgetState::LOGIN:
    {
        page_login();
        break;
    }
    case GuestWidgetState::PROGRESS: //Notice: we are not considering fetchnode & loging out as different pages: the progress text would already be set
    {
        page_progress();
        break;
    }
    case GuestWidgetState::SETTINGUP:
    {
        page_settingUp();
        break;
    }
    case GuestWidgetState::LOGIN2FA:
    {
        page_login2FA();
        break;
    }
    default:
        assert(false && "Unexpected state to reset Guest Widget to");
        break;
    }
}

void GuestWidget::showLoginError(const QString &errorMessage) const
{
    constexpr int animationTimeMillis{300};
    ui->lLoginErrors->setText(errorMessage);
    ui->sLoginTitle->setCurrentWidget(ui->pLoginErrors);
    Utilities::animatePartialFadein(ui->lLoginErrors, animationTimeMillis);
}

void GuestWidget::showLogin2FaError() const
{
    constexpr int animationTimeMillis{300};
    Utilities::animatePartialFadein(ui->lLogin2FAError, animationTimeMillis);
    ui->lLogin2FAError->setText(ui->lLogin2FAError->text().toUpper());
    ui->lLogin2FAError->show();
}

void GuestWidget::showSSLSecureConnectionErrorMessage(MegaRequest *request) const
{
    QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                          tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                           + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")));
}

void GuestWidget::hideLoginError()
{
    const bool isLoginErrorBeingShowed{ui->sLoginTitle->currentWidget() == ui->pLoginErrors};
    if(isLoginErrorBeingShowed)
    {
        constexpr int transitionTimeMillis{100};
        Utilities::animatePartialFadeout(ui->lLoginErrors, transitionTimeMillis);
        QTimer::singleShot(transitionTimeMillis, this, [&](){ui->sLoginTitle->setCurrentWidget(ui->pLoginTitle);});
    }
}

void GuestWidget::hide2FaLoginError()
{
    if(ui->lLogin2FAError->isVisible())
    {
        constexpr int animationTimeMillis{100};
        Utilities::animatePartialFadeout(ui->lLogin2FAError, animationTimeMillis);
        QTimer::singleShot(animationTimeMillis, this, [&](){ui->lLogin2FAError->hide();});
    }
}

void GuestWidget::setBlockState(int lockType)
{
    switch(lockType)
    {
        case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL:
        {
            page_lockedEmailAccount();
            break;
        }

        case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_SMS:
        {
            page_lockedSMSAccount();
            break;
        }
        case MegaApi::ACCOUNT_NOT_BLOCKED:
        default:
        {
            resetPageAfterBlock();
            break;
        }
    }
}

void GuestWidget::on_bLogin_clicked()
{
    mEmail = ui->lEmail->text().toLower().trimmed();
    mPassword = ui->lPassword->text();

    if (!mEmail.length())
    {
        showLoginError(tr("Please, enter your e-mail address"));
        return;
    }

    if (!mEmail.contains(QChar::fromAscii('@')) || !mEmail.contains(QChar::fromAscii('.')))
    {
        showLoginError(tr("Please, enter a valid e-mail address"));
        return;
    }

    if (!mPassword.length())
    {
        showLoginError(tr("Please, enter your password"));
        ui->lPassword->setFocus();
        return;
    }

    app->infoWizardDialogFinished(QDialog::Accepted);
    megaApi->login(mEmail.toUtf8().constData(), mPassword.toUtf8().constData());
    loggingStarted = true;
}

void GuestWidget::on_bCreateAccount_clicked()
{
    app->infoWizardDialogFinished(QDialog::Accepted);
    emit forwardAction(SetupWizard::PAGE_NEW_ACCOUNT);
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

void GuestWidget::on_bForgotPassword_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#recovery")));
}

void GuestWidget::on_bCancel_clicked()
{
    if (closing)
    {
        megaApi->localLogout();
        return;
    }

    if (megaApi->isLoggedIn())
    {
        closing = true;
        megaApi->logout(true, nullptr);
        page_logout();
    }
    else
    {
        loggingStarted = false;
        megaApi->localLogout();
        page_login();
    }
}

void GuestWidget::on_bVerifySMSLogout_clicked()
{
    app->unlink();
}

void GuestWidget::on_bVerifyEmailLogout_clicked()
{
    app->unlink();
}

void GuestWidget::on_bVerifyEmail_clicked()
{
    ui->lEmailSent->setProperty("opacity", 0.0);
    ui->bVerifyEmail->setEnabled(false);
    megaApi->resendVerificationEmail(delegateListener);
}

void GuestWidget::on_bVerifySMS_clicked()
{
    static_cast<MegaApplication *>(qApp)->goToMyCloud();
}

void GuestWidget::on_bWhyAmIseen_clicked()
{
    if (!whyAmISeeingThisDialog)
    {
        QString title {QString::fromUtf8(QT_TRANSLATE_NOOP("MegaInfoMessage", "Locked Accounts"))};
        QString firstP {QString::fromUtf8(QT_TRANSLATE_NOOP("MegaInfoMessage","It is possible that you are using the same password for your MEGA account as for other services, and that at least one of these other services has suffered a data breach."))};
        QString secondP {QString::fromUtf8(QT_TRANSLATE_NOOP("MegaInfoMessage","Your password leaked and is now being used by bad actors to log into your accounts, including, but not limited to, your MEGA account."))};

        whyAmISeeingThisDialog.reset(new MegaInfoMessage(QString::fromUtf8(QT_TRANSLATE_NOOP("MegaInfoMessage","Why am I seeing this?")),title, firstP, secondP,
                                              QIcon(QString::fromUtf8(":/images/locked_account_ico.png")).pixmap(70.0, 70.0)));
    }

    whyAmISeeingThisDialog->show();
    whyAmISeeingThisDialog->activateWindow();
    whyAmISeeingThisDialog->raise();
}

void GuestWidget::fetchNodesAfterBlockCallbak()
{
    loggingStarted = true;
    page_fetchnodes();
}

void GuestWidget::connectToSetupWizard()
{
    auto setupWizard = static_cast<MegaApplication *>(qApp)->getSetupWizard();
    if (setupWizard)
    {
        connect(setupWizard, SIGNAL(pageChanged(int)), this, SLOT(onSetupWizardPageChanged(int)));
    }
}

void GuestWidget::onSetupWizardPageChanged(int page)
{
    switch(page)
    {
        case SetupWizard::PAGE_MODE:
        {
            page_settingUp();
            break;
        }
        case SetupWizard::PAGE_PROGRESS:
        {
            page_progress();// this should already be managed by requests callbacks that also set the proper text
                            // but calling it again is idempotent
            break;
        }
        case SetupWizard::PAGE_LOGOUT:
        {
            page_logout();
            break;
        }
        default:
        {
            page_login();
            break;
        }
    }
}

void GuestWidget::page_login()
{
    if (ui->sPages->currentWidget() == ui->pLogin)
    {
        return;
    }

    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_background.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);

    ui->lPassword->clear();
    ui->sPages->setCurrentWidget(ui->pLogin);

    if(incorrectCredentialsMessageReceived)
    {
        incorrectCredentialsMessageReceived = false;
        showLoginError(tr("Incorrect email and/or password."));
    }
    else
    {
       hideLoginError();
    }

    state = GuestWidgetState::LOGIN;
    resetFocus();

    emit onPageLogin();
}

void GuestWidget::page_progress()
{
    if (ui->sPages->currentWidget() == ui->pProgress)
    {
        return;
    }

    ui->bCancel->setVisible(true);
    ui->bCancel->setEnabled(true);

    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_background.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);

    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(-1);

    ui->sPages->setCurrentWidget(ui->pProgress);
    state = GuestWidgetState::PROGRESS;
}

void GuestWidget::page_settingUp()
{
    if (ui->sPages->currentWidget() == ui->pSettingUp)
    {
        return;
    }

    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_intermediate.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);
    ui->sPages->setCurrentWidget(ui->pSettingUp);
    state = GuestWidgetState::SETTINGUP;
}

void GuestWidget::page_logout()
{
    ui->lProgress->setText(tr("Logging out..."));
    ui->progressBar->setValue(-1);
    page_progress();
}

void GuestWidget::page_lockedEmailAccount()
{
    if (ui->sPages->currentWidget() == ui->pVerifyEmailAccount)
    {
        return;
    }

    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_background.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);
    ui->sPages->setCurrentWidget(ui->pVerifyEmailAccount);
}

void GuestWidget::page_lockedSMSAccount()
{
    if (ui->sPages->currentWidget() == ui->pVerifySMSAccount)
    {
        return;
    }

    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_background.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);
    ui->sPages->setCurrentWidget(ui->pVerifySMSAccount);
}

void GuestWidget::page_login2FA()
{
    if (ui->sPages->currentWidget() == ui->pLogin2FA)
    {
        return;
    }

    ui->lLostAuthCode->setText(tr("[A]Lost your authenticator device?[/A]")
                               .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"https://mega.nz/recovery\"><span style='color:#333333; text-decoration:none; font-size:13px; font-family: \"SF UI Text\"'>"))
                               .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>")));

    ui->sPages->setCurrentWidget(ui->pLogin2FA);
    ui->sPages->setStyleSheet(QStringLiteral("image: url(\":/images/login_plain_background.png\");"));
    state = GuestWidgetState::LOGIN2FA;
    resetFocus();
}

void GuestWidget::reset_UI_props()
{
    ui->lEmailSent->setVisible(false);
    ui->bVerifyEmail->setEnabled(true);

    ui->leCode->clear();
    ui->lLogin2FAError->hide();
}

void GuestWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void GuestWidget::on_bLogin2FaNext_clicked()
{
    QRegExp re(QString::fromUtf8("\\d\\d\\d\\d\\d\\d"));
    const QString pin{ui->leCode->text().trimmed()};
    if (pin.isEmpty() || !re.exactMatch(pin))
    {
        showLogin2FaError();
    }
    else
    {
        megaApi->multiFactorAuthLogin(mEmail.toUtf8(), mPassword.toUtf8().constData(), pin.toUtf8().constData());
    }
}

void GuestWidget::on_bLoging2FaCancel_clicked()
{
    megaApi->localLogout();
    page_login();
    loggingStarted = false;
}

void GuestWidget::on_bLogin2FaHelp_clicked()
{
    QString helpUrl = Preferences::BASE_URL + QString::fromAscii("/recovery");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(helpUrl));
}
