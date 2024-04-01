#ifndef QML_THEME_TARGET_FACTORY_H
#define QML_THEME_TARGET_FACTORY_H

#include "IQMLThemeTarget.h"

#include <QStringList>

#include <unordered_map>
#include <memory>

namespace DTI
{
    class IConcreteThemeTargetFactory
    {
        public :
        virtual IQMLThemeTarget* makeQMLThemeTarget() = 0;
    };

    using QMLThemeTargetFactoryMap = std::unordered_map<std::string, std::unique_ptr<IConcreteThemeTargetFactory>>;

    class QMLThemeTargetFactory
    {
    public:
        QMLThemeTargetFactory() = delete;

        /*
         * Note on pointer ownership: function caller is responsible for deletion of this pointer.
        */
        static IQMLThemeTarget* getQMLThemeTarget(const QString& ThemeTargetId)
        {
            const auto& ThemeTargetFactories = getFactories();
            auto foundIt = ThemeTargetFactories.find(ThemeTargetId.toStdString());
            if (foundIt != ThemeTargetFactories.end())
            {
                return foundIt->second->makeQMLThemeTarget();
            }
            else
            {
                return nullptr;
            }
        }

        static bool registerThemeTargetFactory(const std::string& ThemeTargetId, IConcreteThemeTargetFactory* ThemeTargetFactory)
        {
            auto& ThemeTargetFactories = getFactories();
            if (ThemeTargetFactories.find(ThemeTargetId) == ThemeTargetFactories.end())
            {
                auto result = ThemeTargetFactories.emplace(ThemeTargetId, ThemeTargetFactory);
                return result.second;
            }
            else
            {
                return false;
            }
        }

        static QStringList getRegisteredThemeTargets()
        {
            QStringList registeredThemeTargets;
            for (const auto& ThemeTargetFactory : getFactories())
            {
                registeredThemeTargets.push_back(QString::fromStdString(ThemeTargetFactory.first));
            }
            return registeredThemeTargets;
        }

        private:
        static QMLThemeTargetFactoryMap& getFactories()
        {
            static QMLThemeTargetFactoryMap ThemeTargetFactories{};
            return ThemeTargetFactories;
        }
    };

    template<class TQMLThemeTarget>
    class ConcreteQMLThemeFactory : public IConcreteThemeTargetFactory
    {
    public:
        static bool Register(const std::string& ThemeTargetId)
        {
            return QMLThemeTargetFactory::registerThemeTargetFactory(ThemeTargetId, new ConcreteQMLThemeFactory<TQMLThemeTarget>());
        };

        TQMLThemeTarget* makeQMLThemeTarget()
        {
            return new TQMLThemeTarget();
        }
    };
}

#endif
