#include "VerifyEmailMessage.h"
#include "macx/MacXFunctions.h"
#include "ui_VerifyEmailMessage.h"


VerifyEmailMessage::VerifyEmailMessage(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::VerifyEmailMessage)
{
    m_ui->setupUi(this);
    m_ui->lEmailSent->setVisible(false);
    setAttribute(Qt::WA_DeleteOnClose, true);

    QStyle *style = QApplication::style();
    QIcon tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, this);
    m_ui->bWarning->setIcon(tmpIcon);

    QSize size = m_nativeWidget->size();
#ifdef __APPLE__
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
