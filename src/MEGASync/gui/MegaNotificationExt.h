#ifndef MEGANOTIFICATIONEXT_H
#define MEGANOTIFICATIONEXT_H

#include "megaapi.h"

#include <QObject>

#include <memory>

class MegaNotificationExt : public QObject
{
    Q_OBJECT

public:
    MegaNotificationExt() = delete;
    MegaNotificationExt(const mega::MegaNotification* notification, QObject* parent = nullptr);
    ~MegaNotificationExt() = default;

    int64_t getID() const;
    QString getTitle() const;
    QString getDescription() const;

    bool showImage() const;
    QString getImageNamePath() const;

    bool showIcon() const;
    QString getIconNamePath() const;

    int64_t getStart() const;
    int64_t getEnd() const;
    const char* getActionText() const;
    const char* getActionLink() const;

private:
    std::unique_ptr<const mega::MegaNotification> mNotification;

};

#endif // MEGANOTIFICATIONEXT_H
