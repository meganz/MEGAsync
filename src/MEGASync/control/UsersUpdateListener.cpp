#include "UsersUpdateListener.h"

using namespace mega;

void UsersUpdateListener::onUsersUpdate(MegaApi*, MegaUserList* userList)
{
    for (int i = 0; i < userList->size(); i++)
    {
        MegaUser* user = userList->get(i);
        if (user->hasChanged(MegaUser::CHANGE_TYPE_EMAIL))
        {
            const QString newEmail = QString::fromUtf8(user->getEmail());
            emit userEmailUpdated(user->getHandle(), newEmail);
        }
    }
}
