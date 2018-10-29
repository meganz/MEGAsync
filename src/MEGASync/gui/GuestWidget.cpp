#include "GuestWidget.h"
#include "ui_GuestWidget.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include <QDesktopServices>
#include <QUrl>
#include "platform/Platform.h"
#include "gui/Login2FA.h"
#include <QtConcurrent/QtConcurrent>

using namespace mega;

GuestWidget::GuestWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GuestWidget)
{
    ui->setupUi(this);

    app = (MegaApplication *)qApp;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    closing = false;
    loggingStarted = false;

    delegateListener = new QTMegaRequestListener(megaApi, this);
    megaApi->addRequestListener(delegateListener);

    ui->sPages->setCurrentWidget(ui->pLogin);

    resetFocus();
}

GuestWidget::~GuestWidget()
{
    delete delegateListener;
    delete ui;
}

void GuestWidget::onRequestStart(MegaApi *api, MegaRequest *request)
{
    if (request->getType() == MegaRequest::TYPE_LOGIN)
    {
        ui->lProgress->setText(tr("Logging in..."));
        page_progress();
    }
    else if (request->getType() == MegaRequest::TYPE_LOGOUT)
    {
        page_logout();
    }
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
            if (error->getErrorCode() == MegaError::API_OK)
            {
                if (loggingStarted)
                {
                    megaApi->fetchNodes();
                    if (!preferences->hasLoggedIn())
                    {
                        preferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                    }
                }

                ui->lProgress->setText(tr("Fetching file list..."));
                page_progress();
                break;
            }

            if (loggingStarted)
            {
                if (error->getErrorCode() == MegaError::API_ENOENT)
                {
                    QMessageBox::warning(NULL, tr("Error"), tr("Incorrect email and/or password.") + QString::fromUtf8(" ") + tr("Have you verified your account?"), QMessageBox::Ok);
                }
                else if (error->getErrorCode() == MegaError::API_EMFAREQUIRED)
                {
                    QPointer<GuestWidget> dialog = this;
                    QPointer<Login2FA> verification = new Login2FA(this);
                    int result = verification->exec();
                    if (!dialog || !verification || result != QDialog::Accepted)
                    {
                        if (dialog)
                        {
                            on_bCancel_clicked();
                            page_login();
                        }
                        delete verification;
                        loggingStarted = false;
                        return;
                    }

                    QString pin = verification->pinCode();
                    delete verification;

                    megaApi->multiFactorAuthLogin(request->getEmail(), request->getPassword(), pin.toUtf8().constData(), delegateListener);
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
                    QPointer<Login2FA> verification = new Login2FA(this);
                    verification->invalidCode(true);
                    int result = verification->exec();
                    if (!dialog || !verification || result != QDialog::Accepted)
                    {
                        if (dialog)
                        {
                            on_bCancel_clicked();
                            page_login();
                        }
                        delete verification;
                        loggingStarted = false;
                        return;
                    }

                    QString pin = verification->pinCode();
                    delete verification;

                    megaApi->multiFactorAuthLogin(request->getEmail(), request->getPassword(), pin.toUtf8().constData(), delegateListener);
                    return;
                }
                else if (error->getErrorCode() != MegaError::API_ESSL)
                {
                    QMessageBox::warning(NULL, tr("Error"), QCoreApplication::translate("MegaError", error->getErrorString()), QMessageBox::Ok);
                }

                loggingStarted = false;
                page_login();
            }
            break;
        }

        case MegaRequest::TYPE_FETCH_NODES:
        {
            if (error->getErrorCode() != MegaError::API_OK)
            {
                page_login();
                loggingStarted = false;
                break;
            }

            if (loggingStarted)
            {
                if (!megaApi->isFilesystemAvailable())
                {
                    page_login();
                    QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                           "Please, try again. If the problem persists "
                                                           "please contact bug@mega.co.nz"), QMessageBox::Ok);
                    app->setupWizardFinished(QDialog::Rejected);
                    preferences->setCrashed(true);
                    app->rebootApplication(false);
                    return;
                }

                char *session = megaApi->dumpSession();
                QString sessionKey = QString::fromUtf8(session);
                delete [] session;

                QString email = ui->lEmail->text().toLower().trimmed();

                if (preferences->hasEmail(email))
                {
                    int proxyType = preferences->proxyType();
                    QString proxyServer = preferences->proxyServer();
                    int proxyPort = preferences->proxyPort();
                    int proxyProtocol = preferences->proxyProtocol();
                    bool proxyAuth = preferences->proxyRequiresAuth();
                    QString proxyUsername = preferences->getProxyUsername();
                    QString proxyPassword = preferences->getProxyPassword();

                    preferences->setEmail(email);
                    preferences->setSession(sessionKey);
                    preferences->setProxyType(proxyType);
                    preferences->setProxyServer(proxyServer);
                    preferences->setProxyPort(proxyPort);
                    preferences->setProxyProtocol(proxyProtocol);
                    preferences->setProxyRequiresAuth(proxyAuth);
                    preferences->setProxyUsername(proxyUsername);
                    preferences->setProxyPassword(proxyPassword);

                    Platform::notifyAllSyncFoldersAdded();

                    app->setupWizardFinished(QDialog::Accepted);
                    break;
                }

                emit forwardAction(CONFIG_MODE);
            }

            page_settingUp();
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

    megaApi->login(email.toUtf8().constData(), password.toUtf8().constData());
    loggingStarted = true;
}

void GuestWidget::on_bCreateAccount_clicked()
{
    emit forwardAction(CREATE_ACCOUNT_CLICKED);
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

    QPointer<QMessageBox> msg = new QMessageBox(this);
    msg->setIcon(QMessageBox::Question);
    //        TO-DO: Uncomment when asset is included to the project
    //        msg->setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-question.png")
    //                                                                    : QString::fromUtf8(":/images/mbox-question@2x.png")));

    msg->setWindowTitle(tr("MEGAsync"));
    msg->setText(tr("Are you sure you want to cancel this wizard and undo all changes?"));
    msg->addButton(QMessageBox::Yes);
    msg->addButton(QMessageBox::No);
    msg->setDefaultButton(QMessageBox::No);
    int button = msg->exec();
    if (msg)
    {
        delete msg;
    }

    if (button == QMessageBox::Yes)
    {
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
}

void GuestWidget::page_login()
{
    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_background.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);

    ui->lEmail->clear();
    ui->lPassword->clear();
    ui->sPages->setCurrentWidget(ui->pLogin);

    resetFocus();
}

void GuestWidget::page_progress()
{
    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_background.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);

    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(-1);
    ui->sPages->setCurrentWidget(ui->pProgress);
}

void GuestWidget::page_settingUp()
{
    ui->sPages->setStyleSheet(QString::fromUtf8("image: url(\"://images/login_intermediate.png\");"));
    ui->sPages->style()->unpolish(ui->sPages);
    ui->sPages->style()->polish(ui->sPages);
    ui->sPages->setCurrentWidget(ui->pSettingUp);
}

void GuestWidget::page_logout()
{
    ui->lProgress->setText(tr("Logging out..."));
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(-1);

    ui->sPages->setCurrentWidget(ui->pProgress);
}
