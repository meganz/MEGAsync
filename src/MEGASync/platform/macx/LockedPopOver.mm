#include "LockedPopOver.h"
#include "RefreshAppChangeEvent.h"

LockedPopOver::LockedPopOver()
{
    m_ui.setupUi(this);
    tweakStrings();
}

bool LockedPopOver::event(QEvent *event)
{
    if (RefreshAppChangeEvent::isRefreshEvent(event))
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
