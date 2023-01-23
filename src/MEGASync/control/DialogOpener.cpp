#include "DialogOpener.h"

QList<std::shared_ptr<DialogOpener::DialogInfo>> DialogOpener::mOpenedDialogs = QList<std::shared_ptr<DialogOpener::DialogInfo>>();

std::shared_ptr<DialogOpener::DialogInfo> DialogOpener::findDialogInfo(QDialog *dialog)
{
    foreach(auto dialogInfo, mOpenedDialogs)
    {
        if(dialogInfo->getDialog() == dialog)
        {
            return dialogInfo;
        }
    }

    return nullptr;
}

std::shared_ptr<DialogOpener::DialogInfo> DialogOpener::findSiblingDialogInfo(QDialog *dialog, const QString& classType)
{
    foreach(auto dialogInfo, mOpenedDialogs)
    {
        if(dialogInfo->getDialog() && !dialogInfo->getDialogClass().isEmpty())
        {
            if(dialogInfo->getDialog() != dialog
                    /*&& dialogInfo->dialog->parent() == dialog->parent()*/
                    && dialogInfo->getDialogClass()  == classType)
            {
                return dialogInfo;
            }
        }
    }

    return nullptr;
}

bool DialogOpener::DialogInfo::operator==(const DialogInfo& info)
{
    return info.mDialog == mDialog ? true : false;
}
