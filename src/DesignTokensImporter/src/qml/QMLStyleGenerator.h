#ifndef QML_STYLE_GENERATOR_H
#define QML_STYLE_GENERATOR_H

#include "IQMLStyleGenerator.h"
#include "StyleDefinitions.h"

#include <QObject>

namespace DTI
{
    class QmlStyleGenerator : public QObject, public IQMLStyleGenerator
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
