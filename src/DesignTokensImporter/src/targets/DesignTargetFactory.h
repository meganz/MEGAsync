#ifndef DTI_DESIGN_TARGET_FACTORY_H
#define DTI_DESIGN_TARGET_FACTORY_H

#include "DesignTarget.h"

#include <QStringList>
#include <QDebug>

#include <unordered_map>
#include <memory>

namespace DTI
{
    class IConcreteDesignTargetFactory
    {
        public :
        virtual std::shared_ptr<IDesignTarget> makeDesignTarget() = 0;
    };

    using DesignTargetFactoryMap = std::unordered_map<std::string, std::unique_ptr<IConcreteDesignTargetFactory>>;

    class DesignTargetFactory
    {
    public:
        DesignTargetFactory() = delete;

        static std::shared_ptr<IDesignTarget> getDesignTarget(const QString& targetId)
        {
            if (targetId.isEmpty())
            {
                qWarning() << __func__ << " Error : could not get design target with empty targetId";
                return nullptr;
            }

            const auto& designTargetFactories = getDesignTargetFactories();
            auto foundIt = designTargetFactories.find(targetId.toStdString());
            if (foundIt != designTargetFactories.end())
            {
                return foundIt->second->makeDesignTarget();
            }

            return nullptr;
        }

        static bool registerDesignTargetFactoryById(const std::string& targetId, std::unique_ptr<IConcreteDesignTargetFactory> targetFactory)
        {
            if (targetId.empty() || targetFactory == nullptr)
            {
                qWarning() << __func__ << " Error : could not register design target factory with empty targetId or nullptr factory";

                return false;
            }

            auto& DesignTargetFactories = getDesignTargetFactories();
            if (DesignTargetFactories.find(targetId) == DesignTargetFactories.end())
            {
                auto result = DesignTargetFactories.emplace(targetId, std::move(targetFactory));
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
            return DesignTargetFactory::registerDesignTargetFactoryById(targetId, std::make_unique<ConcreteDesignTargetFactory<TDesignTarget>>());
        };

        std::shared_ptr<IDesignTarget> makeDesignTarget() override
        {
            return std::make_shared<TDesignTarget>();
        }
    };
}

#endif
