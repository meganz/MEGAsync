#include "LockedPopOver.h"

LockedPopOver::LockedPopOver()
{
    m_ui.setupUi(this);
}

void LockedPopOver::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        m_ui.retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
