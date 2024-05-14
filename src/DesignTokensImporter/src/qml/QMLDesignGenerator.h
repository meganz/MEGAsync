#ifndef QML_DESIGN_GENERATOR_H
#define QML_DESIGN_GENERATOR_H

#include "IDesignGenerator.h"
#include "Types.h"

#include <QObject>

namespace DTI
{
    class QMLDesignGenerator : public QObject, public IDesignGenerator
    {
        Q_OBJECT

    public:
        explicit QMLDesignGenerator(QObject* parent = nullptr);
        void deploy(const DesignAssets& designAssets) override;

    signals:
        void finished();
    };
}

#endif
