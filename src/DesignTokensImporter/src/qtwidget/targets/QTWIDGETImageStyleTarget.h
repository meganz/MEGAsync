#ifndef QTWIDGETIMAGESTYLETARGET_H
#define QTWIDGETIMAGESTYLETARGET_H

#include "IQTWIDGETStyleTarget.h"

#include <QVector>

namespace DTI
{
    class QTWIDGETImageStyleTarget : public IQTWIDGETStyleTarget
    {
    public:
        void process(const CurrentStyleBlockInfo& currentBlockInfo) override;
        const QVector<ImageThemeStyleInfo>& getImageStyles() const;

    private:
        static bool registered;
        QVector<ImageThemeStyleInfo>  mImageStyles;
    };
}


#endif // QTWIDGETIMAGESTYLETARGET_H
