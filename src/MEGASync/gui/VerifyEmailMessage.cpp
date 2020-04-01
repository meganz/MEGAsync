#include "VerifyEmailMessage.h"
#ifdef __APPLE__
#include "macx/MacXFunctions.h"
#endif
#include "ui_VerifyEmailMessage.h"

#include <QTimer>
#include <QDebug>
#include <QStyle>
#include "Utilities.h"
#include "MegaApplication.h"

using namespace mega;

VerifyEmailMessage::VerifyEmailMessage(int lockStatus, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::VerifyEmailMessage)
{
    m_ui->setupUi(this);
    m_ui->lEmailSent->setVisible(false);

    m_lockStatus = MegaApi::ACCOUNT_NOT_BLOCKED;
    regenerateUI(lockStatus);

    QStyle *style = QApplication::style();
    QIcon tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, this);
    m_ui->bWarning->setIcon(tmpIcon);

    connect(static_cast<MegaApplication *>(qApp), SIGNAL(unblocked()), this, SLOT(close()));

#ifdef __APPLE__
    QSize size = m_nativeWidget->size();
    m_popover = allocatePopOverWithView(m_nativeWidget->nativeView(), size);
    m_nativeWidget->show();
#endif
}

void VerifyEmailMessage::mousePressEvent(QMouseEvent *event)
{
    if (m_lockStatus == MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL &&
            m_ui->lWhySeenThis->rect().contains(m_ui->lWhySeenThis->mapFrom(this, event->pos())))
    {
#ifdef __APPLE__
        showPopOverRelativeToRect(winId(), m_popover, event->localPos());
#else

        QPoint pos = event->globalPos();

        mLockedPopOver->show();
        mLockedPopOver->ensurePolished();
        mLockedPopOver->move(pos - QPoint(mLockedPopOver->width()/2, mLockedPopOver->height() + 10));
        Utilities::adjustToScreenFunc(pos, mLockedPopOver.get());

        auto initialWidth = mLockedPopOver->width();
        auto initialHeight = mLockedPopOver->height();

        // size might be incorrect the first time it's shown. This works around that and repositions at the expected position afterwards
        QTimer::singleShot(1, this, [this, pos, initialWidth, initialHeight] () {
            mLockedPopOver->update();
            mLockedPopOver->ensurePolished();

            if (initialWidth != mLockedPopOver->width() || initialHeight != mLockedPopOver->height())
            {
                mLockedPopOver->move(pos - QPoint(mLockedPopOver->width()/2, mLockedPopOver->height()));
                Utilities::adjustToScreenFunc(pos, mLockedPopOver.get());
                mLockedPopOver->update();
            }
        });
#endif
    }
}

void VerifyEmailMessage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
        regenerateUI(m_lockStatus, true);
    }

    QDialog::changeEvent(event);
}

void VerifyEmailMessage::regenerateUI(int currentStatus, bool force)
{
    if (!force && m_lockStatus == currentStatus)
    {
        return;
    }

    m_lockStatus = currentStatus;

    switch (m_lockStatus)
    {
        case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL:
        {
            setWindowTitle(tr("Verify your email"));
            m_ui->lVerifyEmailTitle->setText(tr("Verify your email"));
            m_ui->lVerifyEmailDesc->setText(tr("Your account has been temporarily suspended for your safety. Please verify your email and follow its steps to unlock your account."));
            m_ui->lWhySeenThis->setVisible(true);
            m_ui->bResendEmail->setText(tr("Resend email"));

            break;
        }
        case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_SMS:
        {
            setWindowTitle(tr("Verify your account"));
            m_ui->lVerifyEmailTitle->setText(tr("Verify your account"));
            m_ui->lVerifyEmailDesc->setText(tr("Your account has been suspended temporarily due to potential abuse. Please verify your phone number to unlock your account."));
            m_ui->lWhySeenThis->setVisible(false);
            m_ui->lEmailSent->setVisible(false);
            m_ui->bResendEmail->setText(tr("Verify now"));
            break;
        }

    }
}

VerifyEmailMessage::~VerifyEmailMessage()
{
    delete m_ui;
#ifdef __APPLE__
    releaseIdObject(m_popover);
#endif
}

void VerifyEmailMessage::on_bLogout_clicked()
{
    emit logout();
}

void VerifyEmailMessage::on_bResendEmail_clicked()
{
    switch (m_lockStatus)
    {
        case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL:
        {
        //TODO: Finish task of resend email confirmation to fix block situation
            emit resendEmail();
            break;
        }
        case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_SMS:
        {
            static_cast<MegaApplication *>(qApp)->goToMyCloud();
        }
    }
}
