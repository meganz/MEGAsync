#include "GuestWidget.h"
#include "ui_GuestWidget.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include <QDesktopServices>
#include <QUrl>
#include "platform/Platform.h"
#include "gui/Login2FA.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

GuestWidget::GuestWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GuestWidget)
{
    ui->setupUi(this);

#ifdef _WIN32
    if(getenv("QT_SCREEN_SCALE_FACTORS"))
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

    app = (MegaApplication *)qApp;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    closing = false;
    loggingStarted = false;

    delegateListener = new QTMegaRequestListener(megaApi, this);
    megaApi->addRequestListener(delegateListener);

    ui->sPages->setCurrentWidget(ui->pLogin);
    state = GuestWidgetState::LOGIN;

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

void GuestWidget::onRequestStart(MegaApi *api, MegaRequest *request)
{
    if (request->getType() == MegaRequest::TYPE_LOGIN)
    {
        ui->lProgress->setText(tr("Logging in..."));
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
    if (closing)
    {
        if (request->getType() == MegaRequest::TYPE_LOGOUT)
        {
            closing = false;
            loggingStarted = false;
            page_login();
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
                    static_cast<MegaApplication*>(qApp)->fetchNodes();
                    if (!preferences->hasLoggedIn())
                    {
                        preferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                    }
                }

                page_fetchnodes();
                break;
            }

            if (loggingStarted)
            {
                preferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_FAILED);
                if (error->getErrorCode() == MegaError::API_ENOENT)
                {
                    QMessageBox::warning(this, tr("Error"), tr("Incorrect email and/or password."), QMessageBox::Ok);
                }
                else if (error->getErrorCode() == MegaError::API_EMFAREQUIRED)
                {
                    QPointer<GuestWidget> dialog = this;
                    QPointer<Login2FA> verification = new Login2FA();
                    int result = verification->exec();
                    if (!dialog || !verification || result != QDialog::Accepted)
                    {
                        if (dialog)
                        {
                            megaApi->localLogout();
                            page_login();
                            loggingStarted = false;
                        }
                        delete verification;
                        return;
                    }

                    QString pin = verification->pinCode();
                    delete verification;

                    megaApi->multiFactorAuthLogin(request->getEmail(), request->getPassword(), pin.toUtf8().constData());
                    return;
                }
                else if (error->getErrorCode() == MegaError::API_EINCOMPLETE)
                {
                    QMessageBox::warning(NULL, tr("Error"), tr("Please check your e-mail and click the link to confirm your account."), QMessageBox::Ok);
                }
                else if (error->getErrorCode() == MegaError::API_ETOOMANY)
                {
                    QMessageBox::warning(NULL, tr("Error"),
                                             tr("You have attempted to log in too many times.[BR]Please wait until %1 and try again.")
                                             .replace(QString::fromUtf8("[BR]"), QString::fromUtf8("\n"))
                                             .arg(QTime::currentTime().addSecs(3600).toString(QString::fromUtf8("hh:mm")))
                                             , QMessageBox::Ok);
                }
                else if (error->getErrorCode() == MegaError::API_EBLOCKED)
                {
                    QMessageBox::critical(NULL, tr("Error"), tr("Your account has been blocked. Please contact support@mega.co.nz"));
                }
                else if (error->getErrorCode() == MegaError::API_EFAILED || error->getErrorCode() == MegaError::API_EEXPIRED)
                {
                    QPointer<GuestWidget> dialog = this;
                    QPointer<Login2FA> verification = new Login2FA();
                    verification->invalidCode(true);
                    int result = verification->exec();
                    if (!dialog || !verification || result != QDialog::Accepted)
                    {
                        if (dialog)
                        {
                            megaApi->localLogout();
                            page_login();
                            loggingStarted = false;
                        }
                        delete verification;
                        return;
                    }

                    QString pin = verification->pinCode();
                    delete verification;

                    megaApi->multiFactorAuthLogin(request->getEmail(), request->getPassword(), pin.toUtf8().constData());
                    return;
                }
                else if (error->getErrorCode() != MegaError::API_ESSL)
                {
                    QMessageBox::warning(NULL, tr("Error"), QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
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
        case MegaRequest::TYPE_LOGOUT:
        {
            loggingStarted = false;
            page_login();
            break;
        }
    }
}

void GuestWidget::onRequestUpdate(MegaApi *api, MegaRequest *request)
{
    if (request->getType() == MegaRequest::TYPE_FETCH_NODES)
    {
        if (request->getTotalBytes() > 0)
        {
            ui->progressBar->setMaximum(request->getTotalBytes());
            ui->progressBar->setValue(request->getTransferredBytes());
        }
    }
}

void GuestWidget::resetFocus()
{
    if (!ui->lEmail->text().size())
    {
        ui->lEmail->setFocus();
    }

    ui->bLogin->setDefault(true);
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
    closing = false;
    loggingStarted = false;
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
    default:
        assert(false && "Unexpected state to reset Guest Widget to");
        break;
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
    QString email = ui->lEmail->text().toLower().trimmed();
    QString password = ui->lPassword->text();

    if (!email.length())
    {
        QMessageBox::warning(NULL, tr("Error"), tr("Please, enter your e-mail address"), QMessageBox::Ok);
        return;
    }

    if (!email.contains(QChar::fromAscii('@')) || !email.contains(QChar::fromAscii('.')))
    {
        QMessageBox::warning(NULL, tr("Error"), tr("Please, enter a valid e-mail address"), QMessageBox::Ok);
        return;
    }

    if (!password.length())
    {
        QMessageBox::warning(NULL, tr("Error"), tr("Please, enter your password"), QMessageBox::Ok);
        return;
    }

    app->infoWizardDialogFinished(QDialog::Accepted);
    megaApi->login(email.toUtf8().constData(), password.toUtf8().constData());
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
        megaApi->logout();
        page_logout();
    }
    else
    {
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
    app->showVerifyAccountInfo();
}

void GuestWidget::on_bVerifySMS_clicked()
{
    app->showVerifyAccountInfo();
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

    ui->lEmail->clear();
    ui->lPassword->clear();
    ui->sPages->setCurrentWidget(ui->pLogin);

    resetFocus();

    state = GuestWidgetState::LOGIN;

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

void GuestWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
