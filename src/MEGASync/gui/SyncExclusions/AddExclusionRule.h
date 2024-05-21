#ifndef ADDEXCLUSIONRULE_H
#define ADDEXCLUSIONRULE_H

#include "qml/QmlDialogWrapper.h"

class AddExclusionRule : public QMLComponent
{
    Q_OBJECT
public:
    explicit AddExclusionRule(QObject *parent = nullptr, const QStringList &folders = {});

    QUrl getQmlUrl() override;

    QString contextName() override;

    Q_INVOKABLE void appendRuleToFolders(int targetType, int wildCard, QString ruleVale);
private:
    QStringList mFolders;
};

#endif // ADDEXCLUSIONRULE_H
