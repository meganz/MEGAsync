#include "VerifyEmailMessage.h"
#ifdef __APPLE__
#include "macx/MacXFunctions.h"
#endif
#include "ui_VerifyEmailMessage.h"

#include <QTimer>
#include <QDebug>
#include "Utilities.h"


VerifyEmailMessage::VerifyEmailMessage(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::VerifyEmailMessage)
{
    m_ui->setupUi(this);
    m_ui->lEmailSent->setVisible(false);

    QStyle *style = QApplication::style();
    QIcon tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, this);
    m_ui->bWarning->setIcon(tmpIcon);

#ifdef __APPLE__
    QSize size = m_nativeWidget->size();
    m_popover = allocatePopOverWithView(m_nativeWidget->nativeView(), size);
    m_nativeWidget->show();
#endif
}

void VerifyEmailMessage::mousePressEvent(QMouseEvent *event)
{
    if (m_ui->lWhySeenThis->rect().contains(m_ui->lWhySeenThis->mapFrom(this, event->pos())))
    {
#ifdef __APPLE__
        showPopOverRelativeToRect(winId(), m_popover, event->localPos());
#else

        QPoint pos = event->globalPos();

        mLockedPopOver->show();
        mLockedPopOver->ensurePolished();
        mLockedPopOver->move(pos - QPoint(mLockedPopOver->width()/2, mLockedPopOver->height()));
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
    }
    QWidget::changeEvent(event);
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
    //TODO: Finish task of resend email confirmation to fix block situation
    emit resendEmail();
}
