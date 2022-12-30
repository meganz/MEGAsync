#include "DialogOpener.h"

QList<std::shared_ptr<DialogOpener::DialogInfo>> DialogOpener::mOpenedDialogs = QList<std::shared_ptr<DialogOpener::DialogInfo>>();

std::shared_ptr<DialogOpener::DialogInfo> DialogOpener::findDialogInfo(QDialog *dialog)
{
    foreach(auto dialogInfo, mOpenedDialogs)
    {
        if(dialogInfo->dialog == dialog)
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
        if(dialogInfo->dialog && !dialogInfo->dialogClass.isEmpty())
        {
            if(dialogInfo->dialog != dialog
                    /*&& dialogInfo->dialog->parent() == dialog->parent()*/
                    && dialogInfo->dialogClass  == classType)
            {
                return dialogInfo;
            }
        }
    }

    return nullptr;
}

bool DialogOpener::DialogInfo::operator==(const DialogInfo& info)
{
    return info.dialog == dialog ? true : false;
}
