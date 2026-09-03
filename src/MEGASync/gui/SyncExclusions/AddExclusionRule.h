#ifndef ADDEXCLUSIONRULE_H
#define ADDEXCLUSIONRULE_H

#include "QmlDialogWrapper.h"

class AddExclusionRule : public QMLComponent
{
    Q_OBJECT

public:
    explicit AddExclusionRule(QObject *parent = nullptr, const QStringList &folders = {});

    QUrl getQmlUrl() override;

    Q_INVOKABLE void appendRuleToFolders(int targetType, int wildCard, QString ruleValue);
    static void copyCurrentRulesTo(const QString& sourceFolder, const QString& targetFolder);

signals:
    void exclusionRuleAdded(QString folder);

private:
    QStringList mFolders;

    QString getRelative(const QString& path, const QString& fullPath);

};

#endif // ADDEXCLUSIONRULE_H
