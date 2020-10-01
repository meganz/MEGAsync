#include "DynamicTransferQuotaPopOver.h"

DynamicTransferQuotaPopOver::DynamicTransferQuotaPopOver()
{
    m_ui.setupUi(this);
    tweakStrings();
}

void DynamicTransferQuotaPopOver::updateMessage(const QString &message)
{
    m_ui.labelMessage->setText(message);
}


void DynamicTransferQuotaPopOver::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui.retranslateUi(this);
        tweakStrings();
    }
    QWidget::changeEvent(event);
}

void DynamicTransferQuotaPopOver::tweakStrings()
{
    m_ui.labelMessage->setText(m_ui.labelMessage->text()
                                 .replace(QString::fromUtf8("[A]"),
                                          QString::fromUtf8("<p style=\"line-height: 20px;\">"))
                                 .replace(QString::fromUtf8("[/A]"),
                                          QString::fromUtf8("</p>")));
}
