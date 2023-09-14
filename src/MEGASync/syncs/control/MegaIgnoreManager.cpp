#include "MegaIgnoreManager.h"

#include <Utilities.h>

#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QApplication>

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
           mLastRow = 0;
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
                       mHighLimitRule = std::make_shared<MegaIgnoreSizeRule>(this, line, isCommented, mLastRow);
                   }
                   else if(line.startsWith(MegaIgnoreSizeRule::LOW_STRING))
                   {
                       mLowLimitRule = std::make_shared<MegaIgnoreSizeRule>(this, line, isCommented, mLastRow);
                   }
                   else
                   {
                       if(lineSplitted.at(1).startsWith(QLatin1String("*.")))
                       {
                           mRules.append(std::make_shared<MegaIgnoreExtensionRule>(this, line, isCommented, mLastRow));
                       }
                       else
                       {
                           mRules.append(std::make_shared<MegaIgnoreNameRule>(this, line, isCommented, mLastRow));
                       }
                   }
               }

               mLastRow++;
           }
           ignore.close();
       }
    }
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getLowLimitRule()
{
    if(!mLowLimitRule)
    {
        mLowLimitRule = std::make_shared<MegaIgnoreSizeRule>(this, MegaIgnoreSizeRule::Threshold::Low);
    }

    return mLowLimitRule;
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getHighLimitRule()
{
    if(!mHighLimitRule)
    {
        mHighLimitRule = std::make_shared<MegaIgnoreSizeRule>(this, MegaIgnoreSizeRule::Threshold::High);
        mLastRow++;
    }

    return mHighLimitRule;
}

int MegaIgnoreManager::updateRow(int row, const QString &rule)
{
    QFile ignore(mMegaIgnoreFile);
    QStringList issues;
    if(ignore.open(QIODevice::ReadOnly))
    {
        QTextStream in(&ignore);

        auto fileRow(0);
        while (!in.atEnd())
        {
            auto currentRow = in.readLine();
            if(fileRow == row)
            {
                issues.append(rule);
            }
            else
            {
                issues.append(currentRow);
            }
            fileRow++;
        }

        if(row < 0)
        {
            row = fileRow;
            issues.append(rule);
        }

        ignore.close();

        if(ignore.open(QIODevice::WriteOnly))
        {
            QTextStream out(&ignore);
            out << issues.join(QLatin1String("\n"));

            ignore.close();
        }
    }

    return row;
}

////////////////MEGA IGNORE RULE
void MegaIgnoreRule::setCommented(bool newIsCommented)
{
    if(newIsCommented != mIsCommented)
    {
        mIsDirty = true;
        mIsCommented = newIsCommented;

        mManager->updateRow(mRow, getRuleAsString());
    }
}

bool MegaIgnoreRule::isCommented() const
{
    return mIsCommented;
}


/////////////////MEGA IGNORE NAME RULE
MegaIgnoreNameRule::MegaIgnoreNameRule(MegaIgnoreManager* manager, const QString &rule, bool isCommented, int row)
    :MegaIgnoreRule(manager, rule, isCommented, row)
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

        mRightSideRule = ruleSplitted.at(1);
    }
}


QString MegaIgnoreNameRule::getRuleAsString()
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
        rule.append(mRightSideRule);

        return rule;
    }
    else
    {
        return MegaIgnoreRule::getRuleAsString();
    }
}

////////////////MEGA IGNORE EXTENSION RULE
MegaIgnoreExtensionRule::MegaIgnoreExtensionRule(MegaIgnoreManager *manager, const QString &rule, bool isCommented, int row)
    : MegaIgnoreNameRule(manager, rule, isCommented, row)
{
    mRuleType = RuleType::ExtensionRule;
    auto extensionSplitter(mRightSideRule.split(QLatin1String(".")));
    if(extensionSplitter.size() == 2)
    {
        mExtension = extensionSplitter.at(1);
    }
}

////////////////MEGA IGNORE SIZE RULE
const QString MegaIgnoreSizeRule::LOW_STRING = QLatin1String("exclude-lower");
const QString MegaIgnoreSizeRule::HIGH_STRING = QLatin1String("exclude-larger");

MegaIgnoreSizeRule::MegaIgnoreSizeRule(MegaIgnoreManager* manager, const QString &rule, bool isCommented, int row)
    :MegaIgnoreRule(manager, rule, isCommented, row)
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

MegaIgnoreSizeRule::MegaIgnoreSizeRule(MegaIgnoreManager *manager, Threshold type)
    :MegaIgnoreRule(manager, QString(), true, -1)
    , mThreshold(type)
{
}

QString MegaIgnoreSizeRule::getRuleAsString()
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
        return MegaIgnoreRule::getRuleAsString();
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

        mManager->updateRow(mRow, getRuleAsString());
    }
}

void MegaIgnoreSizeRule::setUnit(int newUnit)
{
    if(newUnit != mUnit)
    {
        mIsDirty = true;
        mUnit = static_cast<UnitTypes>(newUnit);

        mManager->updateRow(mRow, getRuleAsString());
    }
}
