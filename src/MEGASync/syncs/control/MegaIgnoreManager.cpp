#include "MegaIgnoreManager.h"

#include <Utilities.h>
#include "Preferences.h"
#include "MegaApplication.h"

#include <QDir>
#include <QChar>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QTemporaryFile>
#include <QFileInfo>

MegaIgnoreManager::MegaIgnoreManager(const QString& syncLocalFolder, bool createDefaultIfNotExist)
{
    const auto ignorePath(syncLocalFolder + QDir::separator() + QString::fromUtf8(".megaignore"));
    mOutputMegaIgnoreFile = ignorePath;
    mMegaIgnoreFile = ignorePath;
    if (createDefaultIfNotExist && !QFile::exists(ignorePath))
    {
        mMegaIgnoreFile = Preferences::instance()->getDataPath() + QDir::separator() + QString::fromUtf8(".megaignore.default");
    }
    parseIgnoresFile();
}

bool MegaIgnoreManager::isValid(const QString& syncLocalFolder)
{
    const auto ignorePath(syncLocalFolder + QDir::separator() + QString::fromUtf8(".megaignore"));
    QFileInfo ignoreInfo(ignorePath);
    if (ignoreInfo.exists())
    {
        QFile ignoreFile(ignorePath);
        auto canBeReadWrite(ignoreFile.open(QIODevice::ReadWrite));
        if (canBeReadWrite)
        {
            ignoreFile.close();
        }
        return canBeReadWrite;
    }
    else
    {
        QDir megaIgnorePath(ignorePath);
        QTemporaryFile test (megaIgnorePath.absolutePath() + QDir::separator());
        return test.open();
    }

    return true;
}

void MegaIgnoreManager::parseIgnoresFile()
{
    mRules.clear();
    mLowLimitRule.reset();
    mHighLimitRule.reset();
    mIgnoreSymLinkRule.reset();
    mExtensionRules.clear();

    QFile ignore(mMegaIgnoreFile);
    if (ignore.exists())
    {
        if (ignore.open(QIODevice::ReadOnly))
        {
            QTextStream in(&ignore);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                if (line.trimmed().isEmpty())
                {
                    continue;
                }
                const bool isCommented(line.startsWith(QLatin1String("#")));
                const auto ruleType = getRuleType(line);
                switch (ruleType)
                {
                    case MegaIgnoreRule::RuleType::SIZERULE:
                    {
                        const static QRegularExpression largeSizeRegEx{ QLatin1String(MegaIgnoreRule::LARGE_SIZE_LEFT_SIDE_REG_EX) };
                        const static QRegularExpression smallSizeRegEx{ QLatin1String(MegaIgnoreRule::SMALL_SIZE_LEFT_SIDE_REG_EX) };
                        if (largeSizeRegEx.match(line).hasMatch())
                        {
                            auto highLimitRule = std::make_shared<MegaIgnoreSizeRule>(line, isCommented);
                            if (!mHighLimitRule || !highLimitRule->isCommented())
                            {
                                mHighLimitRule = highLimitRule;
                            }
                            addRule(highLimitRule);
                        }
                        else if (smallSizeRegEx.match(line).hasMatch())
                        {
                            auto lowLimitRule = std::make_shared<MegaIgnoreSizeRule>(line, isCommented);
                            if (!mLowLimitRule || !lowLimitRule->isCommented())
                            {
                                mLowLimitRule = lowLimitRule;
                            }
                            addRule(lowLimitRule);
                        }
                        break;
                    }
                    case MegaIgnoreRule::RuleType::EXTENSIONRULE:
                    {
                        auto extensionRule(std::make_shared<MegaIgnoreExtensionRule>(line, isCommented));
                        addRule(extensionRule);
                        mExtensionRules.insert(extensionRule->extension(), extensionRule);
                        break;
                    }
                    case MegaIgnoreRule::RuleType::NAMERULE:
                    {
                        std::shared_ptr<MegaIgnoreNameRule> rule = std::make_shared<MegaIgnoreNameRule>(line, isCommented);

                        if (line.compare(QLatin1String(MegaIgnoreRule::IGNORE_SYM_LINK)) == 0)
                        {
                            mIgnoreSymLinkRule = rule;
                        }

                        addRule(rule);

                        break;
                    }
                    default:
                        addRule(std::make_shared<MegaIgnoreInvalidRule>(line, isCommented));
                        break;
                }
            }
            ignore.close();
        }
        std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRC(mMegaIgnoreFile.toStdString().c_str()));
        mIgnoreCRC = QString::fromUtf8(crc.get());
    }

    if (!mLowLimitRule)
    {
        mLowLimitRule = std::make_shared<MegaIgnoreSizeRule>(MegaIgnoreSizeRule::Threshold::LOW);
        addRule(mLowLimitRule);
    }
    if (!mHighLimitRule)
    {
        mHighLimitRule = std::make_shared<MegaIgnoreSizeRule>(MegaIgnoreSizeRule::Threshold::HIGH);
        addRule(mHighLimitRule);
    }
}

std::shared_ptr<MegaIgnoreRule> MegaIgnoreManager::getRuleByOriginalRule(const QString& originalRule)
{
    auto finder = [&originalRule](const std::shared_ptr<MegaIgnoreRule>& rule)
    {
        return rule->originalRule() == originalRule;
    };
    auto it = std::find_if(mRules.begin(), mRules.end(), finder);
    return (it != mRules.end()) ? *it : nullptr;
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getLowLimitRule() const
{
    return mLowLimitRule;
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getHighLimitRule() const
{
    return mHighLimitRule;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::getIgnoreSymLink() const
{
    return mIgnoreSymLinkRule;
}

QList<std::shared_ptr<MegaIgnoreRule>> MegaIgnoreManager::getAllRules() const
{
    return mRules;
}

std::shared_ptr<MegaIgnoreRule> MegaIgnoreManager::findRule(const QString& ruleToCompare)
{
    auto finder = [&ruleToCompare](const std::shared_ptr<MegaIgnoreRule>& rule)
    {
        return rule->isEqual(ruleToCompare);
    };
    auto it = std::find_if(mRules.begin(), mRules.end(), finder);
    return (it != mRules.end()) ? *it : nullptr;
}

MegaIgnoreRule::RuleType MegaIgnoreManager::getRuleType(const QString& line)
{
    const auto lineSplitted(line.split(QLatin1String(":")));
    if (lineSplitted.size() < 2)
    {
        return MegaIgnoreRule::RuleType::INVALIDRULE;
    }
    const QString leftSide = lineSplitted[0].trimmed();
    const QString rightSide = lineSplitted.mid(1).join(QLatin1String(":")).trimmed();

    // Check size rule 
    static const QRegularExpression sizeRuleLeftSide(QLatin1String(MegaIgnoreRule::SIZE_RULE_LEFT_SIDE_REG_EX), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression sizeRuleRightSide(QLatin1String(MegaIgnoreRule::SIZE_RULE_RIGHT_SIDE_REG_EX), QRegularExpression::CaseInsensitiveOption);
    if (sizeRuleLeftSide.match(leftSide).hasMatch() && sizeRuleRightSide.match(rightSide).hasMatch())
    {
        return MegaIgnoreRule::RuleType::SIZERULE;
    }
    // Check if valid left side 
    const static  QRegularExpression nonSizeRuleRegularExpression{ QLatin1String(MegaIgnoreRule::NON_SIZE_LEFT_SIDE_REG_EX) };
    if (!nonSizeRuleRegularExpression.match(leftSide).hasMatch())
    {
        return MegaIgnoreRule::RuleType::INVALIDRULE;
    }

    // Check if type rule
    const static QRegularExpression typeRuleRegEx{ QLatin1String(MegaIgnoreRule::RIGHT_SIDE_TYPE_RULE_REG_EX) };
    if (typeRuleRegEx.match(rightSide).hasMatch())
    {
        return MegaIgnoreRule::RuleType::EXTENSIONRULE;
    }
    return MegaIgnoreRule::NAMERULE;
}

QList<std::shared_ptr<MegaIgnoreNameRule> > MegaIgnoreManager::getNameRules() const
{
    QList<std::shared_ptr<MegaIgnoreNameRule>> rules;
    foreach(const auto & rule, mRules)
    {
        if (rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::NAMERULE)
        {
            auto nameRule = convert<MegaIgnoreNameRule>(rule);
            rules.append(nameRule);
        }
    }

    return rules;
}

QStringList MegaIgnoreManager::getExcludedExtensions() const
{
    QStringList extensions;
    foreach(const auto & rule, mRules)
    {
        if (rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::EXTENSIONRULE)
        {
            auto extensionRule = convert<MegaIgnoreExtensionRule>(rule);
            if (extensionRule)
            {
                extensions.append(extensionRule->extension());
            }
        }
    }
    return extensions;
}

void MegaIgnoreManager::enableExtensions(bool state)
{
    foreach(auto & rule, mRules)
    {
        if (rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::EXTENSIONRULE)
        {
            rule->setCommented(!state);
        }
    }
}

MegaIgnoreManager::ApplyChangesError MegaIgnoreManager::applyChanges(bool updateExtensionRules, const QStringList& updatedExtensions)
{
    ApplyChangesError result(ApplyChangesError::NoUpdateNeeded);
    QStringList rules;
    foreach(auto & rule, mRules)
    {
        if (!rule->isDeleted())
        {
            if (!rule->isValid())
            {
                rule->setCommented(true);
            }
            if (rule->ruleType() == MegaIgnoreRule::RuleType::EXTENSIONRULE && updateExtensionRules)
            {
                continue;
            }
            auto ruleAsText(rule->getModifiedRule());
            if (!ruleAsText.isEmpty())
            {
                rules.append(ruleAsText);
            }
        }

        if (rule->isDirty() || rule->isDeleted())
        {
            result = ApplyChangesError::Ok;
        }
    }
    if (updateExtensionRules)
    {
        result = ApplyChangesError::Ok;

        for (const auto extension : updatedExtensions)
        {
            auto trimmedExtension = extension.trimmed();

            while(trimmedExtension.startsWith(QLatin1String(".")))
            {
                trimmedExtension.remove(0,1);
            }

            if (mExtensionRules.contains(trimmedExtension))
            {
                rules.append(mExtensionRules.value(trimmedExtension)->getModifiedRule());
            }
            else if(!trimmedExtension.isEmpty())
            {
                const MegaIgnoreExtensionRule extensionRule(MegaIgnoreNameRule::Class::EXCLUDE, trimmedExtension);
                rules.append(extensionRule.getModifiedRule());
            }
        }
    }
    if (result == ApplyChangesError::Ok)
    {
        QFile ignore(mOutputMegaIgnoreFile);

        if (ignore.open(QIODevice::WriteOnly))
        {
            QTextStream out(&ignore);
            out << rules.join(QLatin1String("\n"));

            ignore.close();
        }
        else
        {
            result = ApplyChangesError::NoWritePermission;
        }
    }

    return result;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::addIgnoreSymLinksRule()
{
    if (!mIgnoreSymLinkRule)
    {
        mIgnoreSymLinkRule = std::make_shared<MegaIgnoreNameRule>(QLatin1String("*"), MegaIgnoreNameRule::Class::EXCLUDE, MegaIgnoreNameRule::Target::s);
        addRule(mIgnoreSymLinkRule);
    }
    else if (mIgnoreSymLinkRule->isCommented())
    {
        mIgnoreSymLinkRule->setCommented(false);
    }

    return mIgnoreSymLinkRule;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::addIgnoreSymLinkRule(const QString& pattern)
{
    auto rule = std::make_shared<MegaIgnoreNameRule>(pattern, MegaIgnoreNameRule::Class::EXCLUDE, MegaIgnoreNameRule::Target::s);
    addRule(rule);
    return rule;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::addNameRule(MegaIgnoreNameRule::Class classType, const QString& pattern, MegaIgnoreNameRule::Target targetType)
{
    auto rule = std::make_shared<MegaIgnoreNameRule>(pattern, classType, targetType);
    addRule(rule);
    return rule;
}

void MegaIgnoreManager::setOutputIgnorePath(const QString& outputPath)
{
    mOutputMegaIgnoreFile = outputPath;
}

bool MegaIgnoreManager::hasChanged() const
{
    std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRC(mMegaIgnoreFile.toStdString().c_str()));
    auto IgnoreCRC = QString::fromUtf8(crc.get());
    return mIgnoreCRC.compare(IgnoreCRC) != 0;
}
