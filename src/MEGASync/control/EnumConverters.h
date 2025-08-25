#ifndef ENUMCONVERTERS_H
#define ENUMCONVERTERS_H

#include <QMetaEnum>

template<class EnumType>
class EnumConversions
{
public:
    EnumConversions():
        mMetaEnum(QMetaEnum::fromType<EnumType>())
    {}

    QString getString(EnumType type)
    {
        return QString::fromUtf8(mMetaEnum.valueToKey(static_cast<int>(type)));
    }

    EnumType getEnum(const QString& typeAsString)
    {
        return static_cast<EnumType>(mMetaEnum.keyToValue(typeAsString.toUtf8().constData()));
    }

private:
    QMetaEnum mMetaEnum;
};

template<class FlagType>
class FlagsConversions
{
public:
    FlagsConversions():
        mMetaEnum(QMetaEnum::fromType<FlagType>())
    {}

    QString getString(FlagType type)
    {
        QString joinedFlags;

        for (int i = 0; i < mMetaEnum.keyCount(); ++i)
        {
            int value = mMetaEnum.value(i);
            if (value != 0 && (type & FlagType(value)))
            {
                if (!joinedFlags.isEmpty())
                {
                    joinedFlags.append(QLatin1Char('_'));
                }

                joinedFlags.append(QString::fromUtf8(mMetaEnum.key(i)));
            }
        }

        return joinedFlags;
    }

    FlagType getEnum(const QString& typeAsString,
                     Qt::CaseSensitivity sensitive = Qt::CaseInsensitive)
    {
        int result(0);
        if (!typeAsString.isEmpty())
        {
            auto parts = typeAsString.split(QLatin1Char('_'), Qt::SkipEmptyParts);

            if (sensitive == Qt::CaseInsensitive)
            {
                std::transform(parts.begin(),
                               parts.end(),
                               parts.begin(),
                               [](const QString& s)
                               {
                                   return s.toLower();
                               });
            }

            for (int i = 0; i < mMetaEnum.keyCount(); ++i)
            {
                auto name = QString::fromUtf8(mMetaEnum.key(i));
                if (sensitive == Qt::CaseInsensitive)
                {
                    name = name.toLower();
                }

                auto index = parts.indexOf(name);

                if (index >= 0)
                {
                    result |= mMetaEnum.value(i);
                    parts.removeAt(index);
                }
            }
        }
        return static_cast<FlagType>(result);
    }

private:
    QMetaEnum mMetaEnum;
};

#endif // ENUMCONVERTERS_H
