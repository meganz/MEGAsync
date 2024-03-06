#ifndef IQTWIDGETSTYLETARGET_H
#define IQTWIDGETSTYLETARGET_H

#include "Types.h"

#include <QString>

namespace  DTI
{
class IQTWIDGETStyleTarget
{
public:
    struct CurrentStyleBlockInfo
    {
        QString styleSheetLine;
        QString content;
        QString selector;
        QString filePath;
    };
    virtual void process(const CurrentStyleBlockInfo &currentBlockInfo) = 0;
    virtual ~IQTWIDGETStyleTarget() = default;
};
}

#endif // IQTWIDGETSTYLETARGET_H
