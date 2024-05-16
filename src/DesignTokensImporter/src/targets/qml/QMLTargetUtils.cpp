#include "QMLTargetUtils.h"

#include <QMap>

static QMap<QString, QString> tokenIdNamingExceptions = {{"focus", "focusColor"}};

/*
 * helper function : qml style files can't have - as a word separators, so we have to convert
 * from surface-al to surfaceAl
*/
QString DTI::normalizeTokenId(QString tokenId)
{
    auto index = tokenId.indexOf('-');
    while (index != -1)
    {
        tokenId.remove(index, 1);
        if (index < tokenId.size())
        {
            tokenId[index] = tokenId[index].toUpper();
        }
        index = tokenId.indexOf('-', index);
    }

    /*
     * Check if the token id is in the exceptions list.
     * We realize that some token names, like "focus" can't be used as a token id in qml,
     * because they are reserved words.
    */
    if (tokenIdNamingExceptions.contains(tokenId))
    {
        tokenId = tokenIdNamingExceptions[tokenId];
    }

    return tokenId;
}

