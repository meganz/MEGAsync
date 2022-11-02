#include "SyncTooltipCreator.h"

#include <QCoreApplication>
#include <QToolTip>
#include <QtMath>
#include <QFontMetrics>

QString SyncTooltipCreator::createForLocal(const QString& path)
{
    return createFormattedPath(path, QCoreApplication::translate("SyncTooltip", "Local path:"));
}

QString SyncTooltipCreator::createForRemote(const QString& path)
{
    return createFormattedPath(path, QCoreApplication::translate("SyncTooltip", "MEGA path:"));
}

QString SyncTooltipCreator::createFormattedPath(const QString& path, const QString& label)
{
    const QFontMetrics fm (QToolTip::font());
    const QChar blankChar (QLatin1Char(' ')); // We can also use QChar::Tabulation

    QString lineString = label + blankChar + path;
    int currentWidth = fm.size(Qt::TextExpandTabs, lineString).width();
    return currentWidth > mMaxWidthInPixels ? createMultilinePath(path, label, fm, blankChar)
                                            : lineString;
}

QString SyncTooltipCreator::createMultilinePath(const QString& path, const QString& label, const QFontMetrics& fm, const QChar& blankChar)
{
    // Create line separator (with blank spaces)
    QString separator (label.front());
    auto labelWidth = fm.size(Qt::TextExpandTabs, label + blankChar + separator).width();
    while (fm.size(Qt::TextExpandTabs, separator).width() < labelWidth)
    {
        separator.prepend(blankChar);
    }
    separator.chop(1);
    separator.prepend(QChar::LineSeparator);

    // Create multiline text
    QString multilineText;
    QString currLine (label + blankChar);
    for (auto currChar : path)
    {
        currLine += currChar;
        if (fm.size(Qt::TextExpandTabs, currLine).width() > mMaxWidthInPixels)
        {
            multilineText += currLine;
            currLine = currChar != path.cend() ? separator : QString();
        }
    }
    return multilineText + currLine;
}
