#ifndef DTI_DESIGN_GENERATOR_H
#define DTI_DESIGN_GENERATOR_H

#include "IDesignGenerator.h"
#include "Types.h"

#include <QObject>

namespace DTI
{
    class DesignGenerator : public QObject, public IDesignGenerator
    {
        Q_OBJECT

    public:
        explicit DesignGenerator(QObject* parent = nullptr);
        void deploy(const DesignAssets& designAssets) override;
    };
}

#endif
