#ifndef UPDATES_LIST_H
#define UPDATES_LIST_H

#include <QString>

#include <functional>

class UpdatesList
{
public:
    struct UpdateItem
    {
        QString mTitle, mDescription, mImage;
    };

private:
    friend class WhatsNewController;
    std::vector<UpdateItem> mUpdates;
    std::function<void(void)> mAcceptFunction;
    QString mAcceptButtonText;
};

#endif // UPDATES_LIST_H
