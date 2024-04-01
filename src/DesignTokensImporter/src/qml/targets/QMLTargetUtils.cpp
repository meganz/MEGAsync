#include "QMLTargetUtils.h"

/*
 * helper function : qml style files can't have - as a word separators, so we have to convert
 * from surface-al to surfaceAl
*/
QString DTI::normalizeTokenId(QString tokenId)
{
    while(tokenId.indexOf('-') != -1)
    {
        auto index = tokenId.indexOf('-');
        tokenId.remove(index, 1);

        if (index < tokenId.size())
        {
            auto afterSeparatorChar = tokenId.at(index);
            tokenId[index] = afterSeparatorChar.toUpper();
        }
    }

    return tokenId;
}

