#ifndef QML_THEME_GENERATOR_H
#define QML_THEME_GENERATOR_H

#include "IThemeGenerator.h"
#include "Types.h"

#include <QObject>

namespace DTI
{
    class QmlThemeGenerator : public QObject, public IThemeGenerator
    {
        Q_OBJECT

    public:
        explicit QmlThemeGenerator(QObject *parent = nullptr);
        void start(const ThemedColourMap& themeData) override;

    signals:
        void finished();
    };
}

#endif
