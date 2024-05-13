#ifndef WIDGETS_DESIGN_TARGET_FACTORY_H
#define WIDGETS_DESIGN_TARGET_FACTORY_H

#include "WidgetsDesignTarget.h"

#include <unordered_map>
#include <memory>

namespace DTI
{
    class IDesignTargetFactory
    {
        public :
            virtual IWidgetsDesignTarget* makeWidgetsDesignTarget() = 0;
    };

    using WidgetsDesignTargetFactoryMap = std::unordered_map<std::string, std::unique_ptr<IDesignTargetFactory>>;

    class WidgetsDesignTargetFactory
    {
    public:
        WidgetsDesignTargetFactory() = delete;

        /*
         * Note on pointer ownership: function caller is responsible for deletion of this pointer.
        */
        static IWidgetsDesignTarget* getWidgetsDesignTarget(const QString& designTargetId)
        {
            const auto& designTargetFactories = getFactories();
            auto foundTargetIt = designTargetFactories.find(designTargetId.toStdString());
            if (foundTargetIt != designTargetFactories.end())
            {
                return foundTargetIt->second->makeWidgetsDesignTarget();
            }

            return nullptr;
        }

        static bool registerDesignTargetFactory(const std::string& designTargetId, IDesignTargetFactory* designTargetFactory)
        {
            auto& designTargetFactories = getFactories();
            if (designTargetFactories.find(designTargetId) == designTargetFactories.end())
            {
                auto result = designTargetFactories.emplace(designTargetId, designTargetFactory);
                return result.second;
            }

            return false;
        }

        static QStringList getRegisteredDesignTargets()
        {
            QStringList registeredDesignTargets;
            for (const auto& styleTargetFactory : getFactories())
            {
                registeredDesignTargets << QString::fromStdString(styleTargetFactory.first);
            }
            return registeredDesignTargets;
        }

        private:
        static WidgetsDesignTargetFactoryMap& getFactories()
        {
            static WidgetsDesignTargetFactoryMap styleTargetFactories{};
            return styleTargetFactories;
        }
    };

    template<class TWidgetsDesignTarget>
    class WidgetsDesignFactory : public IDesignTargetFactory
    {
    public:
        static bool Register(const std::string& styleTargetId)
        {
            return WidgetsDesignTargetFactory::registerDesignTargetFactory(styleTargetId, new WidgetsDesignFactory<TWidgetsDesignTarget>());
        };

        TWidgetsDesignTarget* makeWidgetsDesignTarget() override
        {
            return new TWidgetsDesignTarget();
        }
    };
}

#endif
