#ifndef WIDGETS_DESIGN_GENERATOR_H
#define WIDGETS_DESIGN_GENERATOR_H

#include "IDesignGenerator.h"
#include "Types.h"

#include <QObject>
#include <QSharedPointer>

namespace DTI
{
    class WidgetsDesignGenerator : public QObject, public IDesignGenerator
    {
        Q_OBJECT

    public:
        explicit WidgetsDesignGenerator(QObject* parent = nullptr);
        void deploy(const DesignAssets& designAssets) override;
    };
}

#endif
