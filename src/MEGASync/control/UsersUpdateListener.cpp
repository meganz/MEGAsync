#include "UsersUpdateListener.h"

using namespace mega;

void UsersUpdateListener::onUsersUpdate(MegaApi*, MegaUserList* userList)
{
    if (userList != nullptr)
    {
        for (int userIndex = 0; userIndex < userList->size(); ++userIndex)
        {
            MegaUser* user = userList->get(userIndex);
            if (user != nullptr && user->hasChanged(MegaUser::CHANGE_TYPE_EMAIL))
            {
                const QString newEmail = QString::fromUtf8(user->getEmail());
                emit userEmailUpdated(user->getHandle(), newEmail);
            }
        }
    }
}
