#ifndef MESSAGE_DIALOG_COMPONENT_H
#define MESSAGE_DIALOG_COMPONENT_H

#include "MessageDialogData.h"
#include "QmlDialogWrapper.h"

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QUrl>

class MessageDialogComponent: public QMLComponent
{
    Q_OBJECT

public:
    explicit MessageDialogComponent(QObject* parent, QPointer<MessageDialogData> data);
    virtual ~MessageDialogComponent() = default;

    QUrl getQmlUrl() override;
    QList<QObject*> getInstancesFromContext() override;

    static void registerQmlModules();

public slots:
    void buttonClicked(int type);
    void setChecked(bool checked);

private:
    QPointer<MessageDialogData> mData;
};

#endif // MESSAGE_DIALOG_COMPONENT_H
