#include "TextDecorator.h"
#include <QDebug>

namespace Text
{

Decorator::Decorator(QObject *parent) :
    QObject(parent)
{
}

void Decorator::process(QString &input)
{
    if(parent())
    {
        if(Decorator *td = qobject_cast<Decorator*>(parent()))
        {
            td->process(input);
        }
    }
}

Link::Link(const QString &link, QObject *parent) :
    Decorator(parent),
    mLinkAddress(link)
{
}

void Link::process(QString &input)
{
    Decorator::process(input);
    input.replace(QLatin1String("[A]"), QString::fromUtf8("<a href=\"%1\">").arg(mLinkAddress));
    input.replace(QLatin1String("[/A]"), QLatin1String("</a>"));
}

ClearLink::ClearLink(QObject *parent) : Decorator(parent)
{

}

void ClearLink::process(QString &input)
{
    Decorator::process(input);
    input.remove(QLatin1String("[A]"));
    input.remove(QLatin1String("[/A]"));
}

Bold::Bold(QObject *parent) : Decorator(parent)
{
}

void Bold::process(QString &input)
{
    Decorator::process(input);
    input.insert(0, QLatin1String("<b>"));
    input.append(QLatin1String("</b>"));
}
}
