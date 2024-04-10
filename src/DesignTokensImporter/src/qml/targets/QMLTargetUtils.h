#ifndef QML_TARGET_UTILS_H
#define QML_TARGET_UTILS_H

#include <QString>

namespace DTI
{
    /*
     * helper function : qml style files can't have - as a word separators, so we have to convert
     * from surface-al to surfaceAl
    */
    QString normalizeTokenId(QString tokenId);
}

#endif
