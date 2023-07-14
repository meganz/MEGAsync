#include "DateTimeFormatter.h"

#include <QLocale>

QString DateTimeFormatter::create(const QString& languageCode, const QDateTime& datetime, QLocale::FormatType format)
{
    DateTimeFormatter formatter(languageCode, datetime, format);
    if (formatter.isToday())
    {
        return formatter.createTodayString();
    }
    if (formatter.isYesterday())
    {
        return formatter.createYesterdayString();
    }
    return formatter.createAnydayString();
}

DateTimeFormatter::DateTimeFormatter(const QString& languageCode, const QDateTime &datetime, QLocale::FormatType format)
    : QObject(nullptr), mLocale(languageCode), mDatetime(datetime), mAddDotForShorMonths(true), mFormat(format)
{
}

bool DateTimeFormatter::isToday() const
{
    return mDatetime.date() == QDateTime::currentDateTime().date();
}

bool DateTimeFormatter::isYesterday() const
{
    return mDatetime.date().addDays(1) == QDateTime::currentDateTime().date();
}

QString DateTimeFormatter::createTodayString()
{
    return tr("Today at %1").arg(createTimeString());
}

QString DateTimeFormatter::createYesterdayString()
{
    return tr("Yesterday at %1").arg(createTimeString());
}

QString DateTimeFormatter::createAnydayString()
{
    auto date = mDatetime.date();
    QString formatString = createLocalizedFormatString();
    formatString = formatString.replace(QLatin1String("[DAYOFWEEK]"), mLocale.dayName(date.dayOfWeek()))
                       .replace(QLatin1String("[DAYOFMONTH]"), QString::number(date.day()))
                       .replace(QLatin1String("[MONTH]"), mFormat == QLocale::FormatType::LongFormat ? mLocale.monthName(date.month())
                                                                                                     : createShortMonthString(date))
                       .replace(QLatin1String("[MONTHNUMBER]"), QString::number(date.month()))
                       .replace(QLatin1String("[YEAR]"), QString::number(date.year()))
            .replace(QLatin1String("[TIME]"), createTimeString());
    if(!formatString.isEmpty())
    {
        formatString[0] = formatString[0].toUpper();
    }
    return formatString;
}

QString DateTimeFormatter::createLocalizedFormatString()
{
    auto lang = mLocale.language();
    mAddDotForShorMonths = true;
    if (lang == QLocale::Arabic)
    {
        return createFormatString("[DAYOFWEEK] الساعة [TIME]",
                                  "[DAYOFMONTH][MONTH] الساعة [TIME]",
                                  mFormat == QLocale::FormatType::LongFormat ? "[DAYOFMONTH][MONTH] [YEAR] الساعة [TIME]"
                                                : "[DAYOFMONTH][MONTH] [YEAR] [TIME]");
    }
    else if (lang == QLocale::Chinese)
    {
        if (mLocale.country() == QLocale::Taiwan)
        {
            return createFormatString(QString::fromUtf8("[DAYOFWEEK] [TIME]"),
                                    QString::fromUtf8("年[MONTHNUMBER]月[DAYOFMONTH]日 [TIME]"),
                                    QString::fromUtf8("[YEAR]年[MONTHNUMBER]月[DAYOFMONTH]日 [TIME]"));
        }
        else
        {
            return createFormatString(QString::fromUtf8("[DAYOFWEEK] [TIME]"),
                                    QString::fromUtf8("[MONTHNUMBER]月[DAYOFMONTH]日 [TIME]"),
                                    QString::fromUtf8("[YEAR]年[MONTHNUMBER]月[DAYOFMONTH]日 [TIME]"));
        }
    }
    else if (lang == QLocale::Dutch)
    {
        return createFormatString("[DAYOFWEEK] om [TIME]",
                                "[DAYOFMONTH] [MONTH] om [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR] om [TIME]");
    }
    else if (lang == QLocale::English)
    {
        mAddDotForShorMonths = false;
        return createFormatString("[DAYOFWEEK] at [TIME]",
                                "[DAYOFMONTH] [MONTH] at [TIME]",
                                "[DAYOFMONTH] [MONTH], [YEAR] at [TIME]");
    }
    else if (lang == QLocale::French)
    {
        return createFormatString("[DAYOFWEEK] à [TIME]",
                                "[DAYOFMONTH] [MONTH] à [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR] à [TIME]");
    }
    else if (lang == QLocale::German)
    {
        return createFormatString("[DAYOFWEEK] [TIME]",
                                "[DAYOFMONTH]. [MONTH] [TIME]",
                                mFormat == QLocale::FormatType::LongFormat ? "[DAYOFWEEK], [DAYOFMONTH]. [MONTH] [YEAR] [TIME]"
                                              : "[DAYOFMONTH]. [MONTH] [YEAR] [TIME]");
    }
    else if (lang == QLocale::Indonesian)
    {
        return createFormatString("[DAYOFWEEK] jam [TIME]",
                                "[DAYOFMONTH] [MONTH] jam [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR] jam [TIME]");
    }
    else if (lang == QLocale::Italian)
    {
        return createFormatString("[DAYOFWEEK] alle [TIME]",
                                "[DAYOFMONTH] [MONTH] alle [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR] alle [TIME]");
    }
    else if (lang == QLocale::Japanese)
    {
        return createFormatString("[DAYOFWEEK] [TIME]",
                                "[MONTHNUMBER]月[DAYOFMONTH]日 [TIME]",
                                "[YEAR]年[MONTHNUMBER]月[DAYOFMONTH]日 [TIME]");
    }
    else if (lang == QLocale::Korean)
    {
        return createFormatString("[DAYOFWEEK] [TIME]",
                                "[MONTHNUMBER]월 [DAYOFMONTH]일 [TIME]",
                                "[YEAR]년 [MONTHNUMBER]월 [DAYOFMONTH]일 [TIME]");
    }
    else if (lang == QLocale::Polish)
    {
        return createFormatString("[DAYOFWEEK] [TIME]",
                                "[DAYOFMONTH] [MONTH], [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR], [TIME]");
    }
    else if (lang == QLocale::Portuguese)
    {
        if (mDatetime.time().hour() == 1)
        {
            return createFormatString("[DAYOFWEEK] a [TIME]",
                                    "[DAYOFMONTH] de [MONTH] a [TIME]",
                                    mFormat == QLocale::FormatType::LongFormat ? "[DAYOFMONTH] de [MONTH] de [YEAR] a [TIME]"
                                                  : "[DAYOFMONTH] [MONTH] [YEAR] a [TIME]");
        }
        return createFormatString("[DAYOFWEEK] às [TIME]",
                                "[DAYOFMONTH] de [MONTH] às [TIME]",
                                mFormat == QLocale::FormatType::LongFormat ? "[DAYOFMONTH] de [MONTH] de [YEAR] às [TIME]"
                                              : "[DAYOFMONTH] [MONTH] [YEAR] às [TIME]");
    }
    else if (lang == QLocale::Romanian)
    {
        return createFormatString("[DAYOFWEEK] la [TIME]",
                                "[DAYOFMONTH] [MONTH], [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR], [TIME]");
    }
    else if (lang == QLocale::Russian)
    {
        return createFormatString("[DAYOFWEEK] в [TIME]",
                                "[DAYOFMONTH] в [MONTH] [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR] в [TIME]");
    }
    else if (lang == QLocale::Spanish)
    {
        return createFormatString("[DAYOFWEEK], [TIME]",
                                "[DAYOFMONTH] de [MONTH], [TIME]",
                                mFormat == QLocale::FormatType::LongFormat ? "[DAYOFMONTH] de [MONTH] de [YEAR], [TIME]"
                                              : "[DAYOFMONTH] [MONTH] [YEAR], [TIME]");
    }
    else if (lang == QLocale::Thai)
    {
        return createFormatString("[DAYOFWEEK]ที่ เวลา [TIME]",
                                "[DAYOFMONTH] [MONTH] เวลา [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR] เวลา [TIME]");
    }
    else if (lang == QLocale::Vietnamese)
    {
        return createFormatString("[DAYOFWEEK], lúc [TIME]",
                                "[DAYOFMONTH] [MONTH], lúc [TIME]",
                                "[DAYOFMONTH] [MONTH] [YEAR], lúc [TIME]");
    }

    // Unsupported language. Should never fall here.
    return createFormatString("[DAYOFWEEK], [DAYOFMONTH] [MONTH] [TIME]",
                            "[DAYOFMONTH] [MONTH] [TIME]",
                            "[DAYOFMONTH] [MONTH] [YEAR] [TIME]");
}

QString DateTimeFormatter::createFormatString(const QString &sameWeekAndYearStr,
                                              const QString &sameYearStr, const QString &otherStr)
{
    auto today = QDateTime::currentDateTime().date();
    if (mDatetime.date().year() == today.year())
    {
        //When it is the same year, we always use the long format
        mFormat = QLocale::FormatType::LongFormat;

        if (mDatetime.date().weekNumber() == today.weekNumber())
        {
            return sameWeekAndYearStr;
        }
        return sameYearStr;
    }
    return otherStr;
}

QString DateTimeFormatter::createFormatString(const char *sameWeekAndYearStr,
                                              const char *sameYearStr, const char *otherStr)
{
    return createFormatString(QString::fromUtf8(sameWeekAndYearStr), QString::fromUtf8(sameYearStr),
                              QString::fromUtf8(otherStr));
}

QString DateTimeFormatter::createTimeString()
{
    auto lang = mLocale.language();
    auto time = mDatetime.time();
    if (lang == QLocale::Arabic)
    {
        bool isPm = (time.hour() > 12);
        QString ampmText = isPm ? QString::fromUtf8("مساءً") : QString::fromUtf8("صباحًا");
        int hours = isPm ? time.hour() - 12 : time.hour();
        return QString::fromUtf8("%1:%2%3").arg(hours)
                                           .arg(createMinuteString(time.minute()))
                                           .arg(ampmText);
    }
    else if (lang == QLocale::English)
    {
        if (time.minute() == 0)
        {
            if (time.hour() == 0)
            {
                return QString::fromLatin1("midnight");
            }
            else if (time.hour() == 12)
            {
                return QString::fromLatin1("noon");
            }
            return time.toString(QString::fromLatin1("hap"));
        }

        return time.toString(QString::fromLatin1("h:mmap"));
    }
    else if (lang == QLocale::French)
    {
        if (time.minute() == 0)
        {
            return QString::fromLatin1("%1 h").arg(time.hour());
        }
        return QString::fromLatin1("%1 h %2").arg(time.hour())
                                             .arg(createMinuteString(time.minute()));
    }
    else if (lang == QLocale::Portuguese)
    {
        if (time.minute() == 0)
        {
            return QString::fromLatin1("%1h").arg(time.hour());
        }
        return QString::fromLatin1("%1h %2").arg(time.hour())
                                             .arg(createMinuteString(time.minute()));
    }
    else if (lang == QLocale::Korean)
    {
        return QString::fromUtf8("%1시%2분").arg(time.hour())
                                             .arg(createMinuteString(time.minute()));
    }
    else if (lang == QLocale::Spanish)
    {
        return QString::fromLatin1("%1:%2 h").arg(time.hour())
                                             .arg(createMinuteString(time.minute()));
    }
    else if (lang == QLocale::Thai)
    {
        return QString::fromUtf8("%1:%2 น.").arg(time.hour())
                                              .arg(createMinuteString(time.minute()));
    }
    else if (lang == QLocale::Vietnamese)
    {
        time.toString(QString::fromUtf8("h:mmap"));
    }

    // One of Chinese, Dutch, German, Indonesian, Italian,
    // Japanese, Polish, Romanian, Russian
    return time.toString(QString::fromUtf8("HH:mm"));
}

QString DateTimeFormatter::createShortMonthString(const QDate &date)
{
    auto month = mLocale.monthName(date.month(), QLocale::FormatType::ShortFormat);
    if(month.endsWith(QLatin1String(".")) && !mAddDotForShorMonths)
    {
        month = month.remove(month.length() - 1,1);
    }
    return month;
}

QString DateTimeFormatter::createMinuteString(int minutes)
{
    QString prefix = (minutes < 10) ? QString::fromLatin1("0") : QString();
    return prefix + QString::number(minutes);
}
