#ifndef WIDGETS_DESIGN_GENERATOR_H
#define WIDGETS_DESIGN_GENERATOR_H

#include "IThemeGenerator.h"
#include "Types.h"

#include <QObject>
#include <QSharedPointer>

namespace DTI
{
    class WidgetsDesignGenerator : public QObject, public IThemeGenerator
    {
        Q_OBJECT

    public:
        explicit WidgetsDesignGenerator(QObject* parent = nullptr);
        void start(const ThemedColorData& themedColorData) override;
    };
}

#endif
