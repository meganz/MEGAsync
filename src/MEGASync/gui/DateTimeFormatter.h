#ifndef DATETIMEFORMATTER_H
#define DATETIMEFORMATTER_H

#include <QObject>

#include <QDateTime>
#include <QLocale>
#include <QString>

class DateTimeFormatter : public QObject
{
    Q_OBJECT

public:
    static QString create(const QString &languageCode, const QDateTime& datetime, QLocale::FormatType format = QLocale::FormatType::LongFormat);

private:
    DateTimeFormatter(const QString& languageCode, const QDateTime& datetime, QLocale::FormatType format);

    bool isToday() const;
    bool isYesterday() const;

    QString createTodayString();
    QString createYesterdayString();
    QString createAnydayString();

    QString createLocalizedFormatString();
    QString createFormatString(const QString& sameWeekAndYearStr,
                               const QString& sameYearStr, const QString& otherStr);
    QString createFormatString(const char* sameWeekAndYearStr,
                               const char* sameYearStr, const char* otherStr);
    QString createTimeString();
    QString createShortMonthString(const QDate& date);

    static QString createMinuteString(int minutes);


    QLocale mLocale;
    QDateTime mDatetime;
    bool mAddDotForShorMonths;
    QLocale::FormatType mFormat;
};

#endif // DATETIMEFORMATTER_H
