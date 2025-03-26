#include "QMLTargetUtils.h"

#include <QMap>

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

    return tokenId;
}

