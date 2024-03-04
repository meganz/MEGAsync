#include "TextDecorator.h"
#include <QDebug>

namespace Text
{

Decorator::Decorator(QObject *parent) :
    QObject(parent)
{
}

void Decorator::process(QString &input) const
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

//Use [A] for url replacement
void Link::process(QString &input) const
{
    Decorator::process(input);
    input.replace(QLatin1String("[A]"), QString::fromUtf8("<a href=\"%1\">").arg(mLinkAddress));
    input.replace(QLatin1String("[/A]"), QLatin1String("</a>"));
}

ClearLink::ClearLink(QObject *parent) : Decorator(parent)
{

}

void ClearLink::process(QString &input) const
{
    Decorator::process(input);
    input.remove(QLatin1String("[A]"));
    input.remove(QLatin1String("[/A]"));
}

Bold::Bold(QObject *parent) : Decorator(parent)
{
}

//Use [B] for bold tags replacement
void Bold::process(QString &input) const
{
    Decorator::process(input);
    input.replace(QLatin1String("[B]"), QLatin1String("<b>"));
    input.replace(QLatin1String("[/B]"), QLatin1String("</b>"));
}
}
