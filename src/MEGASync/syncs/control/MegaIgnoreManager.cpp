#include "MegaIgnoreManager.h"

#include <Utilities.h>

#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QApplication>

namespace
{
    constexpr char SIZE_RULE_REG_EX[] = "#*(exclude-(larger|smaller)):[0-9]+[kmgb]?$";
    constexpr char OTHER_RULE_REG_EX[] = "#*[+-][]adfsNnpGgRr]*:[^:]+";
}

MegaIgnoreManager::MegaIgnoreManager(const QString& syncLocalFolder)
{
    auto ignorePath(syncLocalFolder + QDir::separator() + QString::fromUtf8(".megaignore"));
    QFile ignore(ignorePath);
    if(ignore.exists())
    {
       mMegaIgnoreFile = ignorePath;
       if(ignore.open(QIODevice::ReadOnly))
       {
           QTextStream in(&ignore);
           while (!in.atEnd())
           {
               QString line = in.readLine();
               bool isCommented(false);

               if(line.startsWith(QLatin1String("#")))
               {
                   line.remove(0,1);
                   isCommented = true;
               }

               auto lineSplitted(line.split(QLatin1String(":")));
               if(lineSplitted.size() == 2)
               {
                   if(line.startsWith(MegaIgnoreSizeRule::HIGH_STRING))
                   {
                       mHighLimitRule = std::make_shared<MegaIgnoreSizeRule>(line, isCommented);
                       mRules.append(mHighLimitRule);
                   }
                   else if(line.startsWith(MegaIgnoreSizeRule::LOW_STRING))
                   {
                       mLowLimitRule = std::make_shared<MegaIgnoreSizeRule>(line, isCommented);
                       mRules.append(mLowLimitRule);
                   }
                   else
                   {
                       if(lineSplitted.at(1).startsWith(QLatin1String("*.")))
                       {
                           mRules.append(std::make_shared<MegaIgnoreExtensionRule>(line, isCommented));
                       }
                       else
                       {
                           mRules.append(std::make_shared<MegaIgnoreNameRule>(line, isCommented));
                       }
                   }
               }
           }
           ignore.close();
       }

       if(!mLowLimitRule)
       {
           mLowLimitRule = std::make_shared<MegaIgnoreSizeRule>(MegaIgnoreSizeRule::Threshold::Low);
           mRules.append(mLowLimitRule);
       }

       if(!mHighLimitRule)
       {
           mHighLimitRule = std::make_shared<MegaIgnoreSizeRule>(MegaIgnoreSizeRule::Threshold::High);
           mRules.append(mHighLimitRule);
       }
    }
}

std::shared_ptr<MegaIgnoreRule> MegaIgnoreManager::getRuleByOriginalRule(const QString &originalRule)
{
    foreach(auto& rule, mRules)
    {
        if(rule->originalRule() == originalRule)
        {
            return rule;
        }
    }

    return nullptr;
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getLowLimitRule() const
{
    return mLowLimitRule;
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getHighLimitRule() const
{
    return mHighLimitRule;
}

bool MegaIgnoreManager::isValidRule(const QString& line)
{
    // Sanity check
	if (line.isEmpty())
		return false;
    // Check if size rule

	const QRegularExpression sizeRuleRegularExpression(QLatin1String(SIZE_RULE_REG_EX), QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match = sizeRuleRegularExpression.match(line);
	if (match.hasMatch())
		return true;

    // Check if other type of rule
	QRegularExpression nonSizeRuleRegularExpression{ QLatin1String(OTHER_RULE_REG_EX) };
	match = nonSizeRuleRegularExpression.match(line);
	return match.hasMatch();
}

QList<std::shared_ptr<MegaIgnoreNameRule> > MegaIgnoreManager::getNameRules() const
{
    QList<std::shared_ptr<MegaIgnoreNameRule>> rules;
    foreach(auto& rule, mRules)
    {
        if(rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::NameRule)
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
    foreach(auto& rule, mRules)
    {
        if(rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::ExtensionRule)
        {
            auto extensionRule = convert<MegaIgnoreExtensionRule>(rule);
            if(extensionRule)
            {
                extensions.append(extensionRule->extension());
            }
        }
    }
    return extensions;
}

void MegaIgnoreManager::enableExtensions(bool state)
{
    foreach(auto& rule, mRules)
    {
        if(rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::ExtensionRule)
        {
            rule->setCommented(!state);
        }
    }
}

void MegaIgnoreManager::applyChanges()
{
    QStringList rules;
    foreach(auto& rule, mRules)
    {
        if(rule->isValid() && !rule->isDeleted())
        {
            rules.append(rule->getModifiedRule());
        }
    }

    QFile ignore(mMegaIgnoreFile);
    if(ignore.open(QIODevice::WriteOnly))
    {
        QTextStream out(&ignore);
        out << rules.join(QLatin1String("\n"));

        ignore.close();
    }
}

////////////////MEGA IGNORE RULE
void MegaIgnoreRule::setCommented(bool newIsCommented)
{
    if(newIsCommented != mIsCommented)
    {
        mIsDirty = true;
        mIsCommented = newIsCommented;
    }
}

bool MegaIgnoreRule::isCommented() const
{
    return mIsCommented;
}

const QString &MegaIgnoreRule::originalRule() const
{
    return mRule;
}

bool MegaIgnoreRule::isDeleted() const
{
    return mIsDeleted;
}

void MegaIgnoreRule::setDeleted(bool newIsDeleted)
{
    mIsDeleted = newIsDeleted;
}

/////////////////MEGA IGNORE NAME RULE
MegaIgnoreNameRule::MegaIgnoreNameRule(const QString &rule, bool isCommented)
    :MegaIgnoreRule(rule, isCommented)
{
    mRuleType = RuleType::NameRule;

    auto ruleSplitted = rule.split(QLatin1String(":"));
    if(ruleSplitted.size() == 2)
    {
        //First part -> class, target, type, strategy
        auto leftSide(ruleSplitted.at(0));
        if(!leftSide.isEmpty())
        {
            auto charCounter(0);
            for (auto chr : leftSide)
            {
                if(charCounter == 0)
                {
                    mClass = chr == QLatin1String("-") ? Class::Exclude : Class::Include;
                }
                else
                {
                    if(detectValue(chr, &mTarget, Qt::CaseSensitive))
                    {
                        continue;
                    }

                    if(detectValue(chr, &mType, Qt::CaseSensitive))
                    {
                        continue;
                    }

                    if(detectValue(chr, &mStrategy, Qt::CaseInsensitive))
                    {
                        continue;
                    }
                }
            }
        }

        mPattern = ruleSplitted.at(1);
    }
}

QString MegaIgnoreNameRule::getModifiedRule() const
{
    if(mIsDirty)
    {
        QString rule;
        if(isCommented())
        {
            rule.append(QLatin1String("#"));
        }

        rule.append(mClass == Class::Exclude ? QLatin1String("-") : QLatin1String("+"));
        if(mTarget != Target::None)
        {
            EnumConversions<Target> convertEnum;
            rule.append(convertEnum.getString(mTarget));
        }
        if(mType != Type::None)
        {
            EnumConversions<Type> convertEnum;
            rule.append(convertEnum.getString(mType));
        }
        if(mStrategy != Strategy::None)
        {
            EnumConversions<Strategy> convertEnum;
            rule.append(convertEnum.getString(mStrategy));
        }
        rule.append(QLatin1String(":"));
        rule.append(mPattern);

        return rule;
    }
    else
    {
        return MegaIgnoreRule::getModifiedRule();
    }
}

////////////////MEGA IGNORE EXTENSION RULE
MegaIgnoreExtensionRule::MegaIgnoreExtensionRule(const QString &rule, bool isCommented)
    : MegaIgnoreNameRule(rule, isCommented)
{
    mRuleType = RuleType::ExtensionRule;
    auto extensionSplitter(mPattern.split(QLatin1String(".")));
    if(extensionSplitter.size() == 2)
    {
        mExtension = extensionSplitter.at(1);
    }
}

const QString &MegaIgnoreExtensionRule::extension() const
{
    return mExtension;
}

////////////////MEGA IGNORE SIZE RULE
const QString MegaIgnoreSizeRule::LOW_STRING = QLatin1String("exclude-smaller");
const QString MegaIgnoreSizeRule::HIGH_STRING = QLatin1String("exclude-larger");

MegaIgnoreSizeRule::MegaIgnoreSizeRule(const QString &rule, bool isCommented)
    :MegaIgnoreRule(rule, isCommented)
{
    mRuleType = RuleType::SizeRule;

    auto ruleSplitted = rule.split(QLatin1String(":"));
    if(ruleSplitted.size() == 2)
    {
        auto type(ruleSplitted.at(0));
        if(type == HIGH_STRING)
        {
            mThreshold = Threshold::High;
        }
        else if(type == LOW_STRING)
        {
            mThreshold = Threshold::Low;
        }

        auto size(ruleSplitted.at(1));

        QRegularExpression rx(QLatin1String("[0-9]+"));
        QRegularExpressionMatch match = rx.match(size);
        if (match.hasMatch())
        {
            auto value = match.captured(0);
            mValue = value.toUInt();
            auto unit = size.remove(value);
            if(!unit.isEmpty())
            {
                EnumConversions<UnitTypes> convertEnum;
                mUnit = convertEnum.getEnum(unit);
            }
        }
    }
}

MegaIgnoreSizeRule::MegaIgnoreSizeRule(Threshold type)
    :MegaIgnoreRule(QString(), true)
    , mThreshold(type)
{
}

bool MegaIgnoreSizeRule::isValid()const
{
    return !isCommented() && mValue > 0;
}

QString MegaIgnoreSizeRule::getModifiedRule() const
{
    if(mIsDirty)
    {
        QString rule;
        if(isCommented())
        {
            rule.append(QLatin1String("#"));
        }
        rule.append(mThreshold == Threshold::Low ? LOW_STRING : HIGH_STRING);
        rule.append(QLatin1String(":"));
        rule.append(QString::number(mValue));
        EnumConversions<UnitTypes> convertEnum;
        rule.append(convertEnum.getString(mUnit));
        return rule;
    }
    else
    {
        return MegaIgnoreRule::getModifiedRule();
    }
}

int MegaIgnoreSizeRule::value() const
{
    return mValue;
}

MegaIgnoreSizeRule::UnitTypes MegaIgnoreSizeRule::unit() const
{
    return mUnit;
}

QStringList MegaIgnoreSizeRule::getUnitsForDisplay()
{
    return QStringList() << QApplication::translate("Sizes", "B")
                         << QApplication::translate("Sizes", "KB")
                         << QApplication::translate("Sizes", "MB")
                         << QApplication::translate("Sizes", "GB");
}

void MegaIgnoreSizeRule::setValue(uint64_t newValue)
{
    if(mValue != newValue)
    {
        mIsDirty = true;
        mValue = newValue;
    }
}

void MegaIgnoreSizeRule::setUnit(int newUnit)
{
    if(newUnit != mUnit)
    {
        mIsDirty = true;
        mUnit = static_cast<UnitTypes>(newUnit);
    }
}
