#ifndef DESIGN_GENERATOR_H
#define DESIGN_GENERATOR_H

#include "Types.h"

#include <QObject>

namespace DTI
{
    class DesignGenerator : public QObject
    {
        Q_OBJECT

    public:
        explicit DesignGenerator(QObject* parent = nullptr);
        void deploy(const DesignAssets& designAssets);
    };
}

#endif
