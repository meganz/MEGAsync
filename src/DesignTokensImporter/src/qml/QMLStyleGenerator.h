#ifndef QML_STYLE_GENERATOR_H
#define QML_STYLE_GENERATOR_H

#include "IStyleGenerator.h"
#include "Types.h"

#include <QObject>

namespace DTI
{
    class QmlStyleGenerator : public QObject, public IStyleGenerator
    {
        Q_OBJECT

    public:
        explicit QmlStyleGenerator(QObject *parent = nullptr);
        void start(const FilePathColourMap& styleData) override;

    signals:
        void finished();
    };
}

#endif // QML_STYLE_GENERATOR_H
