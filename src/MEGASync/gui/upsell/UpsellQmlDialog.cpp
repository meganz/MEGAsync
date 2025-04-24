#include "UpsellQmlDialog.h"

#include "DialogOpener.h"
#include "QmlDialogWrapper.h"
#include "UpsellComponent.h"

bool UpsellQmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close)
    {
        auto dialogInfo(DialogOpener::findDialog<QmlDialogWrapper<UpsellComponent>>());
        if (dialogInfo)
        {
            dialogInfo->getDialog()->wrapper()->sendCloseEvent();
        }
    }

    return QmlDialog::event(event);
}
