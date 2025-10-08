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

Link::Link(const QStringList& links, QObject *parent) :
    Decorator(parent),
    mLinkAddresses(links)
{
}

Link::Link(const QString& link, QObject* parent):
    Decorator(parent),
    mLinkAddresses(link)
{

}

//Use [A] for url replacement
static const QString headerTag = QLatin1String("[A]");
void Link::process(QString &input) const
{
    Decorator::process(input);

    auto headerLength(headerTag.length());
    auto currentIndex(0);
    foreach(auto link, mLinkAddresses)
    {
        currentIndex = input.indexOf(headerTag, currentIndex);
        if (link.isEmpty())
        {
            input.replace(currentIndex, headerLength, QString::fromUtf8("<a href=\"empty\">"));
        }
        else
        {
            input.replace(currentIndex,
                          headerLength,
                          QString::fromUtf8("<a href=\"%1\">").arg(link));
        }
        currentIndex += headerLength;
    }

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

NewLine::NewLine(QObject *parent) : Decorator(parent)
{
}

//Use [BR] for new line tags replacement
void NewLine::process(QString &input) const
{
    Decorator::process(input);
    input.replace(QLatin1String("[BR]"), QLatin1String("<br>"));
    input.replace(QLatin1String("[/BR]"), QLatin1String(""));
}

void NewLine::process(QString &input, int count) const
{
    Decorator::process(input);

    QString brTag = QString::fromUtf8("<br>").repeated(count);
    input.replace(QLatin1String("[BR]"), brTag);
    input.replace(QLatin1String("[/BR]"), QLatin1String(""));
}

RichText::RichText(QObject* parent):
    Decorator(parent),
    mLinkAddresses(QStringList())
{}

RichText::RichText(const QStringList& links, QObject* parent):
    Decorator(parent),
    mLinkAddresses(links)
{}

RichText::RichText(const QString& link, QObject* parent):
    Decorator(parent),
    mLinkAddresses(link)
{}

void RichText::process(QString& input) const
{
    if (!mLinkAddresses.isEmpty())
    {
        Text::Link(mLinkAddresses).process(input);
    }
    Text::Bold().process(input);
    Text::NewLine().process(input);
}
}
