#ifndef STRINGCONVERSIONS_H
#define STRINGCONVERSIONS_H

#include "catch/catch.hpp"
#include "trompeloeil.hpp"

#include <QStringList>

namespace Catch
{
template<>
struct StringMaker<QStringList>
{
    static std::string convert(QStringList const& value)
    {
        std::string stringified = "{";
        for (const QString& str: qAsConst(value))
        {
            stringified += str.toStdString() + ", ";
        }
        return stringified + "}";
    }
};

template<>
struct StringMaker<QString>
{
    static std::string convert(QString const& value)
    {
        return value.toStdString();
    }
};
}

namespace trompeloeil
{
template<>
struct printer<QString>
{
    static void print(std::ostream& os, const QString& str)
    {
        os << str.toStdString();
    }
};
}

#endif // STRINGCONVERSIONS_H
