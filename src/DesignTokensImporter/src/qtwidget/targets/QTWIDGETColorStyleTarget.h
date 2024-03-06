#ifndef QTWIDGET_COLOR_STYLE_TARGET_H
#define QTWIDGET_COLOR_STYLE_TARGET_H

#include "IQTWIDGETStyleTarget.h"

#include <QMultiMap>
#include <string>

namespace DTI
{
    class QTWIDGETColorStyleTarget : public IQTWIDGETStyleTarget
    {
    public:
        void process(const CurrentStyleBlockInfo &currentBlockInfo) override ;
        const QMultiMap<QString, PropertiesMap>& getTokenStyles() const;

    private:
        static bool registered;
        QMultiMap<QString, PropertiesMap> mTokenStyles;
    };
}

#endif
