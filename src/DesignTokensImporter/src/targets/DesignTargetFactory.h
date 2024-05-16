#ifndef DTI_DESIGN_TARGET_FACTORY_H
#define DTI_DESIGN_TARGET_FACTORY_H

#include "DesignTarget.h"

#include <QStringList>

#include <unordered_map>
#include <memory>

namespace DTI
{
    class IConcreteDesignTargetFactory
    {
        public :
        virtual IDesignTarget* makeDesignTarget() = 0;
    };

    using DesignTargetFactoryMap = std::unordered_map<std::string, std::unique_ptr<IConcreteDesignTargetFactory>>;

    class DesignTargetFactory
    {
    public:
        DesignTargetFactory() = delete;

        /*
         * Note on pointer ownership: function caller is responsible for deletion of this pointer.
        */
        static IDesignTarget* getDesignTarget(const QString& targetId)
        {
            const auto& designTargetFactories = getDesignTargetFactories();
            auto foundIt = designTargetFactories.find(targetId.toStdString());
            if (foundIt != designTargetFactories.end())
            {
                return foundIt->second->makeDesignTarget();
            }

            return nullptr;
        }

        static bool registerDesignTargetFactory(const std::string& targetId, IConcreteDesignTargetFactory* targetFactory)
        {
            auto& DesignTargetFactories = getDesignTargetFactories();
            if (DesignTargetFactories.find(targetId) == DesignTargetFactories.end())
            {
                auto result = DesignTargetFactories.emplace(targetId, targetFactory);
                return result.second;
            }

            return false;
        }

        static QStringList getRegisteredDesignTargets()
        {
            QStringList registeredDesignTargets;
            for (const auto& designTargetFactory : getDesignTargetFactories())
            {
                registeredDesignTargets.push_back(QString::fromStdString(designTargetFactory.first));
            }
            return registeredDesignTargets;
        }

        private:
        static DesignTargetFactoryMap& getDesignTargetFactories()
        {
            static DesignTargetFactoryMap designTargetFactories{};
            return designTargetFactories;
        }
    };

    template<class TDesignTarget>
    class ConcreteDesignTargetFactory : public IConcreteDesignTargetFactory
    {
    public:
        static bool Register(const std::string& targetId)
        {
            return DesignTargetFactory::registerDesignTargetFactory(targetId, new ConcreteDesignTargetFactory<TDesignTarget>());
        };

        TDesignTarget* makeDesignTarget() override
        {
            return new TDesignTarget();
        }
    };
}

#endif
