#include "LockedPopOver.h"
#include "ThemeManager.h"

LockedPopOver::LockedPopOver()
{
    m_ui.setupUi(this);
    tweakStrings();
}

bool LockedPopOver::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui.retranslateUi(this);
        tweakStrings();
    }
    return QMacNativeWidget::event(event);
}

void LockedPopOver::tweakStrings()
{
    m_ui.lDescLock->setText(m_ui.lDescLock->text()
                                 .replace(QString::fromUtf8("[A]"),
                                          QString::fromUtf8("<p style=\"line-height: 20px;\">"))
                                 .replace(QString::fromUtf8("[/A]"),
                                          QString::fromUtf8("</p>")));

    m_ui.lPassLeaked->setText(m_ui.lPassLeaked->text()
                                 .replace(QString::fromUtf8("[A]"),
                                          QString::fromUtf8("<p style=\"line-height: 20px;\">"))
                                 .replace(QString::fromUtf8("[/A]"),
                                          QString::fromUtf8("</p>")));
}
